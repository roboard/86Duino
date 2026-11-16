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
import java.awt.Component;
import java.awt.Container;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.LayoutManager;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JTextPane;
import javax.swing.text.BadLocationException;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;

import processing.app.Theme;

/**
 * A block of the AI's answer, written in Markdown and shown formatted: text
 * with bold, italic, `code` and links; headings; lists; quotes; rules; tables;
 * code blocks with Copy and Insert into Editor. While the answer arrives the
 * Markdown is read again each time, and only the blocks that changed (usually
 * the last one) are made again.
 */
final class MarkdownView extends JPanel implements ChatView.Sized {

  /** The link of a piece of text (an attribute of the text). */
  private static final Object LINK = new Object() {
    @Override
    public String toString() {
      return "ai.link";
    }
  };

  private final ChatView chat;
  private final StringBuilder raw = new StringBuilder();
  /** The whole block arrived: an open code block is complete too. */
  private boolean finished;
  /** The blocks shown, one component each, in order. */
  private final List<Markdown.Block> shown = new ArrayList<>();
  private Font font;
  private Font small;
  private Font codeFont;

  MarkdownView(ChatView chat, Font font, Font small, Font codeFont) {
    this.chat = chat;
    this.font = font;
    this.small = small;
    this.codeFont = codeFont;
    setOpaque(false);
    setLayout(new Stack());
  }

  void append(String text) {
    raw.append(text);
  }

  /** The Markdown as the AI wrote it (what the copy icon copies). */
  String source() {
    return raw.toString();
  }

  /** The block has ended: the blank lines the AI ends it with are dropped. */
  void finish() {
    int end = raw.length();
    while (end > 0 && Character.isWhitespace(raw.charAt(end - 1))) {
      end--;
    }
    raw.setLength(end);
    finished = true;
    render();
  }

  void setFonts(Font font, Font small, Font codeFont) {
    this.font = font;
    this.small = small;
    this.codeFont = codeFont;
    // everything again: list indents and headings are measured in the font
    removeAll();
    shown.clear();
    render();
  }

  /** Shows the Markdown as it is now. */
  void render() {
    List<Markdown.Block> blocks = Markdown.parse(raw.toString());
    if (finished) {
      for (Markdown.Block b : blocks) {
        b.closed = true; // the answer ended without the closing ```
      }
    }
    int same = 0;
    while (same < shown.size() && same < blocks.size() && blocks.get(same).sameAs(shown.get(same))) {
      same++;
    }
    // a code block that only grew: its text is replaced, the block stays (and keeps its scroll)
    if (same < shown.size() && same < blocks.size()) {
      Markdown.Block was = shown.get(same);
      Markdown.Block now = blocks.get(same);
      if (was.kind == Markdown.Kind.CODE && now.kind == Markdown.Kind.CODE && was.lang.equals(now.lang)) {
        ((CodeBlock) getComponent(same)).update(now.code, now.closed);
        shown.set(same, now);
        same++;
      }
    }
    while (getComponentCount() > same) {
      remove(getComponentCount() - 1);
    }
    while (shown.size() > same) {
      shown.remove(shown.size() - 1);
    }
    for (int i = same; i < blocks.size(); i++) {
      add(make(blocks.get(i)));
      shown.add(blocks.get(i));
    }
    revalidate();
    repaint();
  }

  // ------------------------------------------------------------------ blocks

