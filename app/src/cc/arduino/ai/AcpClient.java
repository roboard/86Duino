/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

/**
 * Minimal Agent Client Protocol (ACP) client: JSON-RPC 2.0, one message per
 * line, over the stdin/stdout of a "hermes acp" process.
 */
public class AcpClient {

  /** Callbacks from the agent. All of them are called on background threads. */
  public interface Listener {
    /** A session/update notification; the argument is params.update. */
    void onSessionUpdate(JsonNode update);

    /**
     * A session/request_permission request. Blocks until the user answers and
     * returns the chosen optionId (null means cancelled).
     */
    String onPermissionRequest(JsonNode params);

    /** The Hermes process has ended. */
    void onExit(int exitCode);
  }

  /** An error answer to one of our requests. */
  public static class AcpException extends IOException {
    public AcpException(String message) {
      super(message);
    }
  }

  private static final Set<AcpClient> RUNNING = ConcurrentHashMap.newKeySet();

  static {
    // System.exit() is used when the IDE quits: never leave python.exe behind.
    Runtime.getRuntime().addShutdownHook(new Thread(() -> {
      for (AcpClient c : RUNNING) {
        c.destroy();
      }
    }, "acp-shutdown"));
  }

  private final ObjectMapper mapper = new ObjectMapper();
  private final Map<Long, CompletableFuture<JsonNode>> pending = new ConcurrentHashMap<>();
  private final AtomicLong nextId = new AtomicLong(1);
  private final ExecutorService agentRequests = Executors.newCachedThreadPool(r -> {
    Thread t = new Thread(r, "acp-agent-request");
    t.setDaemon(true);
    return t;
  });

  private final Listener listener;
  private Process process;
  private OutputStream stdin;

  public AcpClient(Listener listener) {
    this.listener = listener;
  }

  /** Starts "hermes acp" with the given working directory. */
  public void start(HermesBundle bundle, File cwd) throws IOException {
    bundle.ensureHome();
    File log = bundle.getLogFile();
    log.getParentFile().mkdirs();
    if (log.length() > 5L * 1024 * 1024) {
      File old = new File(log.getPath() + ".1");
      old.delete();
      log.renameTo(old);
    }
    ProcessBuilder pb = bundle.processBuilder(cwd, "acp");
    pb.redirectError(ProcessBuilder.Redirect.appendTo(log));
    process = pb.start();
    stdin = process.getOutputStream();
    RUNNING.add(this);

    Thread reader = new Thread(this::readLoop, "acp-reader");
    reader.setDaemon(true);
    reader.start();
  }

  public boolean isAlive() {
    return process != null && process.isAlive();
  }

  public ObjectNode newObject() {
    return mapper.createObjectNode();
  }

  // ---------------------------------------------------------------- protocol

  public CompletableFuture<JsonNode> initialize(String clientName, String clientVersion) {
    ObjectNode params = newObject();
    params.put("protocolVersion", 1);
    ObjectNode caps = params.putObject("clientCapabilities");
    // Hermes writes files itself; the IDE reloads them from disk afterwards.
    ObjectNode fs = caps.putObject("fs");
    fs.put("readTextFile", false);
    fs.put("writeTextFile", false);
    caps.put("terminal", false);
    ObjectNode info = params.putObject("clientInfo");
    info.put("name", clientName);
    info.put("version", clientVersion);
    return request("initialize", params);
  }

  /** Creates a session; the future yields the full session/new result. */
  public CompletableFuture<JsonNode> newSession(File cwd) {
    ObjectNode params = newObject();
    params.put("cwd", cwd.getAbsolutePath());
    params.putArray("mcpServers");
    return request("session/new", params);
  }

  /**
   * Continues a saved session. Hermes first replays its history as session
   * updates, then answers; the result is null or empty when the session is unknown.
   */
  public CompletableFuture<JsonNode> loadSession(String sessionId, File cwd) {
    ObjectNode params = newObject();
    params.put("sessionId", sessionId);
    params.put("cwd", cwd.getAbsolutePath());
    params.putArray("mcpServers");
    return request("session/load", params);
  }

  public CompletableFuture<JsonNode> setMode(String sessionId, String modeId) {
    ObjectNode params = newObject();
    params.put("sessionId", sessionId);
    params.put("modeId", modeId);
    return request("session/set_mode", params);
  }

  /** Sends one user turn; the future yields the result (with stopReason). */
  public CompletableFuture<JsonNode> prompt(String sessionId, List<String> textBlocks) {
    ObjectNode params = newObject();
    params.put("sessionId", sessionId);
    ArrayNode prompt = params.putArray("prompt");
    for (String text : textBlocks) {
      ObjectNode block = prompt.addObject();
      block.put("type", "text");
      block.put("text", text);
    }
    return request("session/prompt", params);
  }

  public void cancel(String sessionId) {
    ObjectNode params = newObject();
    params.put("sessionId", sessionId);
    notify("session/cancel", params);
  }

  // ---------------------------------------------------------------- JSON-RPC

