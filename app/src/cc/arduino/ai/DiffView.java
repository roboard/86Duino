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
import java.awt.Font;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.swing.JTextPane;
import javax.swing.text.BadLocationException;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;

import processing.app.I18n;
import processing.app.PreferencesData;

/**
 * Read-only unified line diff (removed lines red, added lines green, three
 * lines of context around each change).
 */
public class DiffView extends JTextPane {

  private static final int CONTEXT = 3;
  /** Above this many line pairs the LCS table is too big: show a plain replace. */
  private static final long MAX_CELLS = 4_000_000L;

  enum Op { SAME, DEL, ADD }

  static class Line {
    final Op op;
    final int oldNo;
    final int newNo;
    final String text;

    Line(Op op, int oldNo, int newNo, String text) {
      this.op = op;
      this.oldNo = oldNo;
      this.newNo = newNo;
      this.text = text;
    }
  }

  private int added;
  private int removed;

  private int lineCount;

  public DiffView(String oldText, String newText) {
    setEditable(false);
    Font f = PreferencesData.getFont("editor.font");
    setFont(new Font(Font.MONOSPACED, Font.PLAIN, f != null ? f.getSize() : 12));
    // Hermes sends "patch" edits as the patch text itself, not the new file.
    if (newText != null && newText.startsWith("*** Begin Patch")) {
      String applied = applyPatch(oldText, newText);
      if (applied != null) {
        newText = applied;
      } else {
        renderPatch(newText);
        setCaretPosition(0);
        return;
      }
    }
    render(diff(split(oldText), split(newText)));
    setCaretPosition(0);
  }

  /** Number of displayed lines, to size the dialog. */
  public int getLineCount() {
    return lineCount;
  }

  /**
   * Applies a single-file patch in the "*** Begin Patch" (V4A) format to the
   * old text. Returns null when the patch touches several files, adds, deletes
   * or moves files, or when a hunk does not match.
   */
  static String applyPatch(String oldText, String patch) {
    String[] lines = split(patch);
    int updates = 0;
    boolean addFile = false;
    for (String l : lines) {
      if (l.startsWith("*** Update File:")) {
        updates++;
      } else if (l.startsWith("*** Add File:")) {
        addFile = true;
        updates++;
      } else if (l.startsWith("*** Delete File:") || l.startsWith("*** Move to:")) {
        return null;
      }
    }
    if (updates != 1) {
      return null;
    }
    if (addFile) {
      StringBuilder sb = new StringBuilder();
      for (String l : lines) {
        if (l.startsWith("+")) {
          sb.append(l.substring(1)).append('\n');
        }
      }
      return sb.toString();
    }
    List<String> file = new ArrayList<>(Arrays.asList(split(oldText)));
    int cursor = 0;
    List<String> oldBlock = new ArrayList<>();
    List<String> newBlock = new ArrayList<>();
    String anchor = null;
    boolean inHunk = false;
    for (String l : lines) {
      boolean hunkEnd = l.startsWith("@@") || l.startsWith("*** End Patch") || l.startsWith("*** End of File");
      if (hunkEnd || l.startsWith("*** ")) {
        if (inHunk && (!oldBlock.isEmpty() || !newBlock.isEmpty())) {
          cursor = applyHunk(file, cursor, anchor, oldBlock, newBlock);
          if (cursor < 0) {
            return null;
          }
        }
        oldBlock.clear();
        newBlock.clear();
        anchor = l.startsWith("@@") ? l.substring(2).trim() : null;
        inHunk = l.startsWith("@@") || l.startsWith("*** Update File:");
        continue;
      }
      if (!inHunk) {
        continue;
      }
      char c = l.isEmpty() ? ' ' : l.charAt(0);
      String body = l.isEmpty() ? "" : l.substring(1);
      if (c == '-') {
        oldBlock.add(body);
      } else if (c == '+') {
        newBlock.add(body);
      } else {
        oldBlock.add(body);
        newBlock.add(body);
      }
    }
    StringBuilder sb = new StringBuilder();
    for (String l : file) {
      sb.append(l).append('\n');
    }
    return sb.toString();
  }

