/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

import processing.app.Sketch;

/**
 * The AI conversation of one sketch, kept in data\.ai\chat.json inside the
 * sketch folder ("Save As" copies the data folder, so the conversation goes
 * along). It holds what the panel showed, for display, and the Hermes session
 * id, so the AI can continue the conversation with its memory.
 */
final class ChatHistory {

  /** Messages kept (yours and the AI's); tool, info and error lines do not count. */
  static final int MAX_MESSAGES = 200;

  static final String USER = "user";
  static final String AI = "ai";
  static final String TOOL = "tool";
  static final String INFO = "info";
  static final String ERROR = "error";

  static final class Entry {
    final String type;
    String text;

    Entry(String type, String text) {
      this.type = type;
      this.text = text;
    }

    boolean isMessage() {
      return USER.equals(type) || AI.equals(type);
    }
  }

  final List<Entry> entries = new ArrayList<>();
  /** The Hermes session to continue, or null. */
  String sessionId;
  /** Provider, model, Base URL and AI workspace the session was made with (see {@link #aiSignature}). */
  String aiSignature = "";
  String sketchFolder = "";
  String board = "";
  /** Last save, in milliseconds. */
  long updated;
  /** Older messages dropped to stay within {@link #MAX_MESSAGES}. */
  int omitted;

  static File fileFor(Sketch sketch) {
    return new File(new File(sketch.getDataFolder(), ".ai"), "chat.json");
  }

  /** What a Hermes session depends on: a session made with other settings is not continued. */
  static String aiSignature(AiSettings s) {
    String model = s.model == null ? "" : s.model.trim();
    String baseUrl = s.getProvider().needsBaseUrl && s.baseUrl != null ? s.baseUrl.trim() : "";
    return s.getProvider().id + "|" + model + "|" + baseUrl + "|" + HermesBundle.getDefault().getHomeDir().getAbsolutePath();
  }

  void clear() {
    entries.clear();
    sessionId = null;
    aiSignature = "";
    omitted = 0;
  }

  /** Drops the oldest messages beyond {@link #MAX_MESSAGES}; true when something was dropped. */
  boolean trim() {
    int messages = 0;
    for (Entry e : entries) {
      if (e.isMessage()) {
        messages++;
      }
    }
    boolean dropped = false;
    while (messages > MAX_MESSAGES && !entries.isEmpty()) {
      if (entries.remove(0).isMessage()) {
        messages--;
        omitted++;
      }
      dropped = true;
    }
    // start with a message, not with tool lines of a message that is gone
    while (dropped && !entries.isEmpty() && !entries.get(0).isMessage()) {
      entries.remove(0);
    }
    return dropped;
  }

  /** Reads a saved conversation; null when there is none or it cannot be read. */
  static ChatHistory load(File file) {
    if (!file.isFile()) {
      return null;
    }
    try {
      JsonNode root = new ObjectMapper().readTree(file);
      ChatHistory h = new ChatHistory();
      h.sessionId = root.hasNonNull("sessionId") ? root.get("sessionId").asText() : null;
      h.aiSignature = root.path("ai").asText("");
      h.sketchFolder = root.path("sketchFolder").asText("");
      h.board = root.path("board").asText("");
      h.updated = root.path("updated").asLong(0);
      h.omitted = root.path("omitted").asInt(0);
      for (JsonNode m : root.path("messages")) {
        String type = m.path("type").asText("");
        if (!type.isEmpty()) {
          h.entries.add(new Entry(type, m.path("text").asText("")));
        }
      }
      return h;
    } catch (IOException | RuntimeException e) {
      System.err.println("Could not read the AI conversation " + file + ": " + e);
      return null;
    }
  }

  /** Writes the conversation (through a temporary file, so a crash never leaves half a file). */
  void save(File file) throws IOException {
    ObjectMapper mapper = new ObjectMapper();
    ObjectNode root = mapper.createObjectNode();
    root.put("version", 1);
    root.put("updated", updated);
    if (sessionId != null) {
      root.put("sessionId", sessionId);
    }
    root.put("ai", aiSignature);
    root.put("sketchFolder", sketchFolder);
    root.put("board", board);
    root.put("omitted", omitted);
    ArrayNode messages = root.putArray("messages");
    for (Entry e : entries) {
      messages.addObject().put("type", e.type).put("text", e.text);
    }
    File dir = file.getParentFile();
    Files.createDirectories(dir.toPath());
    File tmp = new File(dir, file.getName() + ".tmp");
    mapper.writerWithDefaultPrettyPrinter().writeValue(tmp, root);
    Files.move(tmp.toPath(), file.toPath(), StandardCopyOption.REPLACE_EXISTING);
  }

  /** Removes the saved conversation, and the .ai and data folders when nothing else is in them. */
  static void delete(File file) {
    if (file.isFile() && !file.delete()) {
      return;
    }
    File ai = file.getParentFile();
    File[] left = ai.listFiles();
    if (left != null && left.length == 0 && ai.delete()) {
      File data = ai.getParentFile();
      left = data.listFiles();
      if (left != null && left.length == 0) {
        data.delete();
      }
    }
  }
}