  private Component make(Markdown.Block b) {
    switch (b.kind) {
      case CODE: {
        CodeBlock code = new CodeBlock(b.lang, chat.codeActions(), codeFont, small);
        code.update(b.code, b.closed);
        return code;
      }
      case QUOTE: {
        JTextPane quote = text(b.paras, AiTheme.faint(AiTheme.assistantBg(), AiTheme.chatFg(), 0.7));
        quote.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createMatteBorder(0, Theme.scale(3), 0, 0, AiTheme.markLine()),
            BorderFactory.createEmptyBorder(0, Theme.scale(10), 0, 0)));
        return quote;
      }
      case RULE:
        return new Rule();
      case TABLE:
        return new Table(b.rows);
      default:
        return text(b.paras, AiTheme.chatFg());
    }
  }

  /** Paragraphs, headings and list items in one text pane (selectable together). */
  private JTextPane text(List<Markdown.Para> paras, Color fg) {
    JTextPane pane = new JTextPane();
    ChatView.readOnly(pane);
    pane.setFont(font);
    pane.setForeground(fg);
    StyledDocument doc = pane.getStyledDocument();
    FontMetrics fm = pane.getFontMetrics(font);
    Markdown.Para before = null;
    try {
      for (Markdown.Para p : paras) {
        if (before != null) {
          doc.insertString(doc.getLength(), "\n", plain(font.getSize2D(), false, fg));
        }
        int start = doc.getLength();
        float size = font.getSize2D();
        boolean bold = false;
        if (p.kind == Markdown.ParaKind.HEADING) {
          size *= p.level == 1 ? 1.4f : p.level == 2 ? 1.25f : p.level == 3 ? 1.12f : 1f;
          bold = true;
        }
        String marker = p.kind == Markdown.ParaKind.ITEM ? p.marker + " " : "";
        if (!marker.isEmpty()) {
          doc.insertString(doc.getLength(), marker, plain(size, false, fg));
        }
        for (Markdown.Run r : Markdown.inline(p.text.toString())) {
          doc.insertString(doc.getLength(), r.text, attributes(r, size, bold, fg));
        }
        // Every paragraph gets all of its indents: a new line takes the
        // previous paragraph's ones otherwise (a list item's after a list).
        boolean item = p.kind == Markdown.ParaKind.ITEM;
        int markerWidth = item ? fm.stringWidth(marker) : 0;
        // the whole paragraph (its kept line breaks too): the list indent
        SimpleAttributeSet whole = new SimpleAttributeSet();
        StyleConstants.setLeftIndent(whole, item ? Theme.scale(4) + p.level * Theme.scale(16) + markerWidth : 0);
        StyleConstants.setFirstLineIndent(whole, 0);
        StyleConstants.setSpaceAbove(whole, 0);
        doc.setParagraphAttributes(start, doc.getLength() - start, whole, false);
        // its first line: starts at the marker, with some space above
        SimpleAttributeSet first = new SimpleAttributeSet();
        StyleConstants.setFirstLineIndent(first, -markerWidth);
        if (before != null) {
          boolean listGoesOn = item && before.kind == Markdown.ParaKind.ITEM;
          StyleConstants.setSpaceAbove(first, Theme.scale(listGoesOn ? 2 : p.kind == Markdown.ParaKind.HEADING ? 10 : 6));
        }
        doc.setParagraphAttributes(start, 0, first, false);
        before = p;
      }
    } catch (BadLocationException e) {
      // cannot happen: always at the end
    }
    links(pane);
    return pane;
  }

  private SimpleAttributeSet plain(float size, boolean bold, Color fg) {
    return attributes(new Markdown.Run("", 0, null), size, bold, fg);
  }

  private SimpleAttributeSet attributes(Markdown.Run r, float size, boolean bold, Color fg) {
    SimpleAttributeSet a = new SimpleAttributeSet();
    boolean code = (r.style & Markdown.CODE) != 0;
    StyleConstants.setFontFamily(a, code ? codeFont.getFamily() : font.getFamily());
    StyleConstants.setFontSize(a, Math.round(code ? size * 0.95f : size));
    StyleConstants.setBold(a, bold || (r.style & Markdown.BOLD) != 0);
    StyleConstants.setItalic(a, (r.style & Markdown.ITALIC) != 0);
    StyleConstants.setStrikeThrough(a, (r.style & Markdown.STRIKE) != 0);
    StyleConstants.setForeground(a, r.url != null ? AiTheme.linkFg() : fg);
    if (code) {
      StyleConstants.setBackground(a, AiTheme.inlineCodeBg());
    }
    if (r.url != null) {
      StyleConstants.setUnderline(a, true);
      a.addAttribute(LINK, r.url);
    }
    return a;
  }

  /** A click on a link opens it in the browser; the mouse shows a hand over it. */
  private static void links(JTextPane pane) {
    MouseAdapter m = new MouseAdapter() {
      private String linkAt(MouseEvent e) {
        int pos = pane.viewToModel(e.getPoint());
        if (pos < 0) {
          return null;
        }
        Object url = pane.getStyledDocument().getCharacterElement(pos).getAttributes().getAttribute(LINK);
        return url instanceof String ? (String) url : null;
      }

      @Override
      public void mouseClicked(MouseEvent e) {
        String url = linkAt(e);
        if (url != null && e.getButton() == MouseEvent.BUTTON1 && pane.getSelectionStart() == pane.getSelectionEnd()) {
          processing.app.Base.openURL(url);
        }
      }

      @Override
      public void mouseMoved(MouseEvent e) {
        pane.setCursor(linkAt(e) != null ? Cursor.getPredefinedCursor(Cursor.HAND_CURSOR)
                                         : Cursor.getPredefinedCursor(Cursor.TEXT_CURSOR));
      }
    };
    pane.addMouseListener(m);
    pane.addMouseMotionListener(m);
  }

  // ------------------------------------------------------------------ layout

  private static final int GAP = 6;

  @Override
  public int heightAt(int width) {
    int h = 0;
    Component[] blocks = getComponents();
    for (Component c : blocks) {
      h += ChatView.heightAt(c, width);
    }
    return h + Math.max(0, blocks.length - 1) * Theme.scale(GAP);
  }

  /** The blocks top to bottom, as wide as the bubble. */
  private final class Stack implements LayoutManager {
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
      int w = getWidth();
      int y = 0;
      for (Component c : getComponents()) {
        int h = ChatView.heightAt(c, w);
        c.setBounds(0, y, w, h);
        y += h + Theme.scale(GAP);
      }
    }
  }

  /** --- : a thin line across. */
  private static final class Rule extends JComponent {
    Rule() {
      setPreferredSize(new Dimension(1, Theme.scale(9)));
    }

    @Override
    protected void paintComponent(Graphics g) {
      g.setColor(AiTheme.markLine());
      g.fillRect(0, getHeight() / 2, getWidth(), 1);
    }
  }

  /** A table: the header row bold on a tint, lines around the cells; the columns share the width. */
  private final class Table extends JPanel implements ChatView.Sized {
    private final int cols;
    private final int pad = Theme.scale(6);
    private final List<JTextPane[]> cells = new ArrayList<>();
    private final int[] natural;

    Table(List<List<String>> rows) {
      setOpaque(false);
      int n = 0;
      for (List<String> r : rows) {
        n = Math.max(n, r.size());
      }
      cols = n;
      natural = new int[cols];
      for (int r = 0; r < rows.size(); r++) {
        JTextPane[] line = new JTextPane[cols];
        for (int c = 0; c < cols; c++) {
          String cell = c < rows.get(r).size() ? rows.get(r).get(c) : "";
          Markdown.Para p = new Markdown.Para(Markdown.ParaKind.TEXT, 0, "", cell);
          List<Markdown.Para> one = new ArrayList<>();
          one.add(p);
          JTextPane pane = text(one, AiTheme.chatFg());
          if (r == 0) {
            StyledDocument doc = pane.getStyledDocument();
            SimpleAttributeSet b = new SimpleAttributeSet();
            StyleConstants.setBold(b, true);
            doc.setCharacterAttributes(0, doc.getLength(), b, false);
          }
          line[c] = pane;
          add(pane);
          FontMetrics fm = pane.getFontMetrics(r == 0 ? font.deriveFont(Font.BOLD) : font);
          natural[c] = Math.max(natural[c], fm.stringWidth(plainText(pane)) + 2 * pad + 2);
        }
        cells.add(line);
      }
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
          int[] w = widths(getWidth());
          int y = 0;
          for (JTextPane[] line : cells) {
            int h = rowHeight(line, w);
            int x = 0;
            for (int c = 0; c < cols; c++) {
              line[c].setBounds(x + pad, y + pad / 2, w[c] - 2 * pad, h - pad);
              x += w[c];
            }
            y += h;
          }
        }
      });
    }

    private String plainText(JTextPane pane) {
      try {
        return pane.getDocument().getText(0, pane.getDocument().getLength());
      } catch (BadLocationException e) {
        return "";
      }
    }

    /** Each column as wide as its text when that fits; otherwise all narrower, the wide ones the most. */
    private int[] widths(int width) {
      int[] w = natural.clone();
      int total = 0;
      for (int v : w) {
        total += v;
      }
      if (total > width && total > 0) {
        int min = Theme.scale(36);
        int used = 0;
        for (int c = 0; c < cols; c++) {
          w[c] = Math.max(min, (int) ((long) natural[c] * width / total));
          used += w[c];
        }
        if (used < width) {
          w[cols - 1] += width - used;
        }
      }
      return w;
    }

    private int rowHeight(JTextPane[] line, int[] w) {
      int h = 0;
      for (int c = 0; c < cols; c++) {
        h = Math.max(h, ChatView.heightAt(line[c], Math.max(1, w[c] - 2 * pad)));
      }
      return h + pad;
    }

    @Override
    public int heightAt(int width) {
      int[] w = widths(width);
      int h = 1;
      for (JTextPane[] line : cells) {
        h += rowHeight(line, w);
      }
      return h;
    }

    @Override
    protected void paintComponent(Graphics g) {
      int[] w = widths(getWidth());
      int tableWidth = 0;
      for (int v : w) {
        tableWidth += v;
      }
      tableWidth = Math.min(tableWidth, getWidth());
      int y = 0;
      boolean header = true;
      List<Integer> lines = new ArrayList<>();
      for (JTextPane[] line : cells) {
        int h = rowHeight(line, w);
        if (header) {
          g.setColor(AiTheme.inlineCodeBg());
          g.fillRect(0, 0, tableWidth, h);
          header = false;
        }
        lines.add(y);
        y += h;
      }
      lines.add(Math.min(y, getHeight() - 1));
      g.setColor(AiTheme.markLine());
      for (int ly : lines) {
        g.fillRect(0, ly, tableWidth, 1);
      }
      int x = 0;
      for (int c = 0; c <= cols; c++) {
        g.fillRect(Math.min(x, tableWidth - 1), 0, 1, Math.min(y, getHeight() - 1) + 1);
        if (c < cols) {
          x += w[c];
        }
      }
    }
  }

}
