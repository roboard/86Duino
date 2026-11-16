/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import static processing.app.I18n.tr;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.datatransfer.StringSelection;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.Path2D;
import java.awt.geom.RoundRectangle2D;
import java.util.function.Supplier;

import javax.swing.JComponent;
import javax.swing.SwingUtilities;
import javax.swing.Timer;

import processing.app.Theme;

/**
 * The small copy icon under a message: a click puts the message's text on
 * the clipboard and the icon shows a ✓ for a moment, so you see it worked.
 * Faint, darker under the mouse; its size follows the conversation's small font.
 */
final class CopyButton extends JComponent {

  /** How long the ✓ stays. */
  private static final int CHECK_MS = 1500;

  private final Supplier<String> text;
  private final Timer back = new Timer(CHECK_MS, e -> setCopied(false));
  private boolean hover;
  private boolean copied;

  CopyButton(Supplier<String> text) {
    this.text = text;
    back.setRepeats(false);
    setOpaque(false);
    setToolTipText(tr("Copy"));
    setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
    addMouseListener(new MouseAdapter() {
      @Override
      public void mouseEntered(MouseEvent e) {
        hover = true;
        repaint();
      }

      @Override
      public void mouseExited(MouseEvent e) {
        hover = false;
        repaint();
      }

      @Override
      public void mousePressed(MouseEvent e) {
        if (SwingUtilities.isLeftMouseButton(e)) {
          copy();
        }
      }
    });
  }

  private void copy() {
    try {
      Toolkit.getDefaultToolkit().getSystemClipboard().setContents(new StringSelection(text.get()), null);
    } catch (IllegalStateException e) {
      return; // the clipboard is busy (another program has it): no ✓, click again
    }
    setCopied(true);
    back.restart();
  }

  private void setCopied(boolean copied) {
    this.copied = copied;
    repaint();
  }

  /** Width and height of the drawing: about 2/3 of the small font's size. */
  private int glyph() {
    int size = getFont() != null ? Math.round(getFont().getSize2D() * 0.63f) : Theme.scale(9);
    return Math.max(Theme.scale(7), size);
  }

  private int pad() {
    return Theme.scale(3);
  }

  @Override
  public Dimension getPreferredSize() {
    int size = glyph() + 2 * pad();
    return new Dimension(size, size);
  }

  @Override
  public Dimension getMinimumSize() {
    return getPreferredSize();
  }

  @Override
  protected void paintComponent(Graphics g) {
    Graphics2D g2 = (Graphics2D) g.create();
    try {
      g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      g2.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL, RenderingHints.VALUE_STROKE_PURE);
      float s = glyph();
      float x = (getWidth() - s) / 2f;
      float y = (getHeight() - s) / 2f;
      float line = Math.max(1f, s / 13f);
      g2.setColor(hover || copied ? AiTheme.copyHoverFg() : AiTheme.copyFg());
      if (copied) {
        g2.setStroke(new BasicStroke(line * 1.6f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
        Path2D.Float check = new Path2D.Float();
        check.moveTo(x + s * 0.12f, y + s * 0.54f);
        check.lineTo(x + s * 0.40f, y + s * 0.80f);
        check.lineTo(x + s * 0.90f, y + s * 0.22f);
        g2.draw(check);
        return;
      }
      g2.setStroke(new BasicStroke(line, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
      // two sheets, as copy icons usually are: the one behind up and left, the one in front down and right
      float sheet = s * 0.66f;
      float arc = s * 0.22f;
      float h = line / 2f;
      RoundRectangle2D.Float behind = new RoundRectangle2D.Float(x + h, y + h, sheet - line, sheet - line, arc, arc);
      RoundRectangle2D.Float front = new RoundRectangle2D.Float(x + s - sheet + h, y + s - sheet + h, sheet - line, sheet - line, arc, arc);
      g2.draw(behind);
      Color color = g2.getColor();
      g2.setColor(getParent() != null ? getParent().getBackground() : getBackground());
      g2.fill(front);
      g2.setColor(color);
      g2.draw(front);
    } finally {
      g2.dispose();
    }
  }
}
