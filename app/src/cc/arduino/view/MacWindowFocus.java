/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.view;

import java.awt.AWTEvent;
import java.awt.Component;
import java.awt.Dialog;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.MouseEvent;

import javax.swing.JDialog;
import javax.swing.SwingUtilities;
import javax.swing.Timer;

import processing.app.PreferencesData;
import processing.app.helpers.OSUtils;

/**
 * macOS: gives the keyboard back to a window the user clicks after using
 * another application.
 *
 * Java 8 (Temurin 8u504, macOS 26) often misses the "window became key" event
 * when the user switches to another app and clicks straight back into an IDE
 * window: the click selects text, but the window stays inactive, so there is
 * no caret and typing goes nowhere until the window is minimized and restored.
 * toFront() and requestFocus() do not help. Showing and closing a tiny
 * transparent dialog owned by the window makes macOS move the key window
 * twice, and Java does receive that (about 0.2 s, nothing visible).
 *
 * preferences.txt: gui.macos.focus_fix=on (default) | off
 */
public final class MacWindowFocus {

  public static final String PREF = "gui.macos.focus_fix";

  /** A normal activation may arrive just after the click: only fix what is still inactive then. */
  private static final int CHECK_DELAY_MS = 150;
  private static final int BOUNCE_MS = 120;

  private static boolean busy;

  private MacWindowFocus() {
  }

  /** Call once at startup. Does nothing outside macOS. */
  public static boolean install() {
    if (!OSUtils.isMacOS()) {
      return false;
    }
    if ("off".equalsIgnoreCase(PreferencesData.get(PREF, "on").trim())) {
      return false;
    }
    Toolkit.getDefaultToolkit().addAWTEventListener(MacWindowFocus::mouseEvent, AWTEvent.MOUSE_EVENT_MASK);
    return true;
  }

  private static void mouseEvent(AWTEvent event) {
    if (event.getID() != MouseEvent.MOUSE_PRESSED || busy) {
      return;
    }
    Component clicked = ((MouseEvent) event).getComponent();
    Window window = clicked instanceof Window ? (Window) clicked : SwingUtilities.getWindowAncestor(clicked);
    if (!needsFix(window)) {
      return;
    }
    busy = true;
    Timer check = new Timer(CHECK_DELAY_MS, e -> {
      if (needsFix(window)) {
        bounce(window, clicked);
      } else {
        busy = false;
      }
    });
    check.setRepeats(false);
    check.start();
  }

  /** Frames and dialogs only: popups and tooltips are never active windows. */
  static boolean needsFix(Window w) {
    return (w instanceof Frame || w instanceof Dialog) && w.isShowing()
        && w.getFocusableWindowState() && !w.isActive();
  }

  private static void bounce(Window window, Component clicked) {
    JDialog helper;
    try {
      helper = new JDialog(window);
      helper.setUndecorated(true);
      helper.setSize(1, 1);
      helper.setLocation(window.getX() + 10, window.getY() + 10);
      try {
        helper.setOpacity(0f);
      } catch (Throwable ignore) {
        // translucency not supported: a 1x1 pixel is still hard to see
      }
      helper.setVisible(true);
    } catch (Throwable t) {
      busy = false;
      return;
    }
    Timer close = new Timer(BOUNCE_MS, e -> {
      helper.dispose();
      window.toFront();
      if (clicked != null && clicked.isShowing() && clicked.isFocusable()) {
        clicked.requestFocusInWindow();
      }
      busy = false;
    });
    close.setRepeats(false);
    close.start();
  }
}
