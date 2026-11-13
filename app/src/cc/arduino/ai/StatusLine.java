/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.event.MouseEvent;
import java.text.BreakIterator;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.function.IntSupplier;

import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.ToolTipManager;

import processing.app.Theme;

/**
 * The status line under the title bar ("Thinking...", "Reading xxx"): as tall
 * as the editor's row of tabs beside it, the text centered, on two lines when
 * it is long; longer text ends in "…" and shows whole as a tooltip.
 */
final class StatusLine extends JComponent {

  private static final int MAX_LINES = 2;
  private static final String ELLIPSIS = "…";

  private String text = "";
  /** How tall the editor's row of tabs is (0: not known). */
  private IntSupplier rowHeight = () -> 0;
  /** The text did not fit when it was last drawn: the tooltip shows all of it. */
  private boolean cut;

  StatusLine() {
    setOpaque(true);
    setFont(AiTheme.inPanelFont(new JLabel().getFont()));
    setBorder(BorderFactory.createEmptyBorder(0, Theme.scale(10), 0, Theme.scale(10)));
    ToolTipManager.sharedInstance().registerComponent(this);
  }

  void setRowHeight(IntSupplier rowHeight) {
    this.rowHeight = rowHeight;
    revalidate();
  }

  void setText(String text) {
    text = text == null ? "" : text.trim();
    if (!text.equals(this.text)) {
      this.text = text;
      repaint();
    }
  }

  String getText() {
    return text;
  }

  @Override
  public Dimension getPreferredSize() {
    Insets in = getInsets();
    int line = lineHeight(getFontMetrics(getFont())) + in.top + in.bottom;
    int row = rowHeight.getAsInt();
    return new Dimension(0, Math.max(line, row > 0 ? row : Theme.scale(33)));
  }

  @Override
  public String getToolTipText(MouseEvent e) {
    return cut ? text : null;
  }

  @Override
  protected void paintComponent(Graphics g0) {
    Graphics2D g = (Graphics2D) g0.create();
    g.setColor(getBackground());
    g.fillRect(0, 0, getWidth(), getHeight());
    Map<?, ?> hints = (Map<?, ?>) Toolkit.getDefaultToolkit().getDesktopProperty("awt.font.desktophints");
    if (hints != null) {
      g.addRenderingHints(hints);
    } else {
      g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
    }
    g.setFont(getFont());
    g.setColor(getForeground());
    FontMetrics fm = g.getFontMetrics();
    Insets in = getInsets();
    int width = getWidth() - in.left - in.right;
    int room = getHeight() - in.top - in.bottom;
    int step = lineHeight(fm);
    int max = Math.max(1, Math.min(MAX_LINES, room / step));
    List<String> lines = wrap(text, fm, Math.max(1, width));
    cut = lines.size() > max;
    if (cut) {
      lines = new ArrayList<>(lines.subList(0, max));
      lines.set(max - 1, ellipsize(lines.get(max - 1), fm, width));
    }
    int y = in.top + (room - lines.size() * step) / 2 + fm.getAscent();
    for (String line : lines) {
      g.drawString(line, in.left + (width - fm.stringWidth(line)) / 2, y);
      y += step;
    }
    g.dispose();
  }

  /**
   * Lines close together (no leading), so two fit in the row of tabs' height
   * even at 100%.
   */
  private static int lineHeight(FontMetrics fm) {
    return fm.getAscent() + fm.getDescent();
  }

  /** The line with "…" at its end, cut so both fit. */
  private static String ellipsize(String line, FontMetrics fm, int width) {
    String s = line;
    while (!s.isEmpty() && fm.stringWidth(s + ELLIPSIS) > width) {
      s = s.substring(0, s.length() - 1);
    }
    return s.replaceAll("\\s+$", "") + ELLIPSIS;
  }

  /**
   * The text in lines no wider than width: broken between words, or between
   * any two Chinese characters; a word longer than a line is cut anywhere.
   */
  static List<String> wrap(String text, FontMetrics fm, int width) {
    List<String> lines = new ArrayList<>();
    for (String para : text.split("\r?\n")) {
      BreakIterator it = BreakIterator.getLineInstance();
      it.setText(para);
      int start = 0;
      int fit = 0; // end of the longest piece from start that fits
      int end = it.next();
      while (end != BreakIterator.DONE) {
        if (fm.stringWidth(para.substring(start, end).replaceAll("\\s+$", "")) <= width) {
          fit = end;
          end = it.next();
        } else if (fit > start) {
          lines.add(para.substring(start, fit).replaceAll("\\s+$", ""));
          start = fit;
        } else {
          // one word wider than the line
          int cutAt = start + 1;
          while (cutAt < end && fm.stringWidth(para.substring(start, cutAt + 1)) <= width) {
            cutAt++;
          }
          lines.add(para.substring(start, cutAt));
          start = cutAt;
          fit = start;
        }
      }
      String rest = para.substring(start).replaceAll("\\s+$", "");
      if (!rest.isEmpty() || lines.isEmpty()) {
        lines.add(rest);
      }
    }
    return lines;
  }
}
