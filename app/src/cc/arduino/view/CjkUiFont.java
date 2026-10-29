/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.view;

import java.awt.Font;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import javax.swing.UIDefaults;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;

import processing.app.PreferencesData;
import processing.app.helpers.OSUtils;

/**
 * macOS + Chinese UI: replaces Aqua's UI font (Lucida Grande) with PingFang.
 *
 * Java 8 on macOS (Retina) caches a Latin glyph as blank when it is first drawn
 * inside a string that also needs a fallback font for CJK. Lucida Grande has no
 * CJK glyphs, so "允許 AI 執行" drawn first makes every later "A" and "I" in that
 * font size invisible ("API Key" shows as "P Key", a tab "AI" is empty). PingFang
 * contains both Latin and CJK glyphs, so no fallback happens.
 *
 * Only Lucida Grande entries are replaced (size and style kept, so the interface
 * scale still applies); the editor's monospaced fonts are untouched.
 * preferences.txt: gui.font.cjk=auto (default) | off | &lt;font family&gt;
 */
public final class CjkUiFont {

  public static final String PREF = "gui.font.cjk";

  private static final String AQUA_UI_FAMILY = "Lucida Grande";

  private CjkUiFont() {
  }

  /** Call right after the look and feel is set, before any window is created. */
  public static String apply() {
    if (!OSUtils.isMacOS()) {
      return null;
    }
    String pref = PreferencesData.get(PREF, "auto").trim();
    if (pref.equalsIgnoreCase("off")) {
      return null;
    }
    String family = pref.isEmpty() || pref.equalsIgnoreCase("auto") ? familyFor(Locale.getDefault()) : pref;
    if (family == null || !isInstalled(family)) {
      return null;
    }
    UIDefaults lafDefaults = UIManager.getLookAndFeelDefaults();
    List<Object> keys = new ArrayList<>(lafDefaults.keySet());
    for (Object key : keys) {
      Object value = UIManager.get(key);  // also resolves lazy values
      if (value instanceof Font && AQUA_UI_FAMILY.equals(((Font) value).getFamily(Locale.ENGLISH))) {
        Font f = (Font) value;
        UIManager.put(key, new FontUIResource(family, f.getStyle(), f.getSize()));
      }
    }
    return family;
  }

  /** The PingFang variant for a Chinese locale; null for other languages. */
  static String familyFor(Locale locale) {
    if (!"zh".equals(locale.getLanguage())) {
      return null;
    }
    String country = locale.getCountry();
    if ("HK".equals(country)) {
      return "PingFang HK";
    }
    if ("TW".equals(country) || "MO".equals(country) || "Hant".equals(locale.getScript())) {
      return "PingFang TC";
    }
    return "PingFang SC";
  }

  private static boolean isInstalled(String family) {
    // A missing family silently becomes "Dialog".
    return family.equals(new Font(family, Font.PLAIN, 13).getFamily(Locale.ENGLISH));
  }
}
