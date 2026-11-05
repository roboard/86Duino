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

import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Insets;
import java.awt.LayoutManager;
import java.awt.Rectangle;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import javax.swing.JTextPane;
import javax.swing.JViewport;
import javax.swing.Scrollable;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultCaret;
import javax.swing.text.Document;
import javax.swing.text.JTextComponent;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;

import processing.app.Theme;

/**
 * The conversation: your messages on the right, the AI's on the left, each in
 * a bubble; the AI's tool lines in small text inside its bubble; notes and
 * errors centered between them. Text can be selected inside one bubble.
 */
final class ChatView extends JPanel implements Scrollable {

  /** Width of your messages, part of the conversation's width. */
  private static final double USER_WIDTH = 0.8;
  private static final String ROW = "ai.row";
  private static final String SMALL = "ai.small";

  private enum Row {
    USER_LABEL(SwingConstants.RIGHT, 14), USER(SwingConstants.RIGHT, 3),
    AI_LABEL(SwingConstants.LEFT, 14), AI(SwingConstants.LEFT, 3), AI_MORE(SwingConstants.LEFT, 8),
    NOTE(SwingConstants.CENTER, 8);

    final int align;
    /** Space above the row (none for the first one). */
    final int gap;

    Row(int align, int gap) {
      this.align = align;
      this.gap = gap;
    }
  }

  private Font font;
  private Font small;
  /** Your last message, for the "attached" line. */
  private Bubble lastUser;
  /** The AI's bubble of this answer (null after a note: the next text gets a new one). */
  private Bubble aiBubble;
  /** Its text block that grows while the AI answers, or null. */
  private JTextArea aiText;
  /** The "AI Assistant" label was shown for this answer. */
  private boolean aiLabelShown;

  ChatView(Font font) {
    setLayout(new Rows());
    setBackground(AiTheme.chatBg());
    setBorder(BorderFactory.createEmptyBorder(Theme.scale(6), Theme.scale(10), Theme.scale(10), Theme.scale(10)));
    setFonts(font);
  }

  // ------------------------------------------------------------------ content

  boolean isEmpty() {
    return getComponentCount() == 0;
  }

  void clear() {
    removeAll();
    lastUser = null;
    endAnswer();
    aiLabelShown = false;
    changed(true);
  }

  /** Your message; the AI's answer starts below it. */
  void addUser(String text) {
    boolean follow = true; // your own message is always shown
    closeAi();
    aiBubble = null;
    aiLabelShown = false;
    JLabel label = label(tr("You"));
    add(label, Row.USER_LABEL);
    lastUser = new Bubble(AiTheme.userBg(), AiTheme.faint(AiTheme.userBg(), AiTheme.userAccent(), 0.45),
                          AiTheme.userAccent(), true);
    if (!text.isEmpty()) {
      lastUser.add(text(text, AiTheme.chatFg(), false));
    }
    add(lastUser, Row.USER);
    changed(follow);
  }

  /** A small line under your message (e.g. the compile errors sent with it). */
  void addAttachment(String note) {
    if (lastUser == null) {
      addNote(note, false);
      return;
    }
    boolean follow = atBottom();
    lastUser.add(text(note, AiTheme.faint(AiTheme.userBg(), AiTheme.chatFg(), 0.6), true));
    changed(follow);
  }

  /** Text of the AI's answer; newBlock starts a new paragraph (after a tool line). */
  void appendAi(String text, boolean newBlock) {
    boolean follow = atBottom();
    if (newBlock || aiText == null) {
      closeAi();
      aiText = text(text, AiTheme.chatFg(), false);
      bubble().add(aiText);
    } else {
      aiText.append(text);
    }
    changed(follow);
  }

  /** A tool line, in small text inside the AI's bubble. */
  void addTool(String text) {
    boolean follow = atBottom();
    closeAi();
    aiText = null;
    bubble().add(text(text, AiTheme.faint(AiTheme.assistantBg(), AiTheme.chatFg(), 0.55), true));
    changed(follow);
  }

  /** A note (gray) or an error (red), centered between the messages; returned so it can be taken away. */
  Component addNote(String text, boolean error) {
    boolean follow = atBottom();
    endAnswer();
    JTextPane note = new JTextPane();
    note.setText(text);
    SimpleAttributeSet center = new SimpleAttributeSet();
    StyleConstants.setAlignment(center, StyleConstants.ALIGN_CENTER);
    StyledDocument doc = note.getStyledDocument();
    doc.setParagraphAttributes(0, doc.getLength(), center, false);
    note.setForeground(error ? AiTheme.errorFg() : AiTheme.faint(AiTheme.chatBg(), AiTheme.chatFg(), 0.5));
    readOnly(note);
    note.putClientProperty(SMALL, Boolean.TRUE);
    note.setFont(error ? small : small.deriveFont(Font.ITALIC));
    add(note, Row.NOTE);
    changed(follow);
    return note;
  }

