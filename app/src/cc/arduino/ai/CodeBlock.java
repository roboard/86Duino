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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Insets;
import java.awt.LayoutManager;
import java.awt.Toolkit;
import java.awt.datatransfer.StringSelection;
import java.awt.event.MouseWheelEvent;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.text.DefaultCaret;

import org.fife.ui.rsyntaxtextarea.RSyntaxTextArea;
import org.fife.ui.rsyntaxtextarea.SyntaxConstants;

import processing.app.Theme;

/**
 * A code block of the AI's answer: the code in the editor's colors on a pale
 * background, not wrapped (long lines scroll sideways, Shift + mouse wheel),
 * and over it the language with Copy and, for Arduino / C / C++ code,
 * Insert into Editor. The buttons wait until the whole block has arrived.
 */
final class CodeBlock extends JPanel implements ChatView.Sized {

  private final String lang;
  private final ChatView.CodeActions actions;
  private final JPanel header = new JPanel(new BorderLayout());
  private final JLabel langLabel;
  private final JButton copyButton;
  private final JButton insertButton;
  private final RSyntaxTextArea area = new RSyntaxTextArea();
  private final JScrollPane scroll;
  private String code = "";
  private boolean closed;
  private Timer copiedTimer;

  CodeBlock(String lang, ChatView.CodeActions actions, Font codeFont, Font small) {
    this.lang = lang;
    this.actions = actions;
    setOpaque(false);
    setBorder(BorderFactory.createLineBorder(AiTheme.codeHeaderBg()));

    String style = syntaxStyle(lang);
    if (actions != null) {
      actions.prepare(area, SyntaxConstants.SYNTAX_STYLE_CPLUSPLUS.equals(style)); // the editor's colors
    }
    area.setSyntaxEditingStyle(style);
    area.setEditable(false);
    area.setHighlightCurrentLine(false);
    area.setBracketMatchingEnabled(false);
    area.setCodeFoldingEnabled(false);
    area.setLineWrap(false);
    area.setTabSize(2);
    area.setBackground(AiTheme.codeBg());
    area.setMargin(new Insets(Theme.scale(6), Theme.scale(10), Theme.scale(6), Theme.scale(10)));
    ((DefaultCaret) area.getCaret()).setUpdatePolicy(DefaultCaret.NEVER_UPDATE);

    scroll = new JScrollPane(area, ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER,
                             ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
    scroll.setBorder(null);
    scroll.getViewport().setBackground(AiTheme.codeBg());
    // the wheel scrolls the conversation; Shift + wheel the code sideways
    scroll.setWheelScrollingEnabled(false);
    area.addMouseWheelListener(this::wheel);
    scroll.addMouseWheelListener(this::wheel);

    header.setOpaque(true);
    header.setBackground(AiTheme.codeHeaderBg());
    header.setBorder(BorderFactory.createEmptyBorder(Theme.scale(3), Theme.scale(10), Theme.scale(3), Theme.scale(4)));
    langLabel = new JLabel(label(lang));
    langLabel.setForeground(AiTheme.codeHeaderFg());
    header.add(langLabel, BorderLayout.WEST);
    JPanel buttons = new JPanel(new FlowLayout(FlowLayout.RIGHT, Theme.scale(4), 0));
    buttons.setOpaque(false);
    Color line = AiTheme.mix(AiTheme.codeHeaderBg(), AiTheme.sendBg(), 0.6);
    copyButton = AiWidgets.button(tr("Copy"), AiTheme.codeBg(), AiTheme.mix(AiTheme.sendBg(), Color.BLACK, 0.3), line);
    copyButton.addActionListener(e -> copy());
    buttons.add(copyButton);
    insertButton = AiWidgets.button(tr("Insert into Editor"), AiTheme.sendBg(), AiTheme.sendFg(), line);
    insertButton.addActionListener(e -> {
      if (this.actions != null) {
        this.actions.insert(code);
      }
    });
    insertButton.setVisible(actions != null && insertable(lang));
    buttons.add(insertButton);
    header.add(buttons, BorderLayout.EAST);

    add(header);
    add(scroll);
    setLayout(new LayoutManager() {
      @Override
      public void addLayoutComponent(String name, Component comp) {
      }

      @Override
      public void removeLayoutComponent(Component comp) {
      }

      @Override
      public Dimension preferredLayoutSize(Container parent) {
        return new Dimension(getWidth(), heightAt(getWidth()));
      }

      @Override
      public Dimension minimumLayoutSize(Container parent) {
        return new Dimension(0, 0);
      }

      @Override
      public void layoutContainer(Container parent) {
        Insets in = getInsets();
        int w = Math.max(1, getWidth() - in.left - in.right);
        int hh = header.getPreferredSize().height;
        header.setBounds(in.left, in.top, w, hh);
        scroll.setBounds(in.left, in.top + hh, w, Math.max(1, getHeight() - in.top - in.bottom - hh));
      }
    });
    setFonts(codeFont, small);
    update("", false);
  }

  /** The code so far; closed once the closing ``` (or the end of the answer) arrived. */
  void update(String code, boolean closed) {
    if (!code.equals(this.code)) {
      this.code = code;
      area.setText(code);
    }
    this.closed = closed;
    copyButton.setEnabled(closed);
    insertButton.setEnabled(closed);
  }

  void setFonts(Font codeFont, Font small) {
    area.setFont(codeFont);
    langLabel.setFont(small);
    copyButton.setFont(small);
    insertButton.setFont(small);
    for (JButton b : new JButton[]{copyButton, insertButton}) {
      b.setBorder(BorderFactory.createEmptyBorder(Theme.scale(1), Theme.scale(8), Theme.scale(1), Theme.scale(8)));
    }
    revalidate();
  }

  @Override
  public int heightAt(int width) {
    Insets in = getInsets();
    int inner = width - in.left - in.right;
    Dimension text = area.getPreferredSize();
    int h = in.top + in.bottom + header.getPreferredSize().height + text.height;
    if (text.width > inner) {
      h += scroll.getHorizontalScrollBar().getPreferredSize().height;
    }
    return h;
  }

  @Override
  protected void paintComponent(Graphics g) {
    g.setColor(AiTheme.codeBg());
    g.fillRect(0, 0, getWidth(), getHeight());
  }

  private void copy() {
    Toolkit.getDefaultToolkit().getSystemClipboard().setContents(new StringSelection(code), null);
    copyButton.setText(tr("Copied"));
    if (copiedTimer != null) {
      copiedTimer.stop();
    }
    copiedTimer = new Timer(1500, e -> copyButton.setText(tr("Copy")));
    copiedTimer.setRepeats(false);
    copiedTimer.start();
  }

  private void wheel(MouseWheelEvent e) {
    if (e.isShiftDown() && !e.isControlDown()) {
      JScrollBar bar = scroll.getHorizontalScrollBar();
      if (bar.isVisible()) {
        bar.setValue(bar.getValue() + (int) Math.round(e.getPreciseWheelRotation() * Theme.scale(40)));
      }
      e.consume();
      return;
    }
    Component chat = SwingUtilities.getAncestorOfClass(ChatView.class, this);
    if (chat != null) {
      chat.dispatchEvent(SwingUtilities.convertMouseEvent(e.getComponent(), e, chat));
    }
  }

  // ------------------------------------------------------------------ languages

  /** Arduino, C and C++ code can go into the sketch (and code without a language: usually that). */
  static boolean insertable(String lang) {
    switch (lang) {
      case "":
      case "c":
      case "h":
      case "cpp":
      case "c++":
      case "cc":
      case "cxx":
      case "hpp":
      case "ino":
      case "arduino":
        return true;
      default:
        return false;
    }
  }

  private static String label(String lang) {
    switch (lang) {
      case "":
        return tr("Code");
      case "cpp":
      case "c++":
      case "cc":
      case "cxx":
      case "hpp":
        return "C++";
      case "c":
      case "h":
        return "C";
      case "ino":
      case "arduino":
        return "Arduino";
      default:
        return lang;
    }
  }

  private static String syntaxStyle(String lang) {
    if (insertable(lang)) {
      return SyntaxConstants.SYNTAX_STYLE_CPLUSPLUS;
    }
    switch (lang) {
      case "java":
        return SyntaxConstants.SYNTAX_STYLE_JAVA;
      case "python":
      case "py":
        return SyntaxConstants.SYNTAX_STYLE_PYTHON;
      case "js":
      case "javascript":
        return SyntaxConstants.SYNTAX_STYLE_JAVASCRIPT;
      case "ts":
      case "typescript":
        return SyntaxConstants.SYNTAX_STYLE_TYPESCRIPT;
      case "json":
        return SyntaxConstants.SYNTAX_STYLE_JSON;
      case "xml":
        return SyntaxConstants.SYNTAX_STYLE_XML;
      case "html":
        return SyntaxConstants.SYNTAX_STYLE_HTML;
      case "css":
        return SyntaxConstants.SYNTAX_STYLE_CSS;
      case "bash":
      case "sh":
      case "shell":
      case "zsh":
        return SyntaxConstants.SYNTAX_STYLE_UNIX_SHELL;
      case "bat":
      case "cmd":
      case "batch":
        return SyntaxConstants.SYNTAX_STYLE_WINDOWS_BATCH;
      case "ini":
        return SyntaxConstants.SYNTAX_STYLE_INI;
      case "properties":
        return SyntaxConstants.SYNTAX_STYLE_PROPERTIES_FILE;
      case "makefile":
      case "make":
        return SyntaxConstants.SYNTAX_STYLE_MAKEFILE;
      case "yaml":
      case "yml":
        return SyntaxConstants.SYNTAX_STYLE_YAML;
      case "sql":
        return SyntaxConstants.SYNTAX_STYLE_SQL;
      case "lua":
        return SyntaxConstants.SYNTAX_STYLE_LUA;
      case "cs":
      case "csharp":
        return SyntaxConstants.SYNTAX_STYLE_CSHARP;
      default:
        return SyntaxConstants.SYNTAX_STYLE_NONE;
    }
  }
}
