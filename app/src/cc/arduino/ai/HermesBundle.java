/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import cc.arduino.Constants;
import processing.app.BaseNoGui;
import processing.app.PreferencesData;
import processing.app.helpers.OSUtils;

/**
 * The portable Hermes Agent bundle shipped in &lt;IDE&gt;\hermes (macOS:
 * 86duino.app/Contents/Java/hermes), and the per-user Hermes data folder
 * (HERMES_HOME). Builds the environment that hermes.cmd / ./hermes would
 * normally set, so python can be started directly (starting it through
 * cmd.exe leaves python.exe running when the IDE closes the pipe).
 */
public class HermesBundle {

  private static final boolean WINDOWS = OSUtils.isWindows();

  private final File bundleDir;
  private final File homeDir;

  public HermesBundle(File bundleDir, File homeDir) {
    this.bundleDir = bundleDir;
    this.homeDir = homeDir;
  }

  /** The bundle next to the IDE and the AI data folder chosen in Preferences. */
  public static HermesBundle getDefault() {
    return forHome(PreferencesData.get(AiSettings.PREF_HOME_PATH, ""));
  }

  /** The bundle next to the IDE with the given data folder (empty = default). */
  public static HermesBundle forHome(String homePath) {
    String bundle = PreferencesData.get(AiSettings.PREF_BUNDLE_PATH, "");
    File bundleDir = bundle.isEmpty() ? BaseNoGui.getContentFile("hermes") : new File(bundle);
    File homeDir = homePath == null || homePath.trim().isEmpty() ? defaultHomeDir() : new File(homePath.trim());
    return new HermesBundle(bundleDir, homeDir);
  }

  /**
   * Windows: %APPDATA%\86Duino\hermes. macOS/Linux: &lt;IDE settings folder&gt;/hermes
   * (~/Library/86Duino15/hermes on macOS). A portable IDE uses &lt;IDE&gt;\portable\hermes.
   */
  public static File defaultHomeDir() {
    if (BaseNoGui.getPortableFolder() != null) {
      return new File(BaseNoGui.getPortableFolder(), "hermes");
    }
    if (!WINDOWS) {
      return new File(BaseNoGui.getSettingsFolder(), "hermes");
    }
    String appData = System.getenv("APPDATA");
    if (appData == null || appData.isEmpty()) {
      appData = System.getProperty("user.home");
    }
    return new File(new File(appData, "86Duino"), "hermes");
  }

  public File getBundleDir() {
    return bundleDir;
  }

  public File getHomeDir() {
    return homeDir;
  }

  public File getPython() {
    if (!WINDOWS) {
      return new File(new File(new File(bundleDir, "python"), "bin"), "python3");
    }
    return new File(new File(bundleDir, "python"), "python.exe");
  }

  public boolean isInstalled() {
    return getPython().isFile() && new File(bundleDir, "hermes_launch.py").isFile();
  }

  public File getLogFile() {
    return new File(new File(homeDir, "logs"), "ide-acp.log");
  }

  /** Copies the bundled default config.yaml to HERMES_HOME on first use. */
  public void ensureHome() throws IOException {
    File config = new File(homeDir, "config.yaml");
    if (config.isFile()) {
      return;
    }
    Files.createDirectories(homeDir.toPath());
    File template = new File(new File(bundleDir, "home-template"), "config.yaml");
    if (template.isFile()) {
      Files.copy(template.toPath(), config.toPath(), StandardCopyOption.REPLACE_EXISTING);
    }
  }

  /** Command line that runs the Hermes CLI with the given arguments. */
  public List<String> command(String... args) {
    List<String> cmd = pythonCommand();
    cmd.add(new File(bundleDir, "hermes_launch.py").getAbsolutePath());
    cmd.addAll(Arrays.asList(args));
    return cmd;
  }