  /** Takes a note away again, one that no longer holds (gone already: nothing happens). */
  void removeNote(Component note) {
    if (note == null || note.getParent() != this) {
      return;
    }
    boolean follow = atBottom();
    remove(note);
    changed(follow);
  }

  /** The AI's bubble of this answer, made (with the label, once per answer) when needed. */
  private Bubble bubble() {
    if (aiBubble == null) {
      Row row = Row.AI_MORE;
      if (!aiLabelShown) {
        add(aiLabel(), Row.AI_LABEL);
        aiLabelShown = true;
        row = Row.AI;
      }
      aiBubble = new Bubble(AiTheme.assistantBg(),
                            AiTheme.faint(AiTheme.assistantBg(), AiTheme.assistantAccent(), 0.25),
                            AiTheme.assistantAccent(), false);
      add(aiBubble, row);
    }
    return aiBubble;
  }

  /** After a note the AI's next text gets a new bubble. */
  private void endAnswer() {
    closeAi();
    aiBubble = null;
    aiText = null;
  }

  /** The text block that was growing ends: the blank lines the AI ends it with are dropped. */
  private void closeAi() {
    if (aiText == null) {
      return;
    }
    Document doc = aiText.getDocument();
    try {
      String s = doc.getText(0, doc.getLength());
      int end = s.length();
      while (end > 0 && Character.isWhitespace(s.charAt(end - 1))) {
        end--;
      }
      if (end < s.length()) {
        doc.remove(end, s.length() - end);
      }
    } catch (BadLocationException e) {
      // cannot happen
    }
    aiText = null;
  }

  private void add(Component c, Row row) {
    ((JComponent) c).putClientProperty(ROW, row);
    add(c);
  }

  private JLabel label(String text) {
    JLabel l = new JLabel(text);
    l.setForeground(AiTheme.faint(AiTheme.chatBg(), AiTheme.chatFg(), 0.5));
    l.putClientProperty(SMALL, Boolean.TRUE);
    l.setFont(small);
    return l;
  }

  /** "AI" in a small badge, then "AI Assistant". */
  private JPanel aiLabel() {
    JPanel p = new JPanel();
    p.setLayout(new BoxLayout(p, BoxLayout.X_AXIS));
    p.setOpaque(false);
    JLabel badge = new JLabel(tr("AI"));
    badge.setOpaque(true);
    badge.setBackground(AiTheme.assistantAccent());
    badge.setForeground(AiTheme.readableOn(AiTheme.assistantAccent()));
    badge.setBorder(BorderFactory.createEmptyBorder(1, Theme.scale(4), 1, Theme.scale(4)));
    badge.putClientProperty(SMALL, Boolean.TRUE);
    badge.putClientProperty("ai.bold", Boolean.TRUE);
    badge.setFont(small.deriveFont(Font.BOLD));
    p.add(badge);
    p.add(Box.createHorizontalStrut(Theme.scale(6)));
    p.add(label(tr("AI Assistant")));
    return p;
  }

  private JTextArea text(String text, Color fg, boolean isSmall) {
    JTextArea t = new JTextArea(text);
    t.setLineWrap(true);
    t.setWrapStyleWord(true);
    t.setForeground(fg);
    readOnly(t);
    if (isSmall) {
      t.putClientProperty(SMALL, Boolean.TRUE);
    }
    t.setFont(isSmall ? small : font);
    return t;
  }

  /** Selectable (Ctrl+C copies), never scrolls the conversation to its caret. */
  private static void readOnly(JTextComponent t) {
    t.setEditable(false);
    t.setOpaque(false);
    t.setBorder(null);
    t.setMargin(new Insets(0, 0, 0, 0));
    ((DefaultCaret) t.getCaret()).setUpdatePolicy(DefaultCaret.NEVER_UPDATE);
  }

  // ------------------------------------------------------------------ font, scrolling

  /** The conversation's font; small text (labels, tool lines, notes) is one point smaller. */
  void setFonts(Font font) {
    this.font = font;
    this.small = font.deriveFont((float) Math.max(8, font.getSize() - 1));
    setFonts(this);
    revalidate();
    repaint();
  }

  private void setFonts(Container parent) {
    for (Component c : parent.getComponents()) {
      if (c instanceof JComponent && ((JComponent) c).getClientProperty(SMALL) != null) {
        JComponent j = (JComponent) c;
        Font f = small;
        if (j.getClientProperty("ai.bold") != null) {
          f = small.deriveFont(Font.BOLD);
        } else if (c instanceof JTextPane && c.getFont().isItalic()) {
          f = small.deriveFont(Font.ITALIC);
        }
        c.setFont(f);
      } else if (c instanceof JTextArea) {
        c.setFont(font);
      }
      if (c instanceof Container) {
        setFonts((Container) c);
      }
    }
  }

  /** The end of the conversation is in view (new text then keeps it in view). */
  private boolean atBottom() {
    if (!(getParent() instanceof JViewport)) {
      return true;
    }
    Rectangle view = ((JViewport) getParent()).getViewRect();
    return view.y + view.height >= getHeight() - Theme.scale(24);
  }

