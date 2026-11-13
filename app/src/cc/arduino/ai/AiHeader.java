/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.Insets;
import java.awt.LayoutManager;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;

import processing.app.Editor;
import processing.app.Theme;

/**
 * The top of the AI panel, lined up with the editor beside it: the title bar
 * as tall as the toolbar (the model's logo, "AI Assistant", the model box,
 * New Chat and the X) and under it the status line as tall as the row of tabs.
 */
final class AiHeader extends JPanel {

  private final Editor editor;
  private final JLabel logo = new JLabel();
  private final JLabel title;
  private final JComboBox<String> models;
  private final JComponent buttons;
  /** Provider and size of the logo shown, so it is drawn only when one changes. */
  private String logoShown;

  AiHeader(Editor editor, JLabel title, JComboBox<String> models, JComponent buttons, StatusLine status) {
    super(new BorderLayout());
    this.editor = editor;
    this.title = title;
    this.models = models;
    this.buttons = buttons;

    JPanel bar = new JPanel(new Row());
    bar.setBackground(AiTheme.headerBg());
    bar.setBorder(BorderFactory.createEmptyBorder(0, Theme.scale(10), 0, Theme.scale(6)));
    bar.add(logo);
    bar.add(title);
    bar.add(models);
    bar.add(buttons);
    add(bar, BorderLayout.NORTH);

    status.setBackground(AiTheme.statusBg());
    status.setForeground(AiTheme.statusFg());
    status.setRowHeight(() -> editor == null ? 0 : editor.getTabRowHeight());
    add(status, BorderLayout.CENTER);
  }

  /**
   * The logo of the provider chosen in Preferences (white, from theme/ai/ of
   * the IDE's lib folder: id.svg); a provider without one gets custom.svg.
   */
  void showProvider(AiSettings.Provider provider) {
    int size = Math.max(Theme.scale(12), Math.round(title.getFont().getSize2D() * 1.4f));
    String id = provider.id;
    if (!Theme.getThemeResource("theme/ai/" + id + ".svg").exists()) {
      id = "custom";
    }
    logo.setToolTipText(provider.label);
    if ((id + "@" + size).equals(logoShown)) {
      return;
    }
    logoShown = id + "@" + size;
    Image image = null;
    if (Theme.getThemeResource("theme/ai/" + id + ".svg").exists()) {
      try {
        image = Theme.getThemeImage("ai/" + id, logo, size, size);
      } catch (RuntimeException e) {
        image = null; // no logo then: the title is enough
      }
    }
    logo.setIcon(image == null ? null : new ImageIcon(image));
    logo.setVisible(image != null);
    revalidate();
    repaint();
  }

  /**
   * Logo, title and model box from the left, the buttons at the right, all
   * centered in the height. The model box gets the room between them, up to
   * its own width: a long model name is cut ("gpt-6.1-…"). With too little
   * room even for a short box, the title goes and the logo stays.
   */
  private final class Row implements LayoutManager {

    private int gap() {
      return Theme.scale(6);
    }

    /** The model box is never squeezed narrower than this while the title shows. */
    private int modelsMin() {
      return Theme.scale(60);
    }

    @Override
    public void addLayoutComponent(String name, Component comp) {
    }

    @Override
    public void removeLayoutComponent(Component comp) {
    }

    @Override
    public Dimension preferredLayoutSize(Container parent) {
      Insets in = parent.getInsets();
      int h = 0;
      int w = in.left + in.right;
      for (Component c : new Component[]{logo, title, models, buttons}) {
        if (c.isVisible()) {
          Dimension d = c.getPreferredSize();
          h = Math.max(h, d.height);
          w += d.width + gap();
        }
      }
      int toolbar = editor == null ? 0 : editor.getToolbarHeight();
      h = Math.max(h + Theme.scale(4), toolbar > 0 ? toolbar : Theme.scale(32));
      return new Dimension(w, h + in.top + in.bottom);
    }

    @Override
    public Dimension minimumLayoutSize(Container parent) {
      return new Dimension(0, preferredLayoutSize(parent).height);
    }

    @Override
    public void layoutContainer(Container parent) {
      Insets in = parent.getInsets();
      int top = in.top;
      int height = parent.getHeight() - in.top - in.bottom;
      int right = parent.getWidth() - in.right;

      Dimension b = buttons.getPreferredSize();
      right -= b.width;
      place(buttons, right, top, height, b.width);
      int limit = right - Theme.scale(10); // the model box keeps away from New Chat

      int x = in.left;
      if (logo.isVisible()) {
        int w = logo.getPreferredSize().width;
        place(logo, x, top, height, w);
        x += w + gap();
      }
      int want = models.isVisible() ? models.getPreferredSize().width : 0;
      int titleWidth = title.getPreferredSize().width;
      boolean titleFits = x + titleWidth + (want > 0 ? Theme.scale(10) + Math.min(want, modelsMin()) : 0) <= limit;
      if (titleFits) {
        place(title, x, top, height, titleWidth);
        x += titleWidth + Theme.scale(10);
      } else {
        title.setBounds(0, 0, 0, 0);
      }
      if (models.isVisible()) {
        place(models, x, top, height, Math.max(0, Math.min(want, limit - x)));
      }
    }

    /** At x, this wide, centered in the height. */
    private void place(Component c, int x, int top, int height, int width) {
      int h = Math.min(height, c.getPreferredSize().height);
      c.setBounds(x, top + (height - h) / 2, width, h);
    }
  }
}