  /**
   * The bundled python in isolated mode. On macOS the bytecode cache goes to
   * HERMES_HOME: the bundle inside the .app may be read-only (App
   * Translocation), and -I ignores PYTHONPYCACHEPREFIX.
   */
  private List<String> pythonCommand() {
    List<String> cmd = new ArrayList<>();
    cmd.add(getPython().getAbsolutePath());
    cmd.add("-I");
    if (!WINDOWS) {
      cmd.add("-X");
      cmd.add("pycache_prefix=" + new File(homeDir, "pycache").getAbsolutePath());
    }
    return cmd;
  }

  /** A process builder for Hermes with the environment hermes.cmd would set. */
  public ProcessBuilder processBuilder(File workingDir, String... args) {
    return withHermesEnvironment(new ProcessBuilder(command(args)), workingDir);
  }

  private ProcessBuilder withHermesEnvironment(ProcessBuilder pb, File workingDir) {
    if (workingDir != null) {
      pb.directory(workingDir);
    }
    Map<String, String> env = pb.environment();
    String bundle = bundleDir.getAbsolutePath();
    env.put("HERMES_BUNDLE_DIR", bundle);
    env.put("HERMES_HOME", homeDir.getAbsolutePath());
    if (WINDOWS) {
      env.put("HERMES_GIT_BASH_PATH", bundle + "\\tools\\git\\bin\\bash.exe");
      String path = env.containsKey("PATH") ? env.get("PATH") : env.getOrDefault("Path", "");
      env.remove("Path");
      env.put("PATH", bundle + "\\tools\\git\\cmd;" + bundle + "\\tools\\ripgrep;" + path);
    } else {
      // No Git/bash in the macOS bundle: Hermes uses the system shell (/bin/zsh, /bin/bash).
      String path = env.getOrDefault("PATH", "/usr/bin:/bin:/usr/sbin:/sbin");
      env.put("PATH", bundle + "/tools/ripgrep:" + path);
      // The bundle inside the .app may be read-only (App Translocation): bytecode cache in HERMES_HOME.
      env.put("PYTHONPYCACHEPREFIX", new File(homeDir, "pycache").getAbsolutePath());
    }
    // Hermes starts helper processes WITHOUT -I (its per-home gateway daemon:
    // "python -m hermes_cli.main gateway run"). Give them the isolation -I gives
    // the main process: no user site-packages, no working folder (the sketch) on
    // sys.path. The -I main process ignores these variables.
    env.put("PYTHONNOUSERSITE", "1");
    env.put("PYTHONSAFEPATH", "1");
    env.put("PYTHONIOENCODING", "utf-8");
    // Do not let a Python installed on this computer leak into the bundle.
    env.remove("PYTHONPATH");
    env.remove("PYTHONHOME");
    applyProxy(env);
    return pb;
  }

  /** Passes the proxy configured in Preferences &gt; Network to Hermes. */
  private static void applyProxy(Map<String, String> env) {
    if (!Constants.PROXY_TYPE_MANUAL.equals(PreferencesData.get(Constants.PREF_PROXY_TYPE))) {
      return;
    }
    String host = PreferencesData.get(Constants.PREF_PROXY_MANUAL_HOSTNAME, "").trim();
    String port = PreferencesData.get(Constants.PREF_PROXY_MANUAL_PORT, "").trim();
    if (host.isEmpty()) {
      return;
    }
    String scheme = "SOCKS".equalsIgnoreCase(PreferencesData.get(Constants.PREF_PROXY_MANUAL_TYPE, "")) ? "socks5" : "http";
    String user = PreferencesData.get(Constants.PREF_PROXY_USERNAME, "");
    String pass = PreferencesData.get(Constants.PREF_PROXY_PASSWORD, "");
    StringBuilder url = new StringBuilder(scheme).append("://");
    if (!user.isEmpty()) {
      url.append(encode(user));
      if (!pass.isEmpty()) {
        url.append(':').append(encode(pass));
      }
      url.append('@');
    }
    url.append(host);
    if (!port.isEmpty()) {
      url.append(':').append(port);
    }
    env.put("HTTPS_PROXY", url.toString());
    env.put("HTTP_PROXY", url.toString());
  }

