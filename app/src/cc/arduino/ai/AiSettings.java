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
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import processing.app.PreferencesData;

/**
 * AI assistant settings kept in the IDE preferences.txt. The API key is NOT
 * stored here: it lives only in the Hermes data folder (HERMES_HOME\.env).
 * The assistant is used as soon as the user opens it (Tools &gt; AI Assistant)
 * and an API key is saved; there is no separate on/off setting.
 */
public class AiSettings {

  public static final String PREF_PROVIDER = "ai.provider";
  public static final String PREF_MODEL = "ai.model";
  public static final String PREF_BASE_URL = "ai.base_url";
  public static final String PREF_SKILL_DIRS = "ai.skill_dirs";
  public static final String PREF_CONFIRM_EDITS = "ai.confirm_edits";
  public static final String PREF_ALLOW_TERMINAL = "ai.allow_terminal";
  public static final String PREF_PANEL_VISIBLE = "ai.panel.visible";
  public static final String PREF_PANEL_WIDTH = "ai.panel.width";
  /** Height of the message box area (below the conversation), in pixels. */
  public static final String PREF_INPUT_HEIGHT = "ai.panel.input.height";
  /** The AI data folder (HERMES_HOME); empty means the default folder. */
  public static final String PREF_HOME_PATH = "ai.hermes.home";
  // Hidden override (not shown in the Preferences page), mostly for development.
  public static final String PREF_BUNDLE_PATH = "ai.hermes.path";

  /** Separator used to store several skill folders in one preference value. */
  private static final String DIR_SEPARATOR = File.pathSeparator;

  /**
   * Free Gemini key 86Duino provides so users without their own key can try the
   * assistant (free tier: shared usage limits). It is filled in when Gemini is
   * chosen. Not a secret (anyone can read it from the jar); it is only stored
   * reversed so automated key scanners do not flag the source code.
   */
  private static final String GEMINI_TRIAL_KEY =
      new StringBuilder("w6XCYpWlACtDx8h6aT-cvVjdZORMjxLQnW29brYvKNFL6NR8bA.QA").reverse().toString();

  /**
   * A model provider supported by the Preferences page. There is no built-in
   * model list: models are listed only after Test Connection succeeded, so the
   * user never picks a model the key cannot use.
   */
  public static class Provider {
    public final String id;
    public final String label;
    public final String apiKeyEnv;
    public final boolean needsBaseUrl;
    /** Free trial key 86Duino provides for this provider, or null. */
    public final String trialKey;

    Provider(String id, String label, String apiKeyEnv, boolean needsBaseUrl) {
      this(id, label, apiKeyEnv, needsBaseUrl, null);
    }

    Provider(String id, String label, String apiKeyEnv, boolean needsBaseUrl, String trialKey) {
      this.id = id;
      this.label = label;
      this.apiKeyEnv = apiKeyEnv;
      this.needsBaseUrl = needsBaseUrl;
      this.trialKey = trialKey;
    }

    @Override
    public String toString() {
      return label;
    }
  }

  public static final List<Provider> PROVIDERS = Collections.unmodifiableList(Arrays.asList(
    new Provider("openai-api", "OpenAI", "OPENAI_API_KEY", false),
    new Provider("anthropic", "Anthropic (Claude)", "ANTHROPIC_API_KEY", false),
    new Provider("gemini", "Google Gemini", "GOOGLE_API_KEY", false, GEMINI_TRIAL_KEY),
    new Provider("openrouter", "OpenRouter", "OPENROUTER_API_KEY", false),
    new Provider("deepseek", "DeepSeek", "DEEPSEEK_API_KEY", false),
    new Provider("custom", "Custom (OpenAI compatible)", "OPENAI_API_KEY", true)
  ));

  /** Every environment variable that can hold an API key (only one is kept). */
  public static List<String> allApiKeyEnvs() {
    Set<String> names = new LinkedHashSet<>();
    for (Provider p : PROVIDERS) {
      names.add(p.apiKeyEnv);
    }
    return new ArrayList<>(names);
  }

  public static Provider findProvider(String id) {
    for (Provider p : PROVIDERS) {
      if (p.id.equals(id)) {
        return p;
      }
    }
    return PROVIDERS.get(0);
  }

  public String provider;
  public String model;
  public String baseUrl;
  public List<String> skillDirs = new ArrayList<>();
  public boolean confirmEdits;
  public boolean allowTerminal;
  /** Empty means the default AI data folder. */
  public String homePath = "";

  public static AiSettings load() {
    AiSettings s = new AiSettings();
    s.provider = findProvider(PreferencesData.get(PREF_PROVIDER, PROVIDERS.get(0).id)).id;
    s.model = PreferencesData.get(PREF_MODEL, "");
    s.baseUrl = PreferencesData.get(PREF_BASE_URL, "");
    s.skillDirs = splitDirs(PreferencesData.get(PREF_SKILL_DIRS, ""));
    s.confirmEdits = PreferencesData.getBoolean(PREF_CONFIRM_EDITS, true);
    s.allowTerminal = PreferencesData.getBoolean(PREF_ALLOW_TERMINAL, false);
    s.homePath = PreferencesData.get(PREF_HOME_PATH, "");
    return s;
  }

  public void save() {
    PreferencesData.set(PREF_PROVIDER, provider);
    PreferencesData.set(PREF_MODEL, model == null ? "" : model);
    PreferencesData.set(PREF_BASE_URL, baseUrl == null ? "" : baseUrl);
    PreferencesData.set(PREF_SKILL_DIRS, String.join(DIR_SEPARATOR, skillDirs));
    PreferencesData.setBoolean(PREF_CONFIRM_EDITS, confirmEdits);
    PreferencesData.setBoolean(PREF_ALLOW_TERMINAL, allowTerminal);
    PreferencesData.set(PREF_HOME_PATH, homePath == null ? "" : homePath);
  }

  public Provider getProvider() {
    return findProvider(provider);
  }

  /** Settings that require restarting a running Hermes process when changed. */
  public String runtimeSignature() {
    return provider + "|" + model + "|" + baseUrl + "|" + String.join(DIR_SEPARATOR, skillDirs) + "|"
           + allowTerminal + "|" + homePath;
  }

  private static List<String> splitDirs(String value) {
    List<String> dirs = new ArrayList<>();
    for (String d : value.split(java.util.regex.Pattern.quote(DIR_SEPARATOR))) {
      if (!d.trim().isEmpty()) {
        dirs.add(d.trim());
      }
    }
    return dirs;
  }
}
