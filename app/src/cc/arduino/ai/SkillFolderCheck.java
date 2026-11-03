/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Finds skills Hermes cannot use because of the skill folders in the list.
 * Hermes gives every folder of skills.external_dirs the same precedence (their
 * order does not matter): when two folders hold a skill of the same name it
 * refuses to guess, and no copy can be used, even when the copies are
 * identical. Inside one folder only identical copies are fine. A folder inside
 * another one is searched twice. The rules follow Hermes'
 * agent/skill_utils.py (iter_skill_index_files, pick_skill_candidate).
 *
 * Reads the folders: do not call it on the event thread.
 */
public class SkillFolderCheck {

  /** Folders Hermes does not search (EXCLUDED_SKILL_DIRS). */
  private static final Set<String> EXCLUDED = new HashSet<>(Arrays.asList(
      ".git", ".github", ".hub", ".archive", ".curator_backups", ".locks",
      ".venv", "venv", "node_modules", "site-packages", "__pycache__",
      ".tox", ".nox", ".pytest_cache", ".mypy_cache", ".ruff_cache"));
  /** Folders of a skill's own files, not searched for other skills (SKILL_SUPPORT_DIRS). */
  private static final Set<String> SUPPORT = new HashSet<>(Arrays.asList("references", "templates", "assets", "scripts"));
  private static final String SKILL_FILE = "SKILL.md";
  /** Hermes reads the name from the first 4000 characters and keeps 64 of it. */
  private static final int READ_LIMIT = 4000;
  private static final int MAX_NAME_LENGTH = 64;
  private static final Pattern NAME_LINE = Pattern.compile("name:\\s*(.*?)\\s*");
  private static final Pattern FENCE = Pattern.compile("---\\s*");

  /** A skill name found in more than one place. */
  public static class Conflict {
    public final String name;
    /** The folder of each copy (the one holding its SKILL.md). */
    public final List<File> places = new ArrayList<>();
    /** The skill folder of the list each copy was found in. */
    public final List<String> roots = new ArrayList<>();

    Conflict(String name) {
      this.name = name;
    }
  }

  /** A skill folder of the list that is inside another one. */
  public static class Overlap {
    public final String inner;
    public final String outer;

    Overlap(String inner, String outer) {
      this.inner = inner;
      this.outer = outer;
    }
  }

  public final List<Conflict> conflicts = new ArrayList<>();
  public final List<Overlap> overlaps = new ArrayList<>();

  private static class Skill {
    final String root;
    final File file;
    final File real;
    final int depth;

    Skill(String root, File file, int depth) {
      this.root = root;
      this.file = file;
      this.real = realFile(file);
      this.depth = depth;
    }
  }

  public boolean isEmpty() {
    return conflicts.isEmpty() && overlaps.isEmpty();
  }

  /** Only the problems this skill folder of the list takes part in. */
  public SkillFolderCheck about(String dir) {
    SkillFolderCheck c = new SkillFolderCheck();
    for (Conflict x : conflicts) {
      if (x.roots.contains(dir)) {
        c.conflicts.add(x);
      }
    }
    for (Overlap x : overlaps) {
      if (x.inner.equals(dir) || x.outer.equals(dir)) {
        c.overlaps.add(x);
      }
    }
    return c;
  }

  /** Checks the skill folders; missing ones are skipped, as Hermes does. */
  public static SkillFolderCheck check(List<String> dirs) {
    SkillFolderCheck result = new SkillFolderCheck();
    List<String> roots = new ArrayList<>();
    List<File> realRoots = new ArrayList<>();
    for (String d : dirs) {
      File real = realFile(new File(d));
      if (real.isDirectory() && !realRoots.contains(real)) {
        roots.add(d);
        realRoots.add(real);
      }
    }
    for (int i = 0; i < roots.size(); i++) {
      for (int j = 0; j < roots.size(); j++) {
        if (i != j && realRoots.get(i).toPath().startsWith(realRoots.get(j).toPath())) {
          result.overlaps.add(new Overlap(roots.get(i), roots.get(j)));
        }
      }
    }
    Map<String, List<Skill>> byName = new LinkedHashMap<>();
    for (int i = 0; i < roots.size(); i++) {
      List<Skill> found = new ArrayList<>();
      walk(roots.get(i), realRoots.get(i), 1, found, new HashSet<>());
      for (Skill s : found) {
        byName.computeIfAbsent(skillName(s.file), k -> new ArrayList<>()).add(s);
      }
    }
    for (Map.Entry<String, List<Skill>> e : byName.entrySet()) {
      // a folder inside another one finds the same file twice: that is reported as an overlap
      List<Skill> copies = new ArrayList<>();
      for (Skill s : e.getValue()) {
        if (copies.stream().noneMatch(c -> c.real.equals(s.real))) {
          copies.add(s);
        }
      }
      if (copies.size() > 1 && !resolvedInOneFolder(copies)) {
        Conflict c = new Conflict(e.getKey());
        for (Skill s : copies) {
          c.places.add(s.file.getParentFile());
          c.roots.add(s.root);
        }
        result.conflicts.add(c);
      }
    }
    return result;
  }