  /** Replaces oldBlock (searched from cursor) with newBlock; returns the new cursor or -1. */
  private static int applyHunk(List<String> file, int cursor, String anchor, List<String> oldBlock, List<String> newBlock) {
    if (anchor != null && !anchor.isEmpty()) {
      for (int i = cursor; i < file.size(); i++) {
        if (file.get(i).trim().equals(anchor)) {
          cursor = i + 1;
          break;
        }
      }
    }
    int at = find(file, oldBlock, cursor, false);
    if (at < 0) {
      at = find(file, oldBlock, cursor, true);
    }
    if (at < 0 && cursor > 0) {
      at = find(file, oldBlock, 0, true);
    }
    if (at < 0) {
      return -1;
    }
    for (int k = 0; k < oldBlock.size(); k++) {
      file.remove(at);
    }
    file.addAll(at, newBlock);
    return at + newBlock.size();
  }

  private static int find(List<String> file, List<String> block, int from, boolean trimmed) {
    if (block.isEmpty()) {
      return from <= file.size() ? from : -1;
    }
    for (int i = from; i + block.size() <= file.size(); i++) {
      boolean match = true;
      for (int k = 0; k < block.size() && match; k++) {
        String a = file.get(i + k);
        String b = block.get(k);
        match = trimmed ? a.trim().equals(b.trim()) : a.equals(b);
      }
      if (match) {
        return i;
      }
    }
    return -1;
  }

  /** Shows a patch that could not be applied as it is, colored by line type. */
  private void renderPatch(String patch) {
    StyledDocument doc = getStyledDocument();
    SimpleAttributeSet same = new SimpleAttributeSet();
    StyleConstants.setForeground(same, new Color(0x55, 0x55, 0x55));
    SimpleAttributeSet del = new SimpleAttributeSet();
    StyleConstants.setBackground(del, new Color(0xFF, 0xE0, 0xE0));
    StyleConstants.setForeground(del, new Color(0x9A, 0x10, 0x10));
    SimpleAttributeSet add = new SimpleAttributeSet();
    StyleConstants.setBackground(add, new Color(0xDC, 0xF5, 0xDC));
    StyleConstants.setForeground(add, new Color(0x10, 0x6A, 0x10));
    SimpleAttributeSet meta = new SimpleAttributeSet();
    StyleConstants.setForeground(meta, new Color(0x80, 0x80, 0xC0));
    try {
      for (String l : split(patch)) {
        SimpleAttributeSet style = l.startsWith("***") || l.startsWith("@@") ? meta
                                 : l.startsWith("+") ? add : l.startsWith("-") ? del : same;
        if (style == add) {
          added++;
        } else if (style == del) {
          removed++;
        }
        doc.insertString(doc.getLength(), l + "\n", style);
        lineCount++;
      }
    } catch (BadLocationException e) {
      // cannot happen when appending
    }
  }

  /** Do not wrap long lines: scroll horizontally instead, like a code editor. */
  @Override
  public boolean getScrollableTracksViewportWidth() {
    return getParent() == null || getUI().getPreferredSize(this).width <= getParent().getSize().width;
  }

  public int getAddedCount() {
    return added;
  }

  public int getRemovedCount() {
    return removed;
  }

  static String[] split(String text) {
    if (text == null || text.isEmpty()) {
      return new String[0];
    }
    String t = text.replace("\r\n", "\n");
    if (t.endsWith("\n")) {
      t = t.substring(0, t.length() - 1);
    }
    return t.split("\n", -1);
  }