  private static String encode(String s) {
    try {
      return java.net.URLEncoder.encode(s, "UTF-8");
    } catch (java.io.UnsupportedEncodingException e) {
      return s;
    }
  }

  /** Result of a finished Hermes CLI command. */
  public static class Result {
    public final int exitCode;
    public final String output;

    Result(int exitCode, String output) {
      this.exitCode = exitCode;
      this.output = output;
    }

    public boolean ok() {
      return exitCode == 0;
    }
  }

  /** Runs a Hermes CLI command and waits for it (stdout and stderr merged). */
  public Result run(long timeoutSeconds, String... args) throws IOException, InterruptedException {
    ensureHome();
    ProcessBuilder pb = processBuilder(homeDir, args);
    pb.redirectErrorStream(true);
    Process p = pb.start();
    p.getOutputStream().close();
    ByteArrayOutputStream out = new ByteArrayOutputStream();
    Thread pump = new Thread(() -> copy(p.getInputStream(), out), "hermes-cli-output");
    pump.setDaemon(true);
    pump.start();
    if (!p.waitFor(timeoutSeconds, TimeUnit.SECONDS)) {
      p.destroyForcibly();
      throw new IOException("Hermes did not finish within " + timeoutSeconds + " s: " + String.join(" ", args));
    }
    pump.join(2000);
    return new Result(p.exitValue(), new String(out.toByteArray(), StandardCharsets.UTF_8).trim());
  }

  private static void copy(InputStream in, ByteArrayOutputStream out) {
    byte[] buf = new byte[8192];
    int n;
    try {
      while ((n = in.read(buf)) > 0) {
        synchronized (out) {
          out.write(buf, 0, n);
        }
      }
    } catch (IOException e) {
      // process ended
    }
  }

  /**
   * Writes the IDE settings into this Hermes home in one Python process (the
   * same functions "hermes config set/unset" use; one process per value took
   * about ten seconds). A null or empty apiKey keeps the stored key unless
   * removeKey is set; a new key replaces every stored key, so only one API key
   * is kept at a time.
   */
  public void applySettings(AiSettings s, String apiKey, boolean removeKey) throws IOException, InterruptedException {
    AiSettings.Provider p = s.getProvider();
    com.fasterxml.jackson.databind.ObjectMapper mapper = new com.fasterxml.jackson.databind.ObjectMapper();
    com.fasterxml.jackson.databind.node.ArrayNode ops = mapper.createArrayNode();
    boolean newKey = apiKey != null && !apiKey.trim().isEmpty();
    // the key first: the IDE shows the chat as soon as a key is stored
    if (newKey || removeKey) {
      for (String env : AiSettings.allApiKeyEnvs()) {
        if ((removeKey || !env.equals(p.apiKeyEnv)) && hasApiKey(env)) {
          ops.addArray().add("unset").add(env);
        }
      }
    }
    if (newKey) {
      ops.addArray().add("set").add(p.apiKeyEnv).add(apiKey.trim());
    }
    ops.addArray().add("set").add("model.provider").add(p.id);
    String model = s.model == null ? "" : s.model.trim();
    if (model.isEmpty()) {
      // OK may save before any model was chosen
      ops.addArray().add("unset").add("model.default");
    } else {
      ops.addArray().add("set").add("model.default").add(model);
    }
    String baseUrl = s.baseUrl == null ? "" : s.baseUrl.trim();
    if (p.needsBaseUrl && !baseUrl.isEmpty()) {
      ops.addArray().add("set").add("model.base_url").add(baseUrl);
      // A "custom" endpoint only uses a key named by model.key_env (OPENAI_API_KEY is
      // sent to api.openai.com only); without it Hermes sends "no-key-required".
      ops.addArray().add("set").add("model.key_env").add(p.apiKeyEnv);
    } else {
      ops.addArray().add("unset").add("model.base_url");
      ops.addArray().add("unset").add("model.key_env");
    }
    ops.addArray().add("set").add("skills.external_dirs").add(toYamlList(s.skillDirs));
    addToolsetOps(ops, s);
    ops.addArray().add("set").add("compression.threshold_tokens").add(String.valueOf(COMPRESSION_THRESHOLD_TOKENS));
    runConfigOps(ops);
  }