  private void changed(boolean follow) {
    revalidate();
    repaint();
    if (follow) {
      // after the layout: then the height is known
      SwingUtilities.invokeLater(() -> {
        if (getParent() instanceof JViewport) {
          JViewport viewport = (JViewport) getParent();
          viewport.validate();
          int y = Math.max(0, getHeight() - viewport.getExtentSize().height);
          viewport.setViewPosition(new java.awt.Point(0, y));
        }
      });
    }
  }

  @Override
  public Dimension getPreferredScrollableViewportSize() {
    return getPreferredSize();
  }

  @Override
  public int getScrollableUnitIncrement(Rectangle visible, int orientation, int direction) {
    return Theme.scale(16);
  }

  @Override
  public int getScrollableBlockIncrement(Rectangle visible, int orientation, int direction) {
    return Math.max(Theme.scale(16), visible.height - Theme.scale(32));
  }

  @Override
  public boolean getScrollableTracksViewportWidth() {
    return true;
  }

  @Override
  public boolean getScrollableTracksViewportHeight() {
    // fill the viewport while the conversation is short (its background shows)
    return getParent() instanceof JViewport && getParent().getHeight() > getPreferredSize().height;
  }

  // ------------------------------------------------------------------ layout

  /** The height a component needs at this width (wrapped text grows taller when narrower). */
  static int heightAt(Component c, int width) {
    if (c instanceof Bubble) {
      return ((Bubble) c).heightAt(width);
    }
    c.setSize(width, Short.MAX_VALUE);
    return c.getPreferredSize().height;
  }

  /** The rows top to bottom: yours on the right, the AI's on the left, notes across. */
  private final class Rows implements LayoutManager {
    @Override
    public void addLayoutComponent(String name, Component comp) {
    }

    @Override
    public void removeLayoutComponent(Component comp) {
    }

    @Override
    public Dimension preferredLayoutSize(Container parent) {
      int width = parent.getWidth() > 0 ? parent.getWidth() : Theme.scale(300);
      return new Dimension(width, layout(width, false));
    }

    @Override
    public Dimension minimumLayoutSize(Container parent) {
      return new Dimension(0, 0);
    }

    @Override
    public void layoutContainer(Container parent) {
      layout(parent.getWidth(), true);
    }

    private int layout(int width, boolean place) {
      Insets in = getInsets();
      int inner = Math.max(Theme.scale(40), width - in.left - in.right);
      int y = in.top;
      Component[] rows = getComponents();
      for (int i = 0; i < rows.length; i++) {
        Component c = rows[i];
        Row row = (Row) ((JComponent) c).getClientProperty(ROW);
        if (i > 0) {
          y += Theme.scale(row.gap);
        }
        int w;
        switch (row) {
          case USER:
            w = (int) (inner * USER_WIDTH);
            break;
          case USER_LABEL:
          case AI_LABEL:
            w = Math.min(inner, c.getPreferredSize().width);
            break;
          default:
            w = inner;
        }
        int h = heightAt(c, w);
        int x = row.align == SwingConstants.RIGHT ? in.left + inner - w : in.left;
        if (place) {
          c.setBounds(x, y, w, h);
        }
        y += h;
      }
      return y + in.bottom;
    }
  }

  /**
   * A message: its text blocks top to bottom on a background with a thin line
   * around it and a colored stripe on one side.
   */
  static final class Bubble extends JPanel {
    private final Color line;
    private final Color accent;
    private final boolean accentRight;
    private final int stripe = Theme.scale(4);

    Bubble(Color bg, Color line, Color accent, boolean accentRight) {
      this.line = line;
      this.accent = accent;
      this.accentRight = accentRight;
      setBackground(bg);
      setOpaque(false);
      int pad = Theme.scale(8);
      int side = Theme.scale(12);
      setBorder(BorderFactory.createEmptyBorder(pad, side + (accentRight ? 0 : stripe), pad, side + (accentRight ? stripe : 0)));
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
          int inner = Math.max(1, getWidth() - in.left - in.right);
          int y = in.top;
          for (Component c : getComponents()) {
            int h = ChatView.heightAt(c, inner);
            c.setBounds(in.left, y, inner, h);
            y += h + Theme.scale(3);
          }
        }
      });
    }

    int heightAt(int width) {
      Insets in = getInsets();
      int inner = Math.max(1, width - in.left - in.right);
      int h = in.top + in.bottom;
      Component[] blocks = getComponents();
      for (Component c : blocks) {
        h += ChatView.heightAt(c, inner);
      }
      return h + Math.max(0, blocks.length - 1) * Theme.scale(3);
    }

    @Override
    protected void paintComponent(Graphics g) {
      int w = getWidth();
      int h = getHeight();
      g.setColor(getBackground());
      g.fillRect(0, 0, w, h);
      g.setColor(line);
      g.drawRect(0, 0, w - 1, h - 1);
      g.setColor(accent);
      g.fillRect(accentRight ? w - stripe : 0, 0, stripe, h);
    }
  }
}
