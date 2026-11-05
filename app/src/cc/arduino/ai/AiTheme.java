/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.awt.Color;
import java.awt.Font;

import javax.swing.text.StyleContext;

import processing.app.Theme;

/**
 * Colors of the AI panel, from the IDE theme (theme.txt). Each has an optional
 * ai.* key; without it the color comes from the IDE color the panel sits next
 * to (the toolbar, the editor, the status line, the console), so every theme
 * gets a matching panel.
 */
final class AiTheme {

  private AiTheme() {
  }

  // The IDE colors the panel follows (theme.txt keys), with the default theme's values.
  static Color toolbar() {
    return theme("buttons.bgcolor", 0x6686BA);
  }

  static Color editorBg() {
    return theme("editor.bgcolor", 0xFEFCFF);
  }

  static Color editorFg() {
    return theme("editor.fgcolor", 0x000000);
  }

  static Color notice() {
    return theme("status.notice.bgcolor", 0x7EA3D0);
  }

  static Color console() {
    return theme("console.color", 0x303640);
  }

  static Color lineStatusBg() {
    return theme("linestatus.bgcolor", 0x3C4B78);
  }

  static Color lineStatusFg() {
    return theme("linestatus.color", 0xFFFFFF);
  }

  // ------------------------------------------------------------------ the panel's colors

  /** Title bar and status line. */
  static Color headerBg() {
    return color("ai.header.bgcolor", toolbar());
  }

  static Color headerFg() {
    return color("ai.header.color", readableOn(headerBg()));
  }

  /** The conversation. */
  static Color chatBg() {
    return color("ai.chat.bgcolor", editorBg());
  }

  static Color chatFg() {
    return color("ai.chat.color", editorFg());
  }

  /** Your messages: a light tint of the toolbar color, with a stripe of it on the right. */
  static Color userBg() {
    return color("ai.user.bgcolor", mix(chatBg(), toolbar(), 0.2));
  }

  static Color userAccent() {
    return color("ai.user.accent.color", toolbar());
  }

  /** The AI's messages, with a stripe on the left. */
  static Color assistantBg() {
    return color("ai.assistant.bgcolor", chatBg());
  }

  static Color assistantAccent() {
    return color("ai.assistant.accent.color", lineStatusBg());
  }

  /** The row of "Attach the compile error messages". */
  static Color attachBg() {
    return color("ai.attach.bgcolor", notice());
  }

  /** The message box. */
  static Color inputBg() {
    return color("ai.input.bgcolor", console());
  }

  static Color inputFg() {
    return color("ai.input.color", mix(inputBg(), readableOn(inputBg()), 0.6));
  }

  /** Error lines in the conversation. */
  static Color errorFg() {
    return color("ai.error.color", new Color(0xB0, 0x20, 0x20));
  }

  static Color sendBg() {
    return color("ai.send.bgcolor", toolbar());
  }

  static Color sendFg() {
    return color("ai.send.color", readableOn(sendBg()));
  }

  static Color modelBg() {
    return color("ai.model.bgcolor", lineStatusBg());
  }

  static Color modelFg() {
    return color("ai.model.color", lineStatusFg());
  }

  /** "Model:" in front of the model box, on the message box's background. */
  static Color modelLabelFg() {
    return color("ai.model.label.color", readableOn(inputBg()));
  }

  /** The bar between the editor and the panel: a pale tint of the toolbar color. */
  static Color dividerBg() {
    return color("ai.divider.bgcolor", mix(chatBg(), toolbar(), 0.22));
  }

  // ------------------------------------------------------------------ font

  /**
   * ai.font = family,style,size (like the other fonts of theme.txt), e.g.
   * "Hack,plain,12". The family is any font Java knows: one in java/lib/fonts
   * of the IDE or one of the system. Null when the theme has none or it is
   * mistyped: the panel then keeps the look and feel's fonts.
   */
  private static String[] fontSetting() {
    String value;
    try {
      value = Theme.get("ai.font");
    } catch (RuntimeException e) {
      return null;
    }
    if (value == null) {
      return null;
    }
    String[] parts = value.split(",");
    if (parts.length != 3 || parts[0].trim().isEmpty()) {
      return null;
    }
    return parts;
  }

  /** A font from the system's list; one without Chinese takes the Chinese from the system's text font. */
  private static Font font(String family, int style, int size) {
    return StyleContext.getDefaultStyleContext().getFont(family, style, size);
  }

  /** The conversation and the message box: ai.font, or the given font without one. */
  static Font chatFont(Font fallback) {
    String[] parts = fontSetting();
    if (parts == null) {
      return fallback;
    }
    String styles = parts[1].toLowerCase();
    int style = (styles.contains("bold") ? Font.BOLD : 0) | (styles.contains("italic") ? Font.ITALIC : 0);
    int size;
    try {
      size = (int) Double.parseDouble(parts[2].trim());
    } catch (NumberFormatException e) {
      size = 0;
    }
    if (size < 1) {
      size = fallback.getSize(); // a wrong size: the look and feel's size
    } else {
      size = Theme.scale(size); // like the other fonts of theme.txt
    }
    return font(parts[0].trim(), style, size);
  }

  /** The rest of the panel (title, buttons, labels): the family of ai.font, in their own style and size. */
  static Font inPanelFont(Font f) {
    String[] parts = fontSetting();
    if (parts == null || f == null) {
      return f;
    }
    return font(parts[0].trim(), f.getStyle(), f.getSize());
  }

  // ------------------------------------------------------------------ helpers

  /** Lines, labels and small text: between the background and its text. */
  static Color faint(Color bg, Color fg, double amount) {
    return mix(bg, fg, amount);
  }

  /** a mixed with b: 0 is a, 1 is b. */
  static Color mix(Color a, Color b, double amount) {
    return new Color(mixPart(a.getRed(), b.getRed(), amount), mixPart(a.getGreen(), b.getGreen(), amount),
                     mixPart(a.getBlue(), b.getBlue(), amount));
  }

  private static int mixPart(int a, int b, double amount) {
    return (int) Math.round(a + (b - a) * amount);
  }

  /** White on a dark color, black on a light one. */
  static Color readableOn(Color bg) {
    double luma = 0.299 * bg.getRed() + 0.587 * bg.getGreen() + 0.114 * bg.getBlue();
    return luma < 150 ? Color.WHITE : Color.BLACK;
  }

  private static Color theme(String key, int fallback) {
    return color(key, new Color(fallback));
  }

  /** The theme's color, or the fallback when the key is missing or not a color. */
  private static Color color(String key, Color fallback) {
    try {
      String value = Theme.get(key);
      if (value != null && !value.trim().isEmpty()) {
        Color c = Theme.getColor(key);
        if (c != null) {
          return c;
        }
      }
    } catch (RuntimeException e) {
      // a mistyped color: use the fallback
    }
    return fallback;
  }
}
