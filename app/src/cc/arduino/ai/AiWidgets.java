/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.geom.Line2D;

import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.ButtonModel;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JSplitPane;
import javax.swing.ListCellRenderer;
import javax.swing.plaf.basic.BasicButtonUI;
import javax.swing.plaf.basic.BasicComboBoxUI;
import javax.swing.plaf.basic.BasicGraphicsUtils;
import javax.swing.plaf.basic.BasicSplitPaneDivider;
import javax.swing.plaf.basic.BasicSplitPaneUI;

import processing.app.Theme;

/**
 * Flat buttons, a flat model box and the icons of the AI panel, drawn in the
 * theme's colors (the system look and feel ignores the background color of
 * buttons and combo boxes).
 */
final class AiWidgets {

  private AiWidgets() {
  }

  /** A flat button: its background, a 1-pixel line around it, darker while pressed. */
  static JButton button(String text, Color bg, Color fg, Color line) {
    JButton b = new JButton(text);
    style(b, bg, fg, line);
    return b;
  }

  static void style(AbstractButton b, Color bg, Color fg, Color line) {
    b.setUI(new FlatButtonUI(line));
    b.setBackground(bg);
    b.setForeground(fg);
    b.setOpaque(false);
    b.setFocusPainted(false);
    b.setRolloverEnabled(true);
    b.setBorder(BorderFactory.createEmptyBorder(Theme.scale(4), Theme.scale(10), Theme.scale(4), Theme.scale(10)));
  }

  private static final class FlatButtonUI extends BasicButtonUI {
    private final Color line;

    FlatButtonUI(Color line) {
      this.line = line;
    }

    @Override
    public void paint(Graphics g, JComponent c) {
      AbstractButton b = (AbstractButton) c;
      ButtonModel m = b.getModel();
      Color bg = b.getBackground();
      Color dark = AiTheme.readableOn(bg) == Color.WHITE ? Color.WHITE : Color.BLACK;
      if (m.isPressed() && m.isArmed()) {
        bg = AiTheme.mix(bg, dark, 0.18);
      } else if (m.isRollover() && b.isEnabled()) {
        bg = AiTheme.mix(bg, dark, 0.08);
      }
      g.setColor(bg);
      g.fillRect(0, 0, c.getWidth(), c.getHeight());
      g.setColor(line);
      g.drawRect(0, 0, c.getWidth() - 1, c.getHeight() - 1);
      super.paint(g, c);
    }

    @Override
    protected void paintText(Graphics g, AbstractButton b, Rectangle textRect, String text) {
      // the look and feel draws disabled text etched: fade it instead
      g.setColor(b.isEnabled() ? b.getForeground() : AiTheme.mix(b.getBackground(), b.getForeground(), 0.45));
      g.setFont(b.getFont());
      java.awt.FontMetrics fm = g.getFontMetrics();
      BasicGraphicsUtils.drawStringUnderlineCharAt(g, text, b.getDisplayedMnemonicIndex(),
                                                   textRect.x, textRect.y + fm.getAscent());
    }
  }

  /** Icons drawn in the color of the component's text, so they follow the theme and the size. */
  enum Shape {
    PLAY, STOP, CLOSE, DOWN
  }

  static final class ShapeIcon implements Icon {
    private final Shape shape;
    private final int size;

    ShapeIcon(Shape shape, int size) {
      this.shape = shape;
      this.size = size;
    }

    @Override
    public int getIconWidth() {
      return size;
    }

    @Override
    public int getIconHeight() {
      return size;
    }

    @Override
    public void paintIcon(Component c, Graphics g0, int x, int y) {
      Graphics2D g = (Graphics2D) g0.create();
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      Color fg = c.getForeground();
      if (!c.isEnabled()) {
        fg = AiTheme.mix(c.getBackground(), fg, 0.45);
      }
      g.setColor(fg);
      int s = size;
      switch (shape) {
        case PLAY: {
          // a triangle pointing right, a little narrower than tall
          int w = s * 4 / 5;
          int left = x + (s - w) / 2;
          g.fillPolygon(new Polygon(new int[]{left, left + w, left}, new int[]{y, y + s / 2, y + s}, 3));
          break;
        }
        case STOP: {
          int d = s * 4 / 5;
          g.fillRect(x + (s - d) / 2, y + (s - d) / 2, d, d);
          break;
        }
        case CLOSE: {
          g.setStroke(new BasicStroke(Math.max(1.2f, s / 9f), BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
          // exactly over the icon's square (drawLine moves lines half a pixel right and down)
          g.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL, RenderingHints.VALUE_STROKE_PURE);
          g.draw(new Line2D.Float(x, y, x + s, y + s));
          g.draw(new Line2D.Float(x + s, y, x, y + s));
          break;
        }
        case DOWN: {
          int h = s / 2;
          int top = y + (s - h) / 2;
          g.fillPolygon(new Polygon(new int[]{x, x + s, x + s / 2}, new int[]{top, top, top + h}, 3));
          break;
        }
      }
      g.dispose();
    }
  }

  /** A check box square as tall as the text next to it, in the check box's own colors. */
  static final class CheckIcon implements Icon {
    private final JComponent owner;

    CheckIcon(JComponent owner) {
      this.owner = owner;
    }

    /** About the height of a Chinese character of the owner's font, so it follows the font size. */
    private int size() {
      return Math.max(Theme.scale(8), Math.round(owner.getFont().getSize2D() * 0.9f));
    }

