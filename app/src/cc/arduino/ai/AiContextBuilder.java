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
import java.util.LinkedHashMap;
import java.util.Map;

import processing.app.BaseNoGui;
import processing.app.Sketch;
import processing.app.SketchFile;
import processing.app.debug.TargetBoard;
import processing.app.debug.TargetPlatform;
import processing.app.helpers.PreferencesMap;
import processing.app.packages.UserLibraryFolder;

/**
 * Builds the hidden context block sent with the first message of a
 * conversation: which board is selected, where the sketch is, and which
 * folders the AI may read for reference (cores, variants, libraries, examples).
 */
public class AiContextBuilder {

  /** The reference folders for the current board, by description. */
  public static Map<String, File> referenceFolders() {
    Map<String, File> dirs = new LinkedHashMap<>();
    TargetPlatform platform = BaseNoGui.getTargetPlatform();
    PreferencesMap board = BaseNoGui.getBoardPreferences();
    if (platform != null && board != null) {
      File platformFolder = platform.getFolder();
      String core = board.get("build.core", "arduino");
      File coreFolder = platformFolder;
      if (core.contains(":")) {
        String[] parts = core.split(":", 2);
        TargetPlatform ref = BaseNoGui.getTargetPlatform(parts[0], platform.getId());
        if (ref != null) {
          coreFolder = ref.getFolder();
        }
        core = parts[1];
      }
      put(dirs, "core (Arduino API implementation for this board)", new File(new File(coreFolder, "cores"), core));
      String variant = board.get("build.variant");
      if (variant != null && !variant.isEmpty()) {
        String[] parts = variant.split(":", 2);
        File variantBase = platformFolder;
        if (parts.length == 2) {
          TargetPlatform ref = BaseNoGui.getTargetPlatform(parts[0], platform.getId());
          if (ref != null) {
            variantBase = ref.getFolder();
          }
          variant = parts[1];
        }
        put(dirs, "board variant (pin definitions)", new File(new File(variantBase, "variants"), variant));
      }
    }
    for (UserLibraryFolder lib : BaseNoGui.getLibrariesFolders()) {
      String what;
      switch (lib.location) {
        case IDE_BUILTIN:
          what = "IDE built-in libraries";
          break;
        case CORE:
          what = "board platform libraries";
          break;
        case REFERENCED_CORE:
          what = "referenced platform libraries";
          break;
        case SKETCHBOOK:
          what = "user-installed libraries (each has an examples folder)";
          break;
        default:
          what = "libraries";
      }
      put(dirs, what, lib.folder);
    }
    put(dirs, "IDE built-in examples", BaseNoGui.getExamplesFolder());
    return dirs;
  }

  private static void put(Map<String, File> dirs, String what, File folder) {
    if (folder != null && folder.isDirectory() && !dirs.containsValue(folder)) {
      String key = what;
      for (int n = 2; dirs.containsKey(key); n++) {
        key = what + " " + n;
      }
      dirs.put(key, folder);
    }
  }

  public static String build(Sketch sketch) {
    StringBuilder sb = new StringBuilder();
    sb.append("[86Duino IDE context - written by the IDE, not by the user]\n");
    sb.append("You are the coding assistant inside 86Duino IDE. 86Duino boards are x86 (Vortex86) ")
      .append("boards programmed with the Arduino language (C/C++ sketches, .ino files).\n");
    TargetBoard board = BaseNoGui.getTargetBoard();
    if (board != null) {
      sb.append("Selected board: ").append(board.getName()).append(" (").append(board.getId()).append(")\n");
    }
    sb.append("Sketch folder (your working directory): ").append(sketch.getFolder().getAbsolutePath()).append('\n');
    sb.append("Main sketch file: ").append(sketch.getPrimaryFile().getFileName()).append('\n');
    sb.append("Files in the sketch:");
    for (SketchFile f : sketch.getFiles()) {
      sb.append(' ').append(f.getFileName());
    }
    sb.append('\n');
    sb.append("Reference folders (READ ONLY - read them to learn the real APIs, never modify them):\n");
    for (Map.Entry<String, File> e : referenceFolders().entrySet()) {
      sb.append("- ").append(e.getKey()).append(": ").append(e.getValue().getAbsolutePath()).append('\n');
    }
    sb.append("Rules:\n")
      .append("- Only create or modify files inside the sketch folder. The IDE reloads them automatically.\n")
      .append("- Allowed sketch file types: .ino .h .hh .hpp .c .cpp .s. Keep the main .ino file name.\n")
      .append("- Before using a library or board-specific API, check its header or examples in the reference folders.\n")
      .append("- Edit files with the file tools instead of printing whole files in the chat.\n")
      .append("- The user approves every change in the IDE. If a change is denied, do not try it again in another way: ")
      .append("say what you wanted to change and ask the user what they prefer.\n")
      .append("- Reply in the language the user writes in, and keep explanations short.\n");
    return sb.toString();
  }

  /** Short note sent when the board changes in the middle of a conversation. */
  public static String boardChanged() {
    TargetBoard board = BaseNoGui.getTargetBoard();
    StringBuilder sb = new StringBuilder("[86Duino IDE context] The selected board changed to ");
    sb.append(board != null ? board.getName() + " (" + board.getId() + ")" : "unknown").append(". Reference folders now:\n");
    for (Map.Entry<String, File> e : referenceFolders().entrySet()) {
      sb.append("- ").append(e.getKey()).append(": ").append(e.getValue().getAbsolutePath()).append('\n');
    }
    return sb.toString();
  }

  /** Identifies the board so a change can be detected. */
  public static String boardKey() {
    TargetBoard board = BaseNoGui.getTargetBoard();
    return board == null ? "" : board.getFQBN();
  }
}
