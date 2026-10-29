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
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Window;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;

import com.fasterxml.jackson.databind.JsonNode;

import processing.app.Base;
import processing.app.I18n;
import processing.app.Theme;

/**
 * Asks the user to allow or deny one action the AI wants to perform: a file
 * edit (shown as a diff) or a terminal command.
 */
public class AiPermissionDialog extends JDialog {

  private String answer;

  private AiPermissionDialog(Window owner, JsonNode params, File sketchFolder) {
    super(owner, ModalityType.APPLICATION_MODAL);
    JsonNode toolCall = params.path("toolCall");
    String kind = toolCall.path("kind").asText("");
    boolean isEdit = "edit".equals(kind) || "delete".equals(kind) || "move".equals(kind);
    boolean isExecute = "execute".equals(kind);

    setTitle(isEdit ? tr("AI wants to change a file") : isExecute ? tr("AI wants to run a command") : tr("AI needs your permission"));

    JPanel root = new JPanel(new BorderLayout(0, Theme.scale(8)));
    root.setBorder(BorderFactory.createEmptyBorder(Theme.scale(10), Theme.scale(12), Theme.scale(10), Theme.scale(12)));
    JPanel header = new JPanel();
    header.setLayout(new BoxLayout(header, BoxLayout.Y_AXIS));
    root.add(header, BorderLayout.NORTH);

    List<JsonNode> diffs = new ArrayList<>();
    StringBuilder text = new StringBuilder();
    for (JsonNode c : toolCall.path("content")) {
      if ("diff".equals(c.path("type").asText())) {
        diffs.add(c);
      } else if (c.path("content").has("text")) {
        if (text.length() > 0) {
          text.append('\n');
        }
        text.append(c.path("content").path("text").asText());
      }
    }

    JComponent body;
    if (!diffs.isEmpty()) {
      boolean outside = false;
      JTabbedPane tabs = diffs.size() > 1 ? new JTabbedPane() : null;
      JComponent single = null;
      int maxLines = 0;
      int lineHeight = 16;
      for (JsonNode d : diffs) {
        String path = d.path("path").asText("");
        boolean isNew = !d.has("oldText") || d.get("oldText").isNull();
        boolean inside = isInside(new File(path), sketchFolder);
        outside |= !inside;
        DiffView view = new DiffView(isNew ? "" : d.path("oldText").asText(), d.path("newText").asText());
        maxLines = Math.max(maxLines, view.getLineCount());
        lineHeight = view.getFontMetrics(view.getFont()).getHeight();
        JScrollPane scroll = new JScrollPane(view);
        String shown = inside ? relative(new File(path), sketchFolder) : path;
        JLabel label = new JLabel((isNew ? tr("New file: ") : tr("File: ")) + shown
                                  + "   (+" + view.getAddedCount() + " / -" + view.getRemovedCount() + ")");
        label.setFont(label.getFont().deriveFont(Font.BOLD));
        if (tabs != null) {
          JPanel p = new JPanel(new BorderLayout(0, Theme.scale(4)));
          p.add(label, BorderLayout.NORTH);
          p.add(scroll, BorderLayout.CENTER);
          tabs.addTab(new File(path).getName(), p);
        } else {
          header.add(label);
          single = scroll;
        }
      }
      if (outside) {
        header.add(Box.createVerticalStrut(Theme.scale(4)));
        header.add(warning(tr("Warning: this file is outside the current sketch folder.")));
      }
      body = tabs != null ? tabs : single;
      // as tall as the diff, within limits
      int height = Math.max(Theme.scale(120), Math.min(Theme.scale(480), (maxLines + 2) * lineHeight + Theme.scale(30)));
      body.setPreferredSize(new Dimension(Theme.scale(760), height));
    } else {
      String title = toolCall.path("title").asText("");
      JsonNode raw = toolCall.path("rawInput");
      String command = raw.path("command").asText("");
      if (!title.isEmpty()) {
        JLabel l = new JLabel(title);
        l.setFont(l.getFont().deriveFont(Font.BOLD));
        header.add(l);
      }
      if (isExecute) {
        header.add(Box.createVerticalStrut(Theme.scale(4)));
        header.add(warning(tr("Only allow commands you understand. Commands run on this computer.")));
      }
      JTextArea area = new JTextArea(!command.isEmpty() ? "$ " + command : text.toString());
      area.setEditable(false);
      area.setLineWrap(true);
      area.setWrapStyleWord(false);
      area.setFont(new Font(Font.MONOSPACED, Font.PLAIN, Theme.scale(12)));
      body = new JScrollPane(area);
      body.setPreferredSize(Theme.scale(new Dimension(620, 160)));
    }
    root.add(body, BorderLayout.CENTER);

    JPanel buttons = new JPanel(new FlowLayout(FlowLayout.RIGHT, Theme.scale(6), 0));
    JButton defaultButton = null;
    for (JsonNode o : params.path("options")) {
      String id = o.path("optionId").asText();
      JButton b = new JButton(labelFor(id, o.path("name").asText(id), isEdit));
      b.addActionListener(e -> {
        answer = id;
        dispose();
      });
      buttons.add(b);
      // Enter allows a file edit (it is shown in full); a command must be clicked.
      if (isEdit && id.startsWith("allow") && defaultButton == null) {
        defaultButton = b;
      }
    }
    root.add(buttons, BorderLayout.SOUTH);
    setContentPane(root);
    if (defaultButton != null) {
      getRootPane().setDefaultButton(defaultButton);
    }

    // Closing the window (or Esc) denies the action.
    setDefaultCloseOperation(DISPOSE_ON_CLOSE);
    Base.registerWindowCloseKeys(getRootPane(), e -> {
      answer = findDeny(params);
      dispose();
    });
    addWindowListener(new WindowAdapter() {
      @Override
      public void windowClosing(WindowEvent e) {
        answer = findDeny(params);
      }
    });
    pack();
    setLocationRelativeTo(owner);
  }