  static List<Line> diff(String[] a, String[] b) {
    // Trim the common prefix and suffix first: most edits are small.
    int start = 0;
    while (start < a.length && start < b.length && a[start].equals(b[start])) {
      start++;
    }
    int endA = a.length;
    int endB = b.length;
    while (endA > start && endB > start && a[endA - 1].equals(b[endB - 1])) {
      endA--;
      endB--;
    }
    List<Line> out = new ArrayList<>();
    for (int i = 0; i < start; i++) {
      out.add(new Line(Op.SAME, i + 1, i + 1, a[i]));
    }
    String[] ma = Arrays.copyOfRange(a, start, endA);
    String[] mb = Arrays.copyOfRange(b, start, endB);
    if ((long) ma.length * mb.length > MAX_CELLS) {
      for (int i = 0; i < ma.length; i++) {
        out.add(new Line(Op.DEL, start + i + 1, 0, ma[i]));
      }
      for (int j = 0; j < mb.length; j++) {
        out.add(new Line(Op.ADD, 0, start + j + 1, mb[j]));
      }
    } else {
      int[][] lcs = new int[ma.length + 1][mb.length + 1];
      for (int i = ma.length - 1; i >= 0; i--) {
        for (int j = mb.length - 1; j >= 0; j--) {
          lcs[i][j] = ma[i].equals(mb[j]) ? lcs[i + 1][j + 1] + 1 : Math.max(lcs[i + 1][j], lcs[i][j + 1]);
        }
      }
      int i = 0;
      int j = 0;
      while (i < ma.length || j < mb.length) {
        if (i < ma.length && j < mb.length && ma[i].equals(mb[j])) {
          out.add(new Line(Op.SAME, start + i + 1, start + j + 1, ma[i]));
          i++;
          j++;
        } else if (i < ma.length && (j == mb.length || lcs[i + 1][j] >= lcs[i][j + 1])) {
          // removed lines first, then the lines that replace them
          out.add(new Line(Op.DEL, start + i + 1, 0, ma[i]));
          i++;
        } else {
          out.add(new Line(Op.ADD, 0, start + j + 1, mb[j]));
          j++;
        }
      }
    }
    for (int k = 0; endA + k < a.length; k++) {
      out.add(new Line(Op.SAME, endA + k + 1, endB + k + 1, a[endA + k]));
    }
    return out;
  }

  private void render(List<Line> lines) {
    StyledDocument doc = getStyledDocument();
    SimpleAttributeSet same = new SimpleAttributeSet();
    StyleConstants.setForeground(same, new Color(0x55, 0x55, 0x55));
    SimpleAttributeSet del = new SimpleAttributeSet();
    StyleConstants.setBackground(del, new Color(0xFF, 0xE0, 0xE0));
    StyleConstants.setForeground(del, new Color(0x9A, 0x10, 0x10));
    SimpleAttributeSet add = new SimpleAttributeSet();
    StyleConstants.setBackground(add, new Color(0xDC, 0xF5, 0xDC));
    StyleConstants.setForeground(add, new Color(0x10, 0x6A, 0x10));
    SimpleAttributeSet gap = new SimpleAttributeSet();
    StyleConstants.setForeground(gap, new Color(0x80, 0x80, 0xC0));
    StyleConstants.setItalic(gap, true);

    // Mark which unchanged lines are close enough to a change to be shown.
    boolean[] show = new boolean[lines.size()];
    for (int k = 0; k < lines.size(); k++) {
      if (lines.get(k).op != Op.SAME) {
        for (int c = Math.max(0, k - CONTEXT); c <= Math.min(lines.size() - 1, k + CONTEXT); c++) {
          show[c] = true;
        }
      }
    }
    try {
      int hidden = 0;
      for (int k = 0; k < lines.size(); k++) {
        Line l = lines.get(k);
        if (l.op == Op.DEL) {
          removed++;
        } else if (l.op == Op.ADD) {
          added++;
        }
        if (!show[k]) {
          hidden++;
          continue;
        }
        if (hidden > 0) {
          doc.insertString(doc.getLength(), "      " + I18n.format(tr("... {0} unchanged lines ..."), hidden) + "\n", gap);
          hidden = 0;
        }
        String no = l.op == Op.ADD ? pad(l.newNo) : pad(l.oldNo);
        String mark = l.op == Op.ADD ? "+ " : l.op == Op.DEL ? "- " : "  ";
        doc.insertString(doc.getLength(), no + " " + mark + l.text + "\n",
                         l.op == Op.ADD ? add : l.op == Op.DEL ? del : same);
        lineCount++;
      }
      if (hidden > 0) {
        doc.insertString(doc.getLength(), "      " + I18n.format(tr("... {0} unchanged lines ..."), hidden) + "\n", gap);
      }
      if (lines.isEmpty() || (added == 0 && removed == 0)) {
        doc.insertString(doc.getLength(), tr("(no changes)") + "\n", gap);
      }
    } catch (BadLocationException e) {
      // cannot happen when appending
    }
  }

  private static String pad(int n) {
    String s = Integer.toString(n);
    return "     ".substring(Math.min(5, s.length())) + s;
  }
}