  /**
   * The tools of the IDE chat. The skill tools come only with a skill folder:
   * without them Hermes leaves the skill list out of the prompt.
   */
  static String acpToolsets(AiSettings s) {
    List<String> sets = new ArrayList<>();
    sets.add("file");
    if (s.allowTerminal) {
      sets.add("terminal");
    }
    if (s.skillDirs != null && !s.skillDirs.isEmpty()) {
      sets.add("skills");
    }
    return "[" + String.join(", ", sets) + "]";
  }

  /**
   * The skill tools can also change skills. Keep the user's skill folders as
   * written: no background skill review, no unattended skill edits, no curator.
   */
  private static void addToolsetOps(com.fasterxml.jackson.databind.node.ArrayNode ops, AiSettings s) {
    ops.addArray().add("set").add("platform_toolsets.acp").add(acpToolsets(s));
    ops.addArray().add("set").add("skills.creation_nudge_interval").add("0");
    ops.addArray().add("set").add("skills.write_approval").add("true");
    ops.addArray().add("set").add("curator.enabled").add("false");
  }

  /**
   * A long conversation is summarized once it reaches this many tokens. Hermes'
   * own limit is half the model's window (about 500,000 tokens for a 1M model),
   * and until then every message re-sends the whole history: slow, costly,
   * and soon over a free key's per-minute limit.
   */
  static final int COMPRESSION_THRESHOLD_TOKENS = 100000;

  /**
   * Writes the IDE's settings into a Hermes home set up before the IDE wrote
   * them: the compression limit and the chat's tools (skill tools included).
   */
  public void ensureIdeConfig(AiSettings s) throws IOException, InterruptedException {
    ensureHome();
    File config = new File(homeDir, "config.yaml");
    String text = config.isFile() ? new String(Files.readAllBytes(config.toPath()), StandardCharsets.UTF_8) : "";
    boolean hasLimit = text.contains("threshold_tokens:");
    boolean hasTools = text.contains("creation_nudge_interval:");
    if (hasLimit && hasTools) {
      return;
    }
    com.fasterxml.jackson.databind.node.ArrayNode ops = new com.fasterxml.jackson.databind.ObjectMapper().createArrayNode();
    if (!hasLimit) {
      ops.addArray().add("set").add("compression.threshold_tokens").add(String.valueOf(COMPRESSION_THRESHOLD_TOKENS));
    }
    if (!hasTools) {
      addToolsetOps(ops, s);
    }
    runConfigOps(ops);
  }

  /** Removes every stored API key from this Hermes home. */
  public void removeApiKeys() throws IOException, InterruptedException {
    com.fasterxml.jackson.databind.node.ArrayNode ops = new com.fasterxml.jackson.databind.ObjectMapper().createArrayNode();
    for (String env : AiSettings.allApiKeyEnvs()) {
      if (hasApiKey(env)) {
        ops.addArray().add("unset").add(env);
      }
    }
    if (ops.size() > 0) {
      runConfigOps(ops);
    }
  }