    @Override
    public int getIconWidth() {
      return size();
    }

    @Override
    public int getIconHeight() {
      return size();
    }

    @Override
    public void paintIcon(Component c, Graphics g0, int x, int y) {
      AbstractButton b = (AbstractButton) c;
      ButtonModel m = b.getModel();
      Color bg = b.getBackground();
      Color fg = b.getForeground();
      int s = size();
      Graphics2D g = (Graphics2D) g0.create();
      g.setColor(m.isPressed() && m.isArmed() ? AiTheme.mix(bg, fg, 0.15) : bg);
      g.fillRect(x, y, s, s);
      g.setColor(AiTheme.mix(bg, fg, !b.isEnabled() ? 0.3 : m.isRollover() ? 0.85 : 0.6));
      g.drawRect(x, y, s - 1, s - 1);
      if (m.isSelected()) {
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g.setColor(b.isEnabled() ? fg : AiTheme.mix(bg, fg, 0.45));
        g.setStroke(new BasicStroke(Math.max(1.5f, s / 7f), BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND));
        g.drawPolyline(new int[]{x + s * 22 / 100, x + s * 42 / 100, x + s * 78 / 100},
                       new int[]{y + s * 52 / 100, y + s * 72 / 100, y + s * 28 / 100}, 3);
      }
      g.dispose();
    }
  }

  /** The divider of a split pane as a plain bar of one color. */
  static void styleDivider(JSplitPane split, Color color) {
    split.setUI(new BasicSplitPaneUI() {
      @Override
      public BasicSplitPaneDivider createDefaultDivider() {
        return new BasicSplitPaneDivider(this) {
          @Override
          public void paint(Graphics g) {
            g.setColor(color);
            g.fillRect(0, 0, getWidth(), getHeight());
          }
        };
      }
    });
  }

  /** A flat combo box in its own colors, with a small triangle; its list too. */
  static void styleCombo(JComboBox<?> combo, Color bg, Color fg, Color line) {
    FlatComboUI ui = new FlatComboUI();
    combo.setUI(ui);
    combo.setBackground(bg);
    combo.setForeground(fg);
    combo.setBorder(BorderFactory.createLineBorder(line));
    ui.colorList(bg, fg, line);
  }

  private static final class FlatComboUI extends BasicComboBoxUI {
    /** The list that drops down: the box's colors, the item under the mouse inverted. */
    void colorList(Color bg, Color fg, Color line) {
      listBox.setBackground(bg);
      listBox.setForeground(fg);
      listBox.setSelectionBackground(fg);
      listBox.setSelectionForeground(bg);
      if (popup instanceof JComponent) {
        ((JComponent) popup).setBorder(BorderFactory.createLineBorder(line));
      }
    }

    @Override
    protected JButton createArrowButton() {
      JButton arrow = new JButton(new ShapeIcon(Shape.DOWN, Theme.scale(9)));
      arrow.setUI(new BasicButtonUI());
      arrow.setContentAreaFilled(false);
      arrow.setBorderPainted(false);
      arrow.setFocusable(false);
      arrow.setOpaque(false);
      arrow.setBorder(BorderFactory.createEmptyBorder(0, Theme.scale(4), 0, Theme.scale(6)));
      // the icon takes the combo box's text color
      arrow.setForeground(comboBox.getForeground());
      comboBox.addPropertyChangeListener("foreground", e -> arrow.setForeground(comboBox.getForeground()));
      comboBox.addPropertyChangeListener("enabled", e -> arrow.setEnabled(comboBox.isEnabled()));
      return arrow;
    }

    @Override
    public void paintCurrentValueBackground(Graphics g, Rectangle bounds, boolean hasFocus) {
      g.setColor(comboBox.getBackground());
      g.fillRect(bounds.x, bounds.y, bounds.width, bounds.height);
    }

    @Override
    @SuppressWarnings({"unchecked", "rawtypes"})
    public void paintCurrentValue(Graphics g, Rectangle bounds, boolean hasFocus) {
      // always in the box's own colors (the look and feel uses the selection colors when focused)
      ListCellRenderer renderer = comboBox.getRenderer();
      Component c = renderer.getListCellRendererComponent(listBox, comboBox.getSelectedItem(), -1, false, false);
      c.setFont(comboBox.getFont());
      c.setForeground(comboBox.isEnabled() ? comboBox.getForeground()
                      : AiTheme.mix(comboBox.getBackground(), comboBox.getForeground(), 0.45));
      c.setBackground(comboBox.getBackground());
      // the list that drops down paints its rows with this same renderer:
      // it is see-through here only, or the item under the mouse loses its background
      boolean opaque = c instanceof JComponent && ((JComponent) c).isOpaque();
      if (opaque) {
        ((JComponent) c).setOpaque(false);
      }
      int pad = Theme.scale(4);
      currentValuePane.paintComponent(g, c, comboBox, bounds.x + pad, bounds.y, bounds.width - pad, bounds.height, false);
      if (opaque) {
        ((JComponent) c).setOpaque(true);
      }
    }

    @Override
    public void paint(Graphics g, JComponent c) {
      g.setColor(comboBox.getBackground());
      g.fillRect(0, 0, c.getWidth(), c.getHeight());
      super.paint(g, c);
    }
  }
}
