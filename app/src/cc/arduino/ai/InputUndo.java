/*
 * This file is part of 86Duino IDE.
 *
 * 86Duino IDE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

package cc.arduino.ai;

import java.awt.AWTEvent;
import java.awt.EventQueue;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.InputEvent;
import java.awt.event.InputMethodEvent;
import java.awt.event.KeyEvent;

import javax.swing.AbstractAction;
import javax.swing.InputMap;
import javax.swing.KeyStroke;
import javax.swing.event.DocumentEvent;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.Document;
import javax.swing.text.JTextComponent;
import javax.swing.undo.CannotRedoException;
import javax.swing.undo.CannotUndoException;
import javax.swing.undo.CompoundEdit;
import javax.swing.undo.UndoManager;
import javax.swing.undo.UndoableEdit;

/**
 * Undo (Ctrl+Z) and Redo (Ctrl+Y, Ctrl+Shift+Z) of the message box, the last
 * {@link #STEPS} steps. A step is a word as you type it (the letters up to and
 * with the space or line break after them; a pause of a second starts a new
 * one too), a run of Backspace or Delete, or one action: a paste, a cut,
 * what the input method sends at once, "Attach the compile error messages",
 * and Send clearing the box (Undo brings the message back).
 *
 * The keys are bound in the box itself, so Ctrl+Z there never reaches the
 * Edit menu (which undoes the sketch). The actions are also in the box's
 * ActionMap as "undo" and "redo", for the Edit menu on Mac.
 */
final class InputUndo implements UndoableEditListener {

  /** Steps kept. */
  static final int STEPS = 5;

  /** Typing after a pause this long starts a new step. */
  private static final long PAUSE_MS = 1000;

  static final String UNDO = "undo";
  static final String REDO = "redo";

  private enum Kind {
    TYPE, ERASE, ACTION
  }

  private final Document document;
  private final UndoManager manager = new UndoManager();

  /** The step being added to; ended when the next one starts and before Undo / Redo. */
  private CompoundEdit step;
  private Kind kind;
  private long lastTime;
  /** The step ends with a space or line break: a letter after it starts the next word. */
  private boolean afterSpace;
  /** The event (key, button, input method) that made the last change: its other changes join the step. */
  private AWTEvent lastCause;

  private InputUndo(JTextComponent field) {
    document = field.getDocument();
    manager.setLimit(STEPS);
    document.addUndoableEditListener(this);

    field.getActionMap().put(UNDO, new AbstractAction() {
      @Override
      public void actionPerformed(ActionEvent e) {
        undo();
      }
    });
    field.getActionMap().put(REDO, new AbstractAction() {
      @Override
      public void actionPerformed(ActionEvent e) {
        redo();
      }
    });
    int shortcut = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
    InputMap keys = field.getInputMap();
    keys.put(KeyStroke.getKeyStroke(KeyEvent.VK_Z, shortcut), UNDO);
    keys.put(KeyStroke.getKeyStroke(KeyEvent.VK_Z, shortcut | InputEvent.SHIFT_DOWN_MASK), REDO);
    keys.put(KeyStroke.getKeyStroke(KeyEvent.VK_Y, shortcut), REDO);
  }

  /** Gives the field Undo and Redo of its last {@link #STEPS} steps. */
  static void install(JTextComponent field) {
    new InputUndo(field);
  }

  @Override
  public void undoableEditHappened(UndoableEditEvent e) {
    UndoableEdit edit = e.getEdit();
    long now = System.currentTimeMillis();
    AWTEvent cause = EventQueue.getCurrentEvent();
    // Only changes that are kept come here: the input method's text being
    // composed is not one (Swing leaves it out of Undo), what it sends is.
    String added = "";
    int length = 0;
    boolean insert = false;
    if (edit instanceof DocumentEvent) {
      DocumentEvent d = (DocumentEvent) edit;
      length = d.getLength();
      insert = d.getType() == DocumentEvent.EventType.INSERT;
      if (insert) {
        try {
          added = document.getText(d.getOffset(), length);
        } catch (BadLocationException ex) {
          added = "";
        }
      }
    }
    if (step != null && cause != null && cause == lastCause) {
      // the same key or button: one step (typing over a selection, Attach, ...)
      step.addEdit(edit);
      afterSpace = insert && endsWithSpace(added);
      lastTime = now;
      return;
    }
    // what the input method sends at once is a step of its own (it may come one character at a time)
    boolean inputMethod = cause instanceof InputMethodEvent;
    Kind k = length != 1 || inputMethod ? Kind.ACTION : insert ? Kind.TYPE : Kind.ERASE;
    boolean space = insert && length == 1 && Character.isWhitespace(added.charAt(0));
    boolean newStep = step == null || k == Kind.ACTION || kind != k || kind == Kind.ACTION
                      || now - lastTime > PAUSE_MS || (k == Kind.TYPE && afterSpace && !space);
    if (newStep) {
      if (step != null) {
        step.end();
      }
      step = new CompoundEdit();
      step.addEdit(edit);
      manager.addEdit(step);
      kind = k;
    } else {
      step.addEdit(edit);
    }
    afterSpace = insert && endsWithSpace(added);
    lastTime = now;
    lastCause = cause;
  }

  private static boolean endsWithSpace(String s) {
    return !s.isEmpty() && Character.isWhitespace(s.charAt(s.length() - 1));
  }

  /** The step being typed is finished: Undo takes it back whole, and what comes next is a new step. */
  private void endStep() {
    if (step != null) {
      step.end();
      step = null;
    }
    lastCause = null;
  }

  private void undo() {
    endStep();
    try {
      if (manager.canUndo()) {
        manager.undo();
      }
    } catch (CannotUndoException e) {
      // nothing left to undo
    }
  }

  private void redo() {
    endStep();
    try {
      if (manager.canRedo()) {
        manager.redo();
      }
    } catch (CannotRedoException e) {
      // nothing to redo
    }
  }
}