  /** Runs ["set", key, value] / ["unset", key] operations in one Hermes Python process. */
  private void runConfigOps(com.fasterxml.jackson.databind.node.ArrayNode ops) throws IOException, InterruptedException {
    com.fasterxml.jackson.databind.ObjectMapper mapper = new com.fasterxml.jackson.databind.ObjectMapper();
    ensureHome();
    List<String> cmd = pythonCommand();
    cmd.add("-");
    ProcessBuilder pb = withHermesEnvironment(new ProcessBuilder(cmd), homeDir);
    pb.environment().put("PYTHONDONTWRITEBYTECODE", "1");
    // the values (API key included) travel in the environment, not on the command line
    pb.environment().put("IDE_AI_OPS", mapper.writeValueAsString(ops));
    pb.redirectErrorStream(true);
    String out = runScript(pb, APPLY_SETTINGS_SCRIPT, 60);
    int start = out.lastIndexOf("{\"errors\"");
    if (start < 0) {
      throw new IOException("Could not write the AI settings:\n" + out);
    }
    com.fasterxml.jackson.databind.JsonNode errors = mapper.readTree(out.substring(start)).path("errors");
    if (errors.size() > 0) {
      throw new IOException("Could not write: " + errors.toString() + "\n" + out.substring(0, start).trim());
    }
  }

  /** Runs a Python script given on stdin and returns its output. */
  private static String runScript(ProcessBuilder pb, String script, long timeoutSeconds) throws IOException, InterruptedException {
    Process p = pb.start();
    p.getOutputStream().write(script.getBytes(StandardCharsets.UTF_8));
    p.getOutputStream().close();
    ByteArrayOutputStream out = new ByteArrayOutputStream();
    Thread pump = new Thread(() -> copy(p.getInputStream(), out), "hermes-script-output");
    pump.setDaemon(true);
    pump.start();
    if (!p.waitFor(timeoutSeconds, TimeUnit.SECONDS)) {
      p.destroyForcibly();
      throw new IOException("Hermes did not finish within " + timeoutSeconds + " s");
    }
    pump.join(2000);
    return new String(out.toByteArray(), StandardCharsets.UTF_8).trim();
  }

  private static final String APPLY_SETTINGS_SCRIPT = String.join("\n",
    "import json, os, sys",
    "sys._hermes_pin_default_home = True",
    "import hermes_bootstrap",
    "from hermes_cli.config import set_config_value, unset_config_value",
    "errors = []",
    "for op in json.loads(os.environ['IDE_AI_OPS']):",
    "    try:",
    "        if op[0] == 'set':",
    "            set_config_value(op[1], op[2])",
    "        else:",
    "            unset_config_value(op[1])",
    "    except SystemExit as e:",
    "        # unsetting a value that is not set exits with an error: that is fine",
    "        if op[0] == 'set' and e.code not in (0, None):",
    "            errors.append(op[1])",
    "    except Exception as e:",
    "        errors.append(op[1] + ': ' + str(e))",
    "print(json.dumps({'errors': errors}))",
    "");

  /** The variable name of the stored API key, or null when none is stored. */
  public String storedApiKeyEnv() {
    for (String env : AiSettings.allApiKeyEnvs()) {
      if (hasApiKey(env)) {
        return env;
      }
    }
    return null;
  }

  /** Changes whenever the stored key changes, without revealing it. */
  public String apiKeyFingerprint(String envName) {
    String v = readEnvValue(envName);
    return v == null ? "" : Integer.toHexString(v.hashCode());
  }

  /**
   * A YAML flow list of single-quoted strings. Double quotes are avoided on
   * purpose: Java does not escape them when it builds a Windows command line.
   */
  private static String toYamlList(List<String> values) {
    StringBuilder sb = new StringBuilder("[");
    for (String v : values) {
      if (sb.length() > 1) {
        sb.append(", ");
      }
      sb.append('\'').append(v.replace('\\', '/').replace("\"", "").replace("'", "''")).append('\'');
    }
    return sb.append(']').toString();
  }

  /** True when HERMES_HOME\.env already holds a non-empty value for the key. */
  public boolean hasApiKey(String envName) {
    File env = new File(homeDir, ".env");
    if (!env.isFile()) {
      return false;
    }
    try {
      for (String line : Files.readAllLines(env.toPath(), StandardCharsets.UTF_8)) {
        String l = line.trim();
        if (l.startsWith(envName + "=")) {
          String v = l.substring(envName.length() + 1).trim();
          return !v.isEmpty() && !v.equals("\"\"") && !v.equals("''");
        }
      }
    } catch (IOException e) {
      // treat as missing
    }
    return false;
  }