  /** Like os.walk(followlinks=True) in iter_skill_index_files; depth counts the parts below the root. */
  private static void walk(String root, File dir, int depth, List<Skill> found, Set<File> visited) {
    if (!visited.add(realFile(dir))) {
      return; // a link back to a folder already searched
    }
    String[] names = dir.list();
    if (names == null) {
      return;
    }
    Arrays.sort(names);
    // the exact name: "skill.md" is not a skill for Hermes, also on Windows
    boolean hasSkill = Arrays.asList(names).contains(SKILL_FILE) && new File(dir, SKILL_FILE).isFile();
    if (hasSkill) {
      found.add(new Skill(root, new File(dir, SKILL_FILE), depth));
    }
    for (String name : names) {
      File child = new File(dir, name);
      if (child.isDirectory() && !EXCLUDED.contains(name) && !(hasSkill && SUPPORT.contains(name))) {
        walk(root, child, depth + 1, found, visited);
      }
    }
  }

  /**
   * Hermes picks one copy only when all are in one skill folder, have the same
   * content, and one of them is less deep than the others.
   */
  private static boolean resolvedInOneFolder(List<Skill> copies) {
    Skill first = copies.get(0);
    int shallowest = Integer.MAX_VALUE;
    int atShallowest = 0;
    try {
      byte[] content = Files.readAllBytes(first.file.toPath());
      for (Skill s : copies) {
        if (!s.root.equals(first.root) || !Arrays.equals(content, Files.readAllBytes(s.file.toPath()))) {
          return false;
        }
        if (s.depth < shallowest) {
          shallowest = s.depth;
          atShallowest = 1;
        } else if (s.depth == shallowest) {
          atShallowest++;
        }
      }
    } catch (IOException e) {
      return false;
    }
    return atShallowest == 1;
  }

  /** The name: line of the SKILL.md front matter, else the name of its folder. */
  static String skillName(File skillFile) {
    String name = null;
    try {
      String text = new String(Files.readAllBytes(skillFile.toPath()), StandardCharsets.UTF_8);
      if (text.startsWith("\uFEFF")) {
        text = text.substring(1);
      }
      if (text.length() > READ_LIMIT) {
        text = text.substring(0, READ_LIMIT);
      }
      if (text.startsWith("---")) {
        String[] lines = text.split("\r?\n", -1);
        String found = null;
        for (int i = 1; i < lines.length; i++) {
          if (FENCE.matcher(lines[i]).matches()) {
            name = found; // without the closing line there is no front matter
            break;
          }
          Matcher m = NAME_LINE.matcher(lines[i]);
          if (found == null && m.matches()) {
            found = yamlText(m.group(1));
          }
        }
      }
    } catch (IOException e) {
      // unreadable: Hermes skips it too, but the folder name is a fair guess
    }
    if (name == null || name.isEmpty()) {
      name = skillFile.getParentFile().getName();
    }
    return name.length() > MAX_NAME_LENGTH ? name.substring(0, MAX_NAME_LENGTH) : name;
  }

  /** A one-line YAML value, quoted or plain, with an optional " # comment" after it. */
  private static String yamlText(String value) {
    if (value.startsWith("\"") || value.startsWith("'")) {
      char quote = value.charAt(0);
      StringBuilder sb = new StringBuilder();
      for (int i = 1; i < value.length(); i++) {
        char ch = value.charAt(i);
        if (quote == '"' && ch == '\\' && i + 1 < value.length()) {
          sb.append(value.charAt(++i));
        } else if (ch == quote && quote == '\'' && i + 1 < value.length() && value.charAt(i + 1) == '\'') {
          sb.append(ch); // '' inside single quotes
          i++;
        } else if (ch == quote) {
          return sb.toString();
        } else {
          sb.append(ch);
        }
      }
      return value; // no closing quote
    }
    int comment = value.indexOf(" #");
    return (comment < 0 ? value : value.substring(0, comment)).trim();
  }

  private static File realFile(File f) {
    try {
      return f.getCanonicalFile();
    } catch (IOException e) {
      return f.getAbsoluteFile();
    }
  }
}
