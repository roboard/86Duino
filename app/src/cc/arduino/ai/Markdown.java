/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * The Markdown the AI writes, cut into blocks: text (paragraphs, headings,
 * list items), code, quotes, rules and tables, with the bold, italic, code
 * and links inside the text. Only what AI answers use; anything else stays
 * as it was written. Unfinished Markdown (an answer still arriving) is read
 * as far as it goes: a code block without its closing ``` is still code.
 */
final class Markdown {

  private Markdown() {
  }

  enum Kind {
    TEXT, CODE, QUOTE, RULE, TABLE
  }

  /** One block of the answer. */
  static final class Block {
    final Kind kind;
    /** The Markdown it was made of: the same source draws the same block. */
    final String source;
    /** TEXT and QUOTE: the paragraphs. */
    final List<Para> paras = new ArrayList<>();
    /** CODE: the language after ```, lower case ("" without one), and the code. */
    String lang = "";
    String code = "";
    /** CODE: the closing ``` arrived. */
    boolean closed = true;
    /** TABLE: the rows, the first one the header. */
    final List<List<String>> rows = new ArrayList<>();

    Block(Kind kind, String source) {
      this.kind = kind;
      this.source = source;
    }

    boolean sameAs(Block b) {
      return b != null && kind == b.kind && closed == b.closed && source.equals(b.source);
    }
  }

  enum ParaKind {
    TEXT, HEADING, ITEM
  }

  /** A paragraph, a heading or a list item, with its Markdown inline text. */
  static final class Para {
    final ParaKind kind;
    /** HEADING: 1 to 6. ITEM: how deep in the list, from 0. */
    final int level;
    /** ITEM: "•" or the number as written ("1."). */
    final String marker;
    final StringBuilder text = new StringBuilder();

    Para(ParaKind kind, int level, String marker, String text) {
      this.kind = kind;
      this.level = level;
      this.marker = marker;
      this.text.append(text);
    }
  }

  private static final Pattern FENCE = Pattern.compile("^(\\s*)(`{3,}|~{3,})\\s*([^`\\s]*).*$");
  private static final Pattern HEADING = Pattern.compile("^\\s{0,3}(#{1,6})\\s+(.*?)\\s*#*\\s*$");
  private static final Pattern ITEM = Pattern.compile("^(\\s*)([-*+]|\\d{1,9}[.)])\\s+(.*)$");
  private static final Pattern RULE = Pattern.compile("^\\s{0,3}([-*_])(\\s*\\1){2,}\\s*$");
  private static final Pattern TABLE_LINE = Pattern.compile("^\\s*\\|?\\s*:?-+:?\\s*(\\|\\s*:?-+:?\\s*)*\\|?\\s*$");
  private static final Pattern QUOTE = Pattern.compile("^\\s{0,3}>\\s?(.*)$");

  /** The blocks of this Markdown. */
  static List<Block> parse(String md) {
    String[] lines = md.replace("\r\n", "\n").replace('\r', '\n').split("\n", -1);
    List<Block> blocks = new ArrayList<>();
    Block text = null; // the text block being filled
    StringBuilder textSource = null;
    Para para = null; // the paragraph being filled (null after a blank line)
    int i = 0;
    while (i < lines.length) {
      String line = lines[i];
      Matcher m;
      // ---- code
      if ((m = FENCE.matcher(line)).matches()) {
        closeText(blocks, text, textSource);
        text = null;
        para = null;
        String indent = m.group(1);
        String fence = m.group(2);
        StringBuilder src = new StringBuilder(line);
        StringBuilder code = new StringBuilder();
        boolean closed = false;
        int j = i + 1;
        for (; j < lines.length; j++) {
          String l = lines[j];
          src.append('\n').append(l);
          String t = l.trim();
          if (t.startsWith(fence.substring(0, 3)) && t.replace(fence.charAt(0), ' ').trim().isEmpty()
              && t.length() >= fence.length()) {
            closed = true;
            break;
          }
          if (code.length() > 0 || j > i + 1) {
            code.append('\n');
          }
          code.append(stripIndent(l, indent.length()));
        }
        Block b = new Block(Kind.CODE, src.toString());
        b.lang = m.group(3).toLowerCase();
        b.code = code.toString();
        b.closed = closed;
        blocks.add(b);
        i = closed ? j + 1 : lines.length;
        continue;
      }
      // ---- table: a line with | over a line of dashes
      if (line.contains("|") && i + 1 < lines.length && lines[i + 1].contains("-")
          && TABLE_LINE.matcher(lines[i + 1]).matches()) {
        closeText(blocks, text, textSource);
        text = null;
        para = null;
        StringBuilder src = new StringBuilder(line).append('\n').append(lines[i + 1]);
        List<List<String>> rows = new ArrayList<>();
        rows.add(cells(line));
        int j = i + 2;
        for (; j < lines.length && lines[j].contains("|") && !lines[j].trim().isEmpty(); j++) {
          src.append('\n').append(lines[j]);
          rows.add(cells(lines[j]));
        }
        Block b = new Block(Kind.TABLE, src.toString());
        b.rows.addAll(rows);
        blocks.add(b);
        i = j;
        continue;
      }
      // ---- a rule
      if (RULE.matcher(line).matches()) {
        closeText(blocks, text, textSource);
        text = null;
        para = null;
        blocks.add(new Block(Kind.RULE, line));
        i++;
        continue;
      }
      // ---- a quote: its lines together, read as text
      if (QUOTE.matcher(line).matches()) {
        closeText(blocks, text, textSource);
        text = null;
        para = null;
        StringBuilder src = new StringBuilder();
        StringBuilder inner = new StringBuilder();
        int j = i;
        for (; j < lines.length && (m = QUOTE.matcher(lines[j])).matches(); j++) {
          if (j > i) {
            src.append('\n');
            inner.append('\n');
          }
          src.append(lines[j]);
          inner.append(m.group(1));
        }
        Block b = new Block(Kind.QUOTE, src.toString());
        for (Block t : parse(inner.toString())) {
          b.paras.addAll(t.paras); // only the text of the quote
        }
        blocks.add(b);
        i = j;
        continue;
      }
      // ---- text
      if (text == null) {
        text = new Block(Kind.TEXT, "");
        textSource = new StringBuilder();
      } else {
        textSource.append('\n');
      }
      textSource.append(line);
      if (line.trim().isEmpty()) {
        para = null; // a blank line ends the paragraph
      } else if ((m = HEADING.matcher(line)).matches()) {
        text.paras.add(new Para(ParaKind.HEADING, m.group(1).length(), "", m.group(2)));
        para = null;
      } else if ((m = ITEM.matcher(line)).matches()) {
        String marker = m.group(2);
        int level = Math.min(4, indentWidth(m.group(1)) / 2);
        para = new Para(ParaKind.ITEM, level, Character.isDigit(marker.charAt(0)) ? marker : bullet(level), m.group(3));
        text.paras.add(para);
      } else if (para != null) {
        // the next line of the paragraph or list item: the line break is kept
        para.text.append('\n').append(line.trim());
      } else {
        para = new Para(ParaKind.TEXT, 0, "", line.trim());
        text.paras.add(para);
      }
      i++;
    }
    closeText(blocks, text, textSource);
    return blocks;
  }

  private static void closeText(List<Block> blocks, Block text, StringBuilder source) {
    if (text == null || text.paras.isEmpty()) {
      return;
    }
    Block b = new Block(Kind.TEXT, source.toString());
    b.paras.addAll(text.paras);
    blocks.add(b);
  }

  /** The same bullet at every depth (the indent shows the depth): other bullets are not as wide in every font. */
  private static String bullet(int level) {
    return "•";
  }

  private static int indentWidth(String s) {
    int w = 0;
    for (char c : s.toCharArray()) {
      w += c == '\t' ? 4 : 1;
    }
    return w;
  }

  /** The code inside a fence that was indented (in a list): without that indent. */
  private static String stripIndent(String line, int n) {
    int k = 0;
    while (k < n && k < line.length() && line.charAt(k) == ' ') {
      k++;
    }
    return line.substring(k);
  }

  private static List<String> cells(String line) {
    String t = line.trim();
    if (t.startsWith("|")) {
      t = t.substring(1);
    }
    if (t.endsWith("|") && !t.endsWith("\\|")) {
      t = t.substring(0, t.length() - 1);
    }
    List<String> cells = new ArrayList<>();
    StringBuilder cell = new StringBuilder();
    for (int k = 0; k < t.length(); k++) {
      char c = t.charAt(k);
      if (c == '\\' && k + 1 < t.length() && t.charAt(k + 1) == '|') {
        cell.append('|');
        k++;
      } else if (c == '|') {
        cells.add(cell.toString().trim());
        cell.setLength(0);
      } else {
        cell.append(c);
      }
    }
    cells.add(cell.toString().trim());
    return cells;
  }

  // ------------------------------------------------------------------ inline

  static final int BOLD = 1;
  static final int ITALIC = 2;
  static final int CODE = 4;
  static final int STRIKE = 8;

  /** A piece of text in one style; url is set for a link. */
  static final class Run {
    final String text;
    final int style;
    final String url;

    Run(String text, int style, String url) {
      this.text = text;
      this.style = style;
      this.url = url;
    }
  }

  private static final String PUNCT = "\\`*_{}[]()#+-.!|~>";
  private static final Pattern URL = Pattern.compile("https?://[A-Za-z0-9\\-._~:/?#\\[\\]@!$&'()*+,;=%]+");

  /** The styled pieces of a paragraph's text. */
  static List<Run> inline(String s) {
    List<Run> out = new ArrayList<>();
    inline(s, 0, null, out);
    return out;
  }

  private static void inline(String s, int style, String url, List<Run> out) {
    StringBuilder plain = new StringBuilder();
    int i = 0;
    int n = s.length();
    while (i < n) {
      char c = s.charAt(i);
      // \* is a star
      if (c == '\\' && i + 1 < n && PUNCT.indexOf(s.charAt(i + 1)) >= 0) {
        plain.append(s.charAt(i + 1));
        i += 2;
        continue;
      }
      // `code`
      if (c == '`') {
        int ticks = 0;
        while (i + ticks < n && s.charAt(i + ticks) == '`') {
          ticks++;
        }
        String fence = s.substring(i, i + ticks);
        int end = s.indexOf(fence, i + ticks);
        while (end >= 0 && end + ticks < n && s.charAt(end + ticks) == '`') {
          end = s.indexOf(fence, end + ticks + 1);
        }
        if (end > i + ticks) {
          flush(plain, style, url, out);
          String code = s.substring(i + ticks, end);
          if (code.length() > 2 && code.startsWith(" ") && code.endsWith(" ")) {
            code = code.substring(1, code.length() - 1);
          }
          out.add(new Run(code, style | CODE, url));
          i = end + ticks;
          continue;
        }
        plain.append(fence);
        i += ticks;
        continue;
      }
      // **bold**, __bold__, ~~struck~~
      String pair = i + 1 < n ? s.substring(i, i + 2) : "";
      if (pair.equals("**") || pair.equals("~~") || (pair.equals("__") && wordEdge(s, i - 1))) {
        int end = findClose(s, pair, i + 2);
        if (end > i + 2 && (!pair.equals("__") || wordEdge(s, end + 2))) {
          flush(plain, style, url, out);
          inline(s.substring(i + 2, end), style | (pair.equals("~~") ? STRIKE : BOLD), url, out);
          i = end + 2;
          continue;
        }
      }
      // *italic* (not _italic_: snake_case names are common in code talk)
      if (c == '*' && i + 1 < n && !Character.isWhitespace(s.charAt(i + 1)) && s.charAt(i + 1) != '*') {
        int end = findClose(s, "*", i + 1);
        if (end > i + 1) {
          flush(plain, style, url, out);
          inline(s.substring(i + 1, end), style | ITALIC, url, out);
          i = end + 1;
          continue;
        }
      }
      // [text](url)
      if (c == '[' && url == null) {
        int close = s.indexOf("](", i + 1);
        int end = close < 0 ? -1 : s.indexOf(')', close + 2);
        if (close > i && end > close + 2 && s.substring(i + 1, close).indexOf('\n') < 0) {
          flush(plain, style, url, out);
          String target = s.substring(close + 2, end).trim();
          int space = target.indexOf(' ');
          inline(s.substring(i + 1, close), style, space > 0 ? target.substring(0, space) : target, out);
          i = end + 1;
          continue;
        }
      }
      // <http://...> and bare addresses
      if ((c == 'h' || c == '<') && url == null) {
        int start = c == '<' ? i + 1 : i;
        Matcher m = URL.matcher(s);
        m.region(start, n);
        if (m.lookingAt() && (c == '<' || i == 0 || !Character.isLetterOrDigit(s.charAt(i - 1)))) {
          String link = m.group();
          int end = start + link.length();
          if (c == '<' && end < n && s.charAt(end) == '>') {
            flush(plain, style, url, out);
            out.add(new Run(link, style, link));
            i = end + 1;
            continue;
          }
          if (c == 'h') {
            // the sentence's punctuation after it is not part of it
            while (link.length() > 8 && ".,;:!?)'\"]".indexOf(link.charAt(link.length() - 1)) >= 0) {
              link = link.substring(0, link.length() - 1);
            }
            flush(plain, style, url, out);
            out.add(new Run(link, style, link));
            i += link.length();
            continue;
          }
        }
      }
      plain.append(c);
      i++;
    }
    flush(plain, style, url, out);
  }

  /** Where the closing mark is: not right after a space, not inside `code`. */
  private static int findClose(String s, String mark, int from) {
    int k = from;
    while (k < s.length()) {
      char c = s.charAt(k);
      if (c == '\\') {
        k += 2;
        continue;
      }
      if (c == '`') {
        int end = s.indexOf('`', k + 1);
        if (end < 0) {
          return -1;
        }
        k = end + 1;
        continue;
      }
      if (s.startsWith(mark, k) && !Character.isWhitespace(s.charAt(k - 1))) {
        if (mark.equals("*") && k + 1 < s.length() && s.charAt(k + 1) == '*') {
          k += 2; // a ** inside *...*
          continue;
        }
        return k;
      }
      k++;
    }
    return -1;
  }

  private static boolean wordEdge(String s, int k) {
    return k < 0 || k >= s.length() || !Character.isLetterOrDigit(s.charAt(k));
  }

  private static void flush(StringBuilder plain, int style, String url, List<Run> out) {
    if (plain.length() > 0) {
      out.add(new Run(plain.toString(), style, url));
      plain.setLength(0);
    }
  }
}