  /**
   * Sends one short prompt with the given settings, using a temporary Hermes
   * home so the user's real settings are not changed before they press OK.
   * The stored key of the real home is used when apiKey is empty.
   */
  public Result testConnection(AiSettings s, String apiKey) throws IOException, InterruptedException {
    File tmpHome = Files.createTempDirectory("hermes-test").toFile();
    // holds the key in .env: also removed if the IDE quits during the test
    cc.arduino.files.DeleteFilesOnShutdown.add(tmpHome);
    try {
      HermesBundle tmp = new HermesBundle(bundleDir, tmpHome);
      String key = apiKey;
      if (key == null || key.trim().isEmpty()) {
        key = readEnvValue(s.getProvider().apiKeyEnv);
      }
      if (key == null || key.trim().isEmpty()) {
        return new Result(1, "No API key");
      }
      tmp.applySettings(s, key, false);
      return tmp.run(180, "-z", "Reply with exactly: OK");
    } finally {
      deleteTree(tmpHome);
    }
  }

  /** Result of {@link #listModels}. */
  public static class ModelList {
    /** True when the provider answered (the models listed are the ones this key can use). */
    public final boolean live;
    public final List<String> models;
    /** The model to pick automatically (newest regular chat model), or "" when none. */
    public final String recommended;
    public final String error;

    ModelList(boolean live, List<String> models, String recommended, String error) {
      this.live = live;
      this.models = models;
      this.recommended = recommended;
      this.error = error;
    }
  }

  /**
   * Asks the provider which chat models the key can use and returns the first
   * ten, newest first (release dates from models.dev; a custom endpoint keeps
   * the server's order). Nothing is listed without a key or when the provider
   * cannot be reached: a built-in list could offer models the key cannot use.
   */
  public ModelList listModels(String provider, String apiKey, String baseUrl) throws IOException, InterruptedException {
    File tmpHome = Files.createTempDirectory("hermes-models").toFile();
    cc.arduino.files.DeleteFilesOnShutdown.add(tmpHome);
    try {
      File template = new File(new File(bundleDir, "home-template"), "config.yaml");
      if (template.isFile()) {
        Files.copy(template.toPath(), new File(tmpHome, "config.yaml").toPath());
      }
      HermesBundle tmp = new HermesBundle(bundleDir, tmpHome);
      List<String> cmd = tmp.pythonCommand();
      cmd.add("-");
      ProcessBuilder pb = tmp.withHermesEnvironment(new ProcessBuilder(cmd), tmpHome);
      Map<String, String> env = pb.environment();
      env.put("PYTHONDONTWRITEBYTECODE", "1");
      env.put("IDE_AI_PROVIDER", provider);
      env.put("IDE_AI_KEY", apiKey == null ? "" : apiKey.trim());
      env.put("IDE_AI_BASE_URL", baseUrl == null ? "" : baseUrl.trim());
      // stderr (warnings) is merged; the JSON answer is the last line
      pb.redirectErrorStream(true);
      Process p = pb.start();
      // The script goes through stdin: no quoting problems on the command line.
      p.getOutputStream().write(LIST_MODELS_SCRIPT.getBytes(StandardCharsets.UTF_8));
      p.getOutputStream().close();
      ByteArrayOutputStream out = new ByteArrayOutputStream();
      Thread pump = new Thread(() -> copy(p.getInputStream(), out), "hermes-models-output");
      pump.setDaemon(true);
      pump.start();
      if (!p.waitFor(60, TimeUnit.SECONDS)) {
        p.destroyForcibly();
        throw new IOException("Listing the models took too long");
      }
      pump.join(2000);
      String text = new String(out.toByteArray(), StandardCharsets.UTF_8).trim();
      int start = text.lastIndexOf("{\"live\"");
      if (start < 0) {
        throw new IOException("Unexpected answer: " + text);
      }
      com.fasterxml.jackson.databind.JsonNode json = new com.fasterxml.jackson.databind.ObjectMapper().readTree(text.substring(start));
      List<String> models = new ArrayList<>();
      for (com.fasterxml.jackson.databind.JsonNode m : json.path("models")) {
        models.add(m.asText());
      }
      return new ModelList(json.path("live").asBoolean(), models, json.path("recommended").asText(""),
                           json.path("error").asText(""));
    } finally {
      deleteTree(tmpHome);
    }
  }