  private static JLabel warning(String s) {
    JLabel l = new JLabel(s);
    l.setForeground(new Color(0xB0, 0x30, 0x00));
    l.setFont(l.getFont().deriveFont(Font.BOLD));
    return l;
  }

  private static String labelFor(String optionId, String name, boolean isEdit) {
    switch (optionId) {
      case "allow_once":
        return isEdit ? tr("Allow") : tr("Allow once");
      case "allow_session":
        return tr("Allow for this conversation");
      case "allow_always":
        return tr("Always allow");
      case "deny":
        return tr("Deny");
      case "deny_always":
        return tr("Always deny");
      default:
        return name;
    }
  }

  private static String findDeny(JsonNode params) {
    for (JsonNode o : params.path("options")) {
      if (o.path("kind").asText("").startsWith("reject")) {
        return o.path("optionId").asText();
      }
    }
    return null;
  }

  static boolean isInside(File file, File folder) {
    if (folder == null) {
      return false;
    }
    try {
      String f = file.getCanonicalPath();
      String d = folder.getCanonicalPath();
      return f.equalsIgnoreCase(d) || f.toLowerCase().startsWith(d.toLowerCase() + File.separator);
    } catch (java.io.IOException e) {
      return false;
    }
  }

  private static String relative(File file, File folder) {
    try {
      String f = file.getCanonicalPath();
      String d = folder.getCanonicalPath();
      return f.length() > d.length() ? f.substring(d.length() + 1) : file.getName();
    } catch (java.io.IOException e) {
      return file.getPath();
    }
  }

  /**
   * Shows the dialog and waits for the answer. Must be called on the Swing
   * event thread. Returns the chosen optionId, or the deny option when the
   * dialog is closed.
   */
  public static String ask(Window owner, JsonNode params, File sketchFolder) {
    AiPermissionDialog d = new AiPermissionDialog(owner, params, sketchFolder);
    d.setVisible(true);
    return d.answer != null ? d.answer : findDeny(params);
  }

  /** Short one-line description used in the chat history. */
  static String summary(JsonNode params) {
    JsonNode toolCall = params.path("toolCall");
    for (JsonNode c : toolCall.path("content")) {
      if ("diff".equals(c.path("type").asText())) {
        return new File(c.path("path").asText("")).getName();
      }
    }
    String cmd = toolCall.path("rawInput").path("command").asText("");
    return !cmd.isEmpty() ? cmd : I18n.format("{0}", toolCall.path("title").asText(""));
  }
}