  public CompletableFuture<JsonNode> request(String method, ObjectNode params) {
    long id = nextId.getAndIncrement();
    CompletableFuture<JsonNode> future = new CompletableFuture<>();
    pending.put(id, future);
    ObjectNode msg = newObject();
    msg.put("jsonrpc", "2.0");
    msg.put("id", id);
    msg.put("method", method);
    msg.set("params", params);
    try {
      send(msg);
    } catch (IOException e) {
      pending.remove(id);
      future.completeExceptionally(e);
    }
    return future;
  }

  public void notify(String method, ObjectNode params) {
    ObjectNode msg = newObject();
    msg.put("jsonrpc", "2.0");
    msg.put("method", method);
    msg.set("params", params);
    try {
      send(msg);
    } catch (IOException e) {
      // process gone; onExit reports it
    }
  }

  private synchronized void send(JsonNode msg) throws IOException {
    if (stdin == null) {
      throw new IOException("Hermes is not running");
    }
    stdin.write(mapper.writeValueAsBytes(msg));
    stdin.write('\n');
    stdin.flush();
  }

  private void respond(JsonNode id, JsonNode result) {
    ObjectNode msg = newObject();
    msg.put("jsonrpc", "2.0");
    msg.set("id", id);
    msg.set("result", result);
    try {
      send(msg);
    } catch (IOException e) {
      // process gone
    }
  }

  private void respondError(JsonNode id, int code, String message) {
    ObjectNode msg = newObject();
    msg.put("jsonrpc", "2.0");
    msg.set("id", id);
    ObjectNode error = msg.putObject("error");
    error.put("code", code);
    error.put("message", message);
    try {
      send(msg);
    } catch (IOException e) {
      // process gone
    }
  }

  private void readLoop() {
    try (BufferedReader in = new BufferedReader(new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
      String line;
      while ((line = in.readLine()) != null) {
        line = line.trim();
        if (line.isEmpty() || line.charAt(0) != '{') {
          continue;
        }
        JsonNode msg;
        try {
          msg = mapper.readTree(line);
        } catch (IOException e) {
          continue;
        }
        dispatch(msg);
      }
    } catch (IOException e) {
      // stream closed
    }
    int code = -1;
    try {
      code = process.waitFor();
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    }
    RUNNING.remove(this);
    for (CompletableFuture<JsonNode> f : pending.values()) {
      f.completeExceptionally(new IOException("Hermes exited (code " + code + ")"));
    }
    pending.clear();
    agentRequests.shutdownNow();
    listener.onExit(code);
  }

  private void dispatch(JsonNode msg) {
    JsonNode id = msg.get("id");
    JsonNode method = msg.get("method");
    if (method != null && id != null && !id.isNull()) {
      // A request from the agent: answer it without blocking the reader.
      agentRequests.execute(() -> handleAgentRequest(id, method.asText(), msg.path("params")));
    } else if (method != null) {
      if ("session/update".equals(method.asText())) {
        try {
          listener.onSessionUpdate(msg.path("params").path("update"));
        } catch (RuntimeException e) {
          e.printStackTrace();
        }
      }
    } else if (id != null && id.canConvertToLong()) {
      CompletableFuture<JsonNode> f = pending.remove(id.asLong());
      if (f != null) {
        JsonNode error = msg.get("error");
        if (error != null && !error.isNull()) {
          f.completeExceptionally(new AcpException(error.path("message").asText("ACP error")));
        } else {
          f.complete(msg.path("result"));
        }
      }
    }
  }

  private void handleAgentRequest(JsonNode id, String method, JsonNode params) {
    if ("session/request_permission".equals(method)) {
      String optionId;
      try {
        optionId = listener.onPermissionRequest(params);
      } catch (RuntimeException e) {
        e.printStackTrace();
        optionId = null;
      }
      ObjectNode result = newObject();
      ObjectNode outcome = result.putObject("outcome");
      if (optionId == null) {
        outcome.put("outcome", "cancelled");
      } else {
        outcome.put("outcome", "selected");
        outcome.put("optionId", optionId);
      }
      respond(id, result);
    } else {
      respondError(id, -32601, "Method not supported by 86Duino IDE: " + method);
    }
  }

  // ---------------------------------------------------------------- shutdown

  /** Closes stdin so Hermes shuts down cleanly, then kills it if it does not. */
  public void close() {
    synchronized (this) {
      if (stdin != null) {
        try {
          stdin.close();
        } catch (IOException e) {
          // ignore
        }
        stdin = null;
      }
    }
    if (process == null) {
      return;
    }
    try {
      if (!process.waitFor(5, TimeUnit.SECONDS)) {
        destroy();
      }
    } catch (InterruptedException e) {
      destroy();
      Thread.currentThread().interrupt();
    }
  }

  private void destroy() {
    if (process != null && process.isAlive()) {
      process.destroy();
      try {
        if (!process.waitFor(2, TimeUnit.SECONDS)) {
          process.destroyForcibly();
        }
      } catch (InterruptedException e) {
        process.destroyForcibly();
      }
    }
  }
}