  private static final String LIST_MODELS_SCRIPT = String.join("\n",
    "import json, os, re, sys, urllib.error, urllib.request",
    "sys._hermes_pin_default_home = True",
    "import hermes_bootstrap",
    "from hermes_cli.models import provider_model_ids",
    "provider = os.environ.get('IDE_AI_PROVIDER', 'openai-api')",
    "key = os.environ.get('IDE_AI_KEY', '')",
    "base = os.environ.get('IDE_AI_BASE_URL', '').rstrip('/')",
    "ENDPOINTS = {",
    "    'openai-api': 'https://api.openai.com/v1/models',",
    "    'anthropic': 'https://api.anthropic.com/v1/models?limit=100',",
    "    'gemini': 'https://generativelanguage.googleapis.com/v1beta/models?pageSize=200',",
    "    'openrouter': 'https://openrouter.ai/api/v1/models',",
    "    'deepseek': 'https://api.deepseek.com/models',",
    "}",
    "# Non-chat models of the public providers. A custom endpoint is the user's own server:",
    "# everything it lists is shown (e.g. qwen3-30b-a3b-instruct IS a chat model there).",
    "NOT_CHAT = re.compile(r'embed|whisper|tts|dall-e|image|moderation|audio|realtime|transcribe|search|'",
    "                      r'babbage|davinci|computer-use|sora|veo|imagen|aqa|^gpt-3\\.5-turbo-instruct', re.I)",
    "def live_models():",
    "    url = (base + '/models') if provider == 'custom' else ENDPOINTS.get(provider)",
    "    if not url or (not key and provider != 'openrouter'):",
    "        return None",
    "    headers = {'User-Agent': '86Duino-IDE'}",
    "    if provider == 'anthropic':",
    "        headers.update({'x-api-key': key, 'anthropic-version': '2023-06-01'})",
    "    elif provider == 'gemini':",
    "        headers['x-goog-api-key'] = key",
    "    elif key:",
    "        headers['Authorization'] = 'Bearer ' + key",
    "    try:",
    "        with urllib.request.urlopen(urllib.request.Request(url, headers=headers), timeout=20) as r:",
    "            data = json.load(r)",
    "    except urllib.error.HTTPError as e:",
    "        # the endpoint's own message (e.g. LiteLLM's), not just 'HTTP Error 400'",
    "        body = e.read().decode('utf-8', 'replace')",
    "        try:",
    "            err = json.loads(body).get('error', body)",
    "            body = err.get('message', body) if isinstance(err, dict) else str(err)",
    "        except ValueError:",
    "            pass",
    "        raise RuntimeError('HTTP %s: %s' % (e.code, body.strip()[:300]))",
    "    out, seen = [], set()",
    "    for m in data.get('data') or data.get('models') or []:",
    "        mid = m.get('id') or m.get('name', '')",
    "        if mid.startswith('models/'):",
    "            mid = mid[len('models/'):]",
    "        if provider == 'gemini' and 'generateContent' not in m.get('supportedGenerationMethods', ['generateContent']):",
    "            continue",
    "        if not mid or mid in seen or (provider != 'custom' and NOT_CHAT.search(mid)):",
    "            continue",
    "        seen.add(mid)",
    "        # the provider's own date, used when models.dev does not know the model",
    "        created = m.get('created') or m.get('created_at') or ''",
    "        if isinstance(created, (int, float)):",
    "            import datetime",
    "            created = datetime.datetime.fromtimestamp(created, datetime.timezone.utc).strftime('%Y-%m-%d')",
    "        out.append((mid, str(created)[:10]))",
    "    return out",
    "def models_dev_dates():",
    "    # release dates from models.dev (public, no key needed)",
    "    try:",
    "        from agent.models_dev import fetch_models_dev, PROVIDER_TO_MODELS_DEV",
    "        data = fetch_models_dev() or {}",
    "        entry = data.get(PROVIDER_TO_MODELS_DEV.get(provider, provider)) or {}",
    "        dates = {}",
    "        for mid, info in (entry.get('models') or {}).items():",
    "            d = (info or {}).get('release_date') or (info or {}).get('last_updated')",
    "            if d:",
    "                dates[mid] = str(d)[:10]",
    "        return dates",
    "    except Exception:",
    "        return {}",
    "# variants that should not be picked automatically",
    "SPECIAL = re.compile(r'preview|(^|[-_.])exp(erimental)?([-_.]|$)|vision|audio|realtime|tts|image|search|'",
    "                     r'transcribe|embed', re.I)",
    "error = ''",
    "try:",
    "    live = live_models()",
    "except Exception as e:",
    "    live, error = None, str(e)",
    "models, recommended = [], ''",
    "if live and provider == 'custom':",
    "    models = [m for m, _ in live]  # the server's own order",
    "    recommended = models[0]",
    "elif live:",
    "    try:",
    "        catalog = provider_model_ids(provider) or []  # only to rank models of the same date",
    "    except Exception:",
    "        catalog = []",
    "    rank = {m: i for i, m in enumerate(catalog)}",
    "    dates = models_dev_dates()",
    "    items = sorted(live, key=lambda it: (rank.get(it[0], 10 ** 6), it[0]))",
    "    # newest first; models without any date last (the sort is stable)",
    "    items = sorted(items, key=lambda it: dates.get(it[0]) or it[1] or '', reverse=True)",
    "    models = [m for m, _ in items]",
    "    recommended = next((m for m in models if not SPECIAL.search(m)), models[0])",
    "    if provider == 'gemini':",
    "        # a fast, low-cost 'flash' model (also what the free trial key can use): newest first,",
    "        # a full flash before a flash-lite; no aliases (-latest) or omni/special variants",
    "        flash = [m for m in models if 'flash' in m and not SPECIAL.search(m)",
    "                 and 'latest' not in m and 'omni' not in m]",
    "        full = [m for m in flash if 'lite' not in m]",
    "        recommended = (full or flash or [recommended])[0]",
    "top = models[:10]",
    "if recommended and recommended not in top:",
    "    top = [recommended] + top[:9]",
    "print(json.dumps({'live': bool(live), 'models': top, 'recommended': recommended, 'error': error}))",
    "");

  /** The stored value of a key in HERMES_HOME\.env (never shown to the user). */
  String readEnvValue(String envName) {
    File env = new File(homeDir, ".env");
    try {
      for (String line : Files.readAllLines(env.toPath(), StandardCharsets.UTF_8)) {
        String l = line.trim();
        if (l.startsWith(envName + "=")) {
          String v = l.substring(envName.length() + 1).trim();
          if (v.length() >= 2 && (v.startsWith("\"") && v.endsWith("\"") || v.startsWith("'") && v.endsWith("'"))) {
            v = v.substring(1, v.length() - 1);
          }
          return v;
        }
      }
    } catch (IOException e) {
      // missing
    }
    return null;
  }

  private static void deleteTree(File f) {
    File[] children = f.listFiles();
    if (children != null) {
      for (File c : children) {
        deleteTree(c);
      }
    }
    f.delete();
  }
}
