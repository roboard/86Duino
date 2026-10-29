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
import java.awt.CardLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.JTextPane;
import javax.swing.SwingUtilities;
import javax.swing.text.BadLocationException;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;

import com.fasterxml.jackson.databind.JsonNode;

import processing.app.Base;
import processing.app.BaseNoGui;
import processing.app.Editor;
import processing.app.EditorTab;
import processing.app.I18n;
import processing.app.PreferencesData;
import processing.app.Sketch;
import processing.app.SketchController;
import processing.app.SketchFile;
import processing.app.Theme;
import processing.app.helpers.Keys;

/**
 * The AI assistant side panel of one editor window. It owns one "hermes acp"
 * process whose working directory is the sketch folder, shows the
 * conversation, asks the user before the AI changes files or runs commands,
 * and reloads the sketch after the AI changed it.
 */
public class AiChatPanel extends JPanel implements AcpClient.Listener {

  private static final String CARD_CHAT = "chat";
  private static final String CARD_SETUP = "setup";

  private final Editor editor;
  private final Base base;

  private final JTextPane transcript = new JTextPane();
  private final JTextArea input = new JTextArea(3, 20);
  private final JButton sendButton = new JButton(tr("Send"));
  private final JButton stopButton = new JButton(tr("Stop"));
  private final JButton newButton = new JButton(tr("New Chat"));
  private final JLabel statusLabel = new JLabel(" ");
  /** The model in use, next to the title. */
  private final JLabel modelLabel = new JLabel(" ");
  /** "Retry", shown when the model list could not be fetched. */
  private final JButton retryButton = new JButton(tr("Retry"));
  /** A text area, not an HTML label: HTML labels do not wrap Chinese text. */
  private final JTextArea setupText = new JTextArea();
  private final JPanel cards = new JPanel(new CardLayout());

  private final SimpleAttributeSet userStyle = new SimpleAttributeSet();
  private final SimpleAttributeSet userLabelStyle = new SimpleAttributeSet();
  private final SimpleAttributeSet aiStyle = new SimpleAttributeSet();
  private final SimpleAttributeSet aiLabelStyle = new SimpleAttributeSet();
  private final SimpleAttributeSet toolStyle = new SimpleAttributeSet();
  private final SimpleAttributeSet infoStyle = new SimpleAttributeSet();
  private final SimpleAttributeSet errorStyle = new SimpleAttributeSet();

  // Conversation state. Changed on the event thread only, except where noted.
  private AcpClient client;
  private String sessionId;
  private File sessionFolder;
  private String sessionSettings;
  private String sessionBoard;
  private boolean contextSent;
  private volatile boolean busy;
  private boolean lastWasAssistantText;
  private boolean assistantLabelShown;
  /** Settings the current session was made (or continued) with, see {@link ChatHistory#aiSignature}. */
  private String sessionAi;

  // The conversation saved in the sketch (data\.ai\chat.json).
  private ChatHistory history = new ChatHistory();
  private boolean historyLoaded;
  /** Something was shown since the last save. */
  private boolean historyDirty;
  /** False while a saved conversation is drawn again, so it is not recorded twice. */
  private boolean recording = true;
  /** A saved session to continue with the next message, or null. */
  private String resumeSessionId;
  /** Set (by the turn thread) while Hermes replays a continued session: that replay is not shown. */
  private volatile boolean replaying;

  /** Edit tool calls of the current turn: toolCallId to the files it touches. */
  private final Map<String, List<File>> editToolCalls = new ConcurrentHashMap<>();
  /** Every tool call of the current turn, so failures can be reported. */
  private final Map<String, String> toolCallTitles = new ConcurrentHashMap<>();

  public AiChatPanel(Editor editor, Base base) {
    super(new BorderLayout());
    this.editor = editor;
    this.base = base;
    setMinimumSize(Theme.scale(new Dimension(260, 200)));
    setPreferredSize(Theme.scale(new Dimension(380, 400)));

    // Header: title and buttons, the status line below
    JPanel header = new JPanel(new BorderLayout(Theme.scale(6), Theme.scale(2)));
    header.setBorder(BorderFactory.createEmptyBorder(Theme.scale(4), Theme.scale(8), Theme.scale(4), Theme.scale(4)));
    JLabel title = new JLabel(tr("AI Assistant"));
    title.setFont(title.getFont().deriveFont(Font.BOLD));
    header.add(title, BorderLayout.WEST);
    modelLabel.setForeground(Color.GRAY);
    header.add(modelLabel, BorderLayout.CENTER);
    JPanel headerButtons = new JPanel(new FlowLayout(FlowLayout.RIGHT, Theme.scale(2), 0));
    newButton.setToolTipText(tr("Start a new conversation"));
    newButton.setMargin(new java.awt.Insets(1, 6, 1, 6));
    newButton.addActionListener(e -> newConversation());
    headerButtons.add(newButton);
    JButton hide = new JButton("×");
    hide.setMargin(new java.awt.Insets(1, 6, 1, 6));
    hide.setToolTipText(tr("Hide the AI Assistant"));
    hide.addActionListener(e -> editor.setAiPanelVisible(false));
    headerButtons.add(hide);
    header.add(headerButtons, BorderLayout.EAST);
    statusLabel.setForeground(Color.GRAY);
    header.add(statusLabel, BorderLayout.SOUTH);
    add(header, BorderLayout.NORTH);

    // Chat card
    transcript.setEditable(false);
    transcript.setBorder(BorderFactory.createEmptyBorder(Theme.scale(6), Theme.scale(8), Theme.scale(6), Theme.scale(8)));
    JScrollPane transcriptScroll = new JScrollPane(transcript, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                                   JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    transcriptScroll.setBorder(BorderFactory.createMatteBorder(1, 0, 1, 0, Color.LIGHT_GRAY));

    input.setLineWrap(true);
    input.setWrapStyleWord(true);
    input.setFont(transcript.getFont());
    input.setToolTipText(tr("Enter: send, Shift+Enter: new line"));
    input.addKeyListener(new KeyAdapter() {
      @Override
      public void keyPressed(KeyEvent e) {
        if (e.getKeyCode() == KeyEvent.VK_ENTER && !e.isShiftDown() && !e.isControlDown()) {
          e.consume();
          send();
        }
      }
    });
    sendButton.addActionListener(e -> send());
    stopButton.addActionListener(e -> stop());
    stopButton.setEnabled(false);

    JPanel inputPanel = new JPanel(new BorderLayout(Theme.scale(4), 0));
    inputPanel.setBorder(BorderFactory.createEmptyBorder(Theme.scale(6), Theme.scale(6), Theme.scale(6), Theme.scale(6)));
    inputPanel.add(new JScrollPane(input), BorderLayout.CENTER);
    JPanel inputButtons = new JPanel(new java.awt.GridLayout(2, 1, 0, Theme.scale(4)));
    inputButtons.add(sendButton);
    inputButtons.add(stopButton);
    // a taller message box keeps the buttons at their size, at the top
    JPanel buttonColumn = new JPanel(new BorderLayout());
    buttonColumn.add(inputButtons, BorderLayout.NORTH);
    inputPanel.add(buttonColumn, BorderLayout.EAST);

    // The border between the conversation and the message box can be dragged:
    // the message box keeps at least its button column, and takes at most
    // three quarters of the height.
    int inputMin = inputPanel.getPreferredSize().height;
    inputPanel.setMinimumSize(new Dimension(0, inputMin));
    JSplitPane chat = new JSplitPane(JSplitPane.VERTICAL_SPLIT, transcriptScroll, inputPanel);
    chat.setContinuousLayout(true);
    chat.setResizeWeight(1D); // a taller window gives the conversation the room
    chat.setBorder(null);
    chat.setDividerSize(Theme.scale(6));
    // the divider draws no lines of its own (the conversation's bottom line is
    // the only separator); it still shows the resize cursor and can be dragged
    if (chat.getUI() instanceof javax.swing.plaf.basic.BasicSplitPaneUI) {
      ((javax.swing.plaf.basic.BasicSplitPaneUI) chat.getUI()).getDivider().setBorder(null);
    }
    Keys.killBinding(chat, Keys.ctrl(KeyEvent.VK_TAB));
    Keys.killBinding(chat, Keys.ctrlShift(KeyEvent.VK_TAB));
    // read now: the split pane's own first layout moves the divider before it is placed
    int savedInput = PreferencesData.getInteger(AiSettings.PREF_INPUT_HEIGHT, inputMin);
    boolean[] placed = new boolean[1];
    chat.addComponentListener(new java.awt.event.ComponentAdapter() {
      @Override
      public void componentResized(java.awt.event.ComponentEvent e) {
        int height = chat.getHeight();
        if (height <= 0) {
          return;
        }
        int maxInput = Math.max(inputMin, height * 3 / 4);
        transcriptScroll.setMinimumSize(new Dimension(0, height - maxInput - chat.getDividerSize()));
        int input = placed[0] ? height - chat.getDividerLocation() - chat.getDividerSize() : savedInput;
        input = Math.max(inputMin, Math.min(maxInput, input));
        placed[0] = true;
        chat.setDividerLocation(height - input - chat.getDividerSize());
      }
    });
    chat.addPropertyChangeListener(JSplitPane.DIVIDER_LOCATION_PROPERTY, e -> {
      int input = chat.getHeight() - chat.getDividerLocation() - chat.getDividerSize();
      if (placed[0] && input >= inputMin) {
        PreferencesData.setInteger(AiSettings.PREF_INPUT_HEIGHT, input);
      }
    });
    cards.add(chat, CARD_CHAT);

    // Setup card, shown while no API key is saved
    JPanel setup = new JPanel(new BorderLayout(0, Theme.scale(10)));
    setup.setBorder(BorderFactory.createEmptyBorder(Theme.scale(20), Theme.scale(16), Theme.scale(20), Theme.scale(16)));
    setupText.setEditable(false);
    setupText.setFocusable(false);
    setupText.setOpaque(false);
    setupText.setLineWrap(true);
    setupText.setWrapStyleWord(false);
    setupText.setFont(new JLabel().getFont());
    setupText.setBorder(null);
    setup.add(setupText, BorderLayout.NORTH);
    JButton openPrefs = new JButton(tr("Open AI Settings..."));
    openPrefs.addActionListener(e -> openSettings());
    retryButton.addActionListener(e -> {
      pickFailedFor = null; // try the model list again
      refreshSetupState();
    });
    JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT, 0, 0));
    p.add(retryButton);
    p.add(Box.createHorizontalStrut(Theme.scale(6)));
    p.add(openPrefs);
    setup.add(p, BorderLayout.CENTER);
    cards.add(setup, CARD_SETUP);
    add(cards, BorderLayout.CENTER);

    StyleConstants.setBold(userLabelStyle, true);
    StyleConstants.setForeground(userLabelStyle, new Color(0x1F, 0x5F, 0xA8));
    StyleConstants.setForeground(userStyle, new Color(0x20, 0x20, 0x20));
    StyleConstants.setBold(aiLabelStyle, true);
    StyleConstants.setForeground(aiLabelStyle, new Color(0x00, 0x87, 0x7F));
    StyleConstants.setForeground(aiStyle, new Color(0x20, 0x20, 0x20));
    StyleConstants.setForeground(toolStyle, new Color(0x70, 0x70, 0x70));
    StyleConstants.setFontSize(toolStyle, Math.max(10, transcript.getFont().getSize() - 1));
    StyleConstants.setForeground(infoStyle, new Color(0x80, 0x80, 0x80));
    StyleConstants.setItalic(infoStyle, true);
    StyleConstants.setForeground(errorStyle, new Color(0xB0, 0x20, 0x20));

    refreshSetupState();
  }

  // ------------------------------------------------------------------ setup

  /** Shows the setup hint instead of the chat when the assistant cannot run. */
  public void refreshSetupState() {
    loadHistory();
    AiSettings settings = AiSettings.load();
    HermesBundle bundle = HermesBundle.getDefault();
    String problem = setupProblem(settings, bundle);
    if (problem != null && AiPreferencesPanel.isSaving()) {
      // OK was just pressed: the key may not be on disk yet
      problem = tr("Saving the AI settings...");
    } else if (problem != null && pickModelAutomatically(settings, bundle)) {
      problem = tr("Choosing an AI model...");
    }
    boolean listFailed = problem != null && modelListFailed(settings, bundle);
    if (listFailed) {
      problem = tr("Could not get the model list. Please check the network connection.");
      if (!pickFailedError.isEmpty()) {
        problem += "<br><br>" + I18n.format(tr("Error: {0}"), pickFailedError);
      }
    }
    retryButton.setVisible(listFailed);
    if (problem == null) {
      String model = settings.model.trim();
      modelLabel.setText(I18n.format(tr("Model: {0}"), model));
      modelLabel.setToolTipText(settings.getProvider().label + " / " + model);
      ((CardLayout) cards.getLayout()).show(cards, CARD_CHAT);
      newButton.setEnabled(true);
    } else {
      modelLabel.setText(" ");
      modelLabel.setToolTipText(null);
      // the messages are shared with HTML labels: turn them into plain text
      setupText.setText(problem.replace("<br>", "\n").replace("&gt;", ">").replace("&lt;", "<").replace("&amp;", "&"));
      ((CardLayout) cards.getLayout()).show(cards, CARD_SETUP);
      newButton.setEnabled(false);
    }
  }

  private static String setupProblem(AiSettings s, HermesBundle bundle) {
    if (!bundle.isInstalled()) {
      return I18n.format(tr("The AI component (Hermes Agent) was not found in:<br>{0}"), bundle.getBundleDir().getAbsolutePath());
    }
    if (!bundle.hasApiKey(s.getProvider().apiKeyEnv)) {
      return tr("The AI Assistant did not find an API key for the AI model. Please set it in Preferences &gt; AI.");
    }
    if (s.model == null || s.model.trim().isEmpty()) {
      return tr("No AI model is selected. Choose one in Preferences &gt; AI.");
    }
    return null;
  }

  /** An automatic model choice is running (shared by all editor windows). */
  private static final java.util.concurrent.atomic.AtomicBoolean pickingModel = new java.util.concurrent.atomic.AtomicBoolean();
  /** The key + provider an automatic choice already failed for, so it is not retried in a loop. */
  private static volatile String pickFailedFor;
  /** Why that choice failed (the endpoint's or the connection's error), or "". */
  private static volatile String pickFailedError = "";

  private static String pickAttempt(AiSettings s, HermesBundle bundle) {
    AiSettings.Provider p = s.getProvider();
    return p.id + "|" + bundle.apiKeyFingerprint(p.apiKeyEnv) + "|" + s.baseUrl;
  }

  /** A key is saved, no model is, and the model list could not be fetched for it. */
  private static boolean modelListFailed(AiSettings s, HermesBundle bundle) {
    boolean modelMissing = s.model == null || s.model.trim().isEmpty();
    return bundle.isInstalled() && modelMissing && bundle.hasApiKey(s.getProvider().apiKeyEnv)
           && pickAttempt(s, bundle).equals(pickFailedFor);
  }

  /**
   * A key is saved but no model is (e.g. OK was pressed before Test Connection):
   * choose the model the Preferences page would recommend for this key (newest;
   * a flash model for Gemini), save it, and open the chat. Returns true while
   * the choice is running.
   */
  private static boolean pickModelAutomatically(AiSettings s, HermesBundle bundle) {
    AiSettings.Provider p = s.getProvider();
    boolean modelMissing = s.model == null || s.model.trim().isEmpty();
    if (!bundle.isInstalled() || !modelMissing || !bundle.hasApiKey(p.apiKeyEnv)
        || (p.needsBaseUrl && (s.baseUrl == null || s.baseUrl.trim().isEmpty()))) {
      return false;
    }
    String attempt = pickAttempt(s, bundle);
    if (attempt.equals(pickFailedFor)) {
      return false; // show "could not get the model list" (and Retry) instead of trying again
    }
    if (!pickingModel.compareAndSet(false, true)) {
      return true; // another window is already choosing
    }
    Thread t = new Thread(() -> {
      String model = "";
      String error = "";
      try {
        HermesBundle.ModelList list = bundle.listModels(p.id, bundle.readEnvValue(p.apiKeyEnv), s.baseUrl);
        model = !list.recommended.isEmpty() ? list.recommended : list.models.isEmpty() ? "" : list.models.get(0);
        error = list.error == null ? "" : list.error.trim();
        if (!model.isEmpty()) {
          s.model = model;
          bundle.applySettings(s, null, false); // Hermes' model.default
        }
      } catch (Exception e) {
        model = "";
        error = e.getMessage() == null ? "" : e.getMessage().trim();
      }
      String chosen = model;
      String why = error.length() > 300 ? error.substring(0, 300) + "..." : error;
      SwingUtilities.invokeLater(() -> {
        pickingModel.set(false);
        if (chosen.isEmpty()) {
          pickFailedError = why;
          pickFailedFor = attempt;
        } else if (AiSettings.load().model.trim().isEmpty()) {
          // written to preferences.txt when the IDE quits, like the other settings
          processing.app.PreferencesData.set(AiSettings.PREF_MODEL, chosen);
        }
        AiPreferencesPanel.refreshAllEditors();
      });
    }, "ai-pick-model");
    t.setDaemon(true);
    t.start();
    return true;
  }

  // ------------------------------------------------------------------ saved conversation

  /** Where the conversation of this sketch is kept; null for an untitled or read-only sketch. */
  private File historyFile() {
    Sketch sketch = editor.getSketch();
    SketchController controller = editor.getSketchController();
    if (sketch == null || controller == null || controller.isUntitled() || controller.isReadOnly()) {
      return null;
    }
    return ChatHistory.fileFor(sketch);
  }

  /**
   * Shows the conversation saved in the sketch, once, when the panel first
   * appears. The AI continues it with the next message when the AI settings
   * are still the ones it was made with.
   */
  private void loadHistory() {
    if (historyLoaded || editor.getSketch() == null || busy) {
      return;
    }
    historyLoaded = true;
    File file = historyFile();
    ChatHistory saved = file == null ? null : ChatHistory.load(file);
    if (saved == null || saved.entries.isEmpty()) {
      return;
    }
    history = saved;
    String when = saved.updated > 0
                  ? new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm").format(new java.util.Date(saved.updated)) : "";
    showHistory(I18n.format(tr("Previous conversation: {0}"), when));
    if (saved.sessionId != null && saved.aiSignature.equals(ChatHistory.aiSignature(AiSettings.load()))) {
      resumeSessionId = saved.sessionId;
    } else if (saved.sessionId != null) {
      recording = false;
      appendInfo(tr("The AI settings changed since then, so the AI does not remember the conversation above. The next message starts a new conversation."));
      recording = true;
    }
  }

  /** Saves the conversation in the sketch; called after every turn and when the window closes. */
  private void saveHistory() {
    File file = historyFile();
    if (file == null || !historyDirty || history.entries.isEmpty()) {
      return;
    }
    historyDirty = false;
    // without a session (Hermes did not start) the saved one is kept for the next try
    if (sessionId != null) {
      history.sessionId = sessionId;
      history.aiSignature = sessionAi;
      history.board = sessionBoard == null ? "" : sessionBoard;
    }
    history.sketchFolder = editor.getSketch().getFolder().getAbsolutePath();
    history.updated = System.currentTimeMillis();
    if (history.trim() && !busy) {
      showHistory(null); // the screen keeps the same messages as the file
    }
    try {
      history.save(file);
    } catch (IOException e) {
      System.err.println("Could not save the AI conversation " + file + ": " + e);
    }
  }

  /** Everything that needs a new Hermes process when it changes, the API key included. */
  private static String sessionSignature(AiSettings s, HermesBundle bundle) {
    return s.runtimeSignature() + "|" + bundle.apiKeyFingerprint(s.getProvider().apiKeyEnv);
  }

  private void openSettings() {
    cc.arduino.view.preferences.Preferences dialog = new cc.arduino.view.preferences.Preferences(editor, base);
    dialog.selectAiTab();
    dialog.setLocationRelativeTo(editor);
    dialog.setVisible(true);
  }

  /** Called after Preferences were saved. */
  public void settingsChanged() {
    refreshSetupState();
    AiSettings s = AiSettings.load();
    HermesBundle bundle = HermesBundle.getDefault();
    if (client != null && !busy && !sessionSignature(s, bundle).equals(sessionSettings)) {
      // The next message starts Hermes again with the new settings.
      shutdownClient();
      if (setupProblem(s, bundle) == null) {
        appendInfo(tr("AI settings changed. The next message starts a new conversation."));
      }
    }
  }

  public boolean isBusy() {
    return busy;
  }

  // ------------------------------------------------------------------ actions

  public void focusInput() {
    input.requestFocusInWindow();
  }

  private void setBusy(boolean b, String status) {
    busy = b;
    sendButton.setEnabled(!b);
    stopButton.setEnabled(b);
    statusLabel.setText(status == null ? " " : status);
  }

  private void send() {
    String text = input.getText().trim();
    if (text.isEmpty() || busy) {
      return;
    }
    AiSettings settings = AiSettings.load();
    HermesBundle bundle = HermesBundle.getDefault();
    String problem = setupProblem(settings, bundle);
    if (problem != null) {
      refreshSetupState();
      return;
    }
    if (!prepareSketch()) {
      return;
    }
    Sketch sketch = editor.getSketch();
    File folder = sketch.getFolder();

    // Everything that touches IDE state is read here, on the event thread.
    boolean restart = client == null || !client.isAlive()
                      || !sessionSignature(settings, bundle).equals(sessionSettings);
    boolean newSession = restart || sessionId == null || !folder.equals(sessionFolder);
    if (newSession && sessionId != null && !folder.equals(sessionFolder)) {
      appendInfo(tr("The sketch folder changed, so a new conversation was started."));
    }
    // continue the conversation saved in the sketch (only as the first session of this window)
    String resume = newSession && sessionId == null ? resumeSessionId : null;
    if (resume != null && !history.aiSignature.equals(ChatHistory.aiSignature(settings))) {
      appendInfo(tr("The AI settings changed since then, so the AI does not remember the conversation above. The next message starts a new conversation."));
      resume = null;
    }
    resumeSessionId = null;
    List<String> blocks = new ArrayList<>();
    String board = AiContextBuilder.boardKey();
    if (newSession || !contextSent) {
      blocks.add(AiContextBuilder.build(sketch));
    } else if (!board.equals(sessionBoard)) {
      blocks.add(AiContextBuilder.boardChanged());
    }
    blocks.add(text);

    input.setText("");
    appendUser(text);
    editToolCalls.clear();
    toolCallTitles.clear();
    answeredThisTurn.clear();
    setBusy(true, restart ? tr("Starting the AI...") : tr("Thinking..."));

    AcpClient currentClient = restart ? null : client;
    String currentSession = newSession ? null : sessionId;
    if (restart) {
      shutdownClient();
    }
    String resumeId = resume;
    Thread worker = new Thread(() -> runTurn(bundle, settings, folder, currentClient, currentSession, resumeId, board, blocks), "ai-turn");
    worker.setDaemon(true);
    worker.start();
  }

  /**
   * Makes sure the AI works on the files the user sees: unsaved changes are
   * saved, and a read-only example must first be saved somewhere else.
   */
  private boolean prepareSketch() {
    SketchController controller = editor.getSketchController();
    if (controller.isReadOnly()) {
      int r = JOptionPane.showConfirmDialog(editor,
          tr("This sketch is read-only (for example a built-in example), so the AI cannot change it.\nSave a copy of the sketch first?"),
          tr("AI Assistant"), JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);
      return r == JOptionPane.YES_OPTION && editor.handleSaveAs();
    }
    Sketch sketch = editor.getSketch();
    if (sketch.isModified()) {
      try {
        if (controller.isUntitled()) {
          // An untitled sketch already lives in a temporary folder: save there
          // without asking for a name.
          sketch.save();
        } else if (!editor.handleSave(true)) {
          return false;
        }
      } catch (IOException e) {
        appendError(tr("Could not save the sketch: ") + e.getMessage());
        return false;
      }
    }
    return true;
  }

  /**
   * Runs on a background thread. A null client means start Hermes first; a
   * null session id means create a new conversation first.
   */
  private void runTurn(HermesBundle bundle, AiSettings settings, File folder, AcpClient c,
                       String sid, String resumeId, String board, List<String> blocks) {
    try {
      AiPreferencesPanel.waitForPendingSave();
      if (c == null) {
        try {
          bundle.ensureIdeConfig(settings);
        } catch (IOException e) {
          System.err.println("Could not write the AI settings: " + e.getMessage());
        }
        c = new AcpClient(this);
        c.start(bundle, folder);
        AcpClient started = c;
        onEdt(() -> client = started);
        c.initialize("86Duino IDE", BaseNoGui.VERSION_NAME_LONG).get(120, TimeUnit.SECONDS);
      }
      if (sid == null) {
        // the conversation saved in the sketch first: Hermes keeps it in the AI workspace
        if (resumeId != null && resumeSession(c, resumeId, folder)) {
          sid = resumeId;
          // the saved session keeps its old mode: use today's setting
          c.setMode(sid, settings.confirmEdits ? "default" : "accept_edits").get(30, TimeUnit.SECONDS);
        } else {
          if (resumeId != null) {
            onEdt(() -> appendInfo(tr("The AI could not continue the previous conversation (it is not in this AI workspace), so the AI does not remember the conversation above. A new conversation was started.")));
          }
          JsonNode res = c.newSession(folder).get(120, TimeUnit.SECONDS);
          sid = res.path("sessionId").asText(null);
          if (sid == null) {
            throw new IOException("Hermes did not create a session");
          }
          if (!settings.confirmEdits) {
            c.setMode(sid, "accept_edits").get(30, TimeUnit.SECONDS);
          }
        }
        String newSid = sid;
        String signature = sessionSignature(settings, bundle);
        String ai = ChatHistory.aiSignature(settings);
        onEdt(() -> {
          sessionId = newSid;
          sessionFolder = folder;
          sessionSettings = signature;
          sessionAi = ai;
          contextSent = false;
        });
      }
      onEdt(() -> statusLabel.setText(tr("Thinking...")));
      JsonNode result = c.prompt(sid, blocks).get();
      String stopReason = result.path("stopReason").asText("");
      onEdt(() -> {
        contextSent = true;
        sessionBoard = board;
        finishTurn();
        if ("cancelled".equals(stopReason)) {
          appendInfo(tr("Stopped."));
        } else if ("max_tokens".equals(stopReason) || "max_turn_requests".equals(stopReason)) {
          appendInfo(tr("The AI stopped because it reached its limit."));
        }
      });
    } catch (ExecutionException e) {
      failTurn(e.getCause() != null ? e.getCause() : e);
    } catch (Exception e) {
      failTurn(e);
    }
  }

  /**
   * Asks Hermes to continue a saved session. Its history replay is not shown:
   * the panel already shows the saved conversation. False when Hermes does not
   * have the session (another computer or AI workspace) or cannot load it.
   */
  private boolean resumeSession(AcpClient c, String id, File folder) {
    replaying = true;
    try {
      JsonNode res = c.loadSession(id, folder).get(120, TimeUnit.SECONDS);
      // an unknown session is answered with null or {} (a loaded one lists its modes and models)
      return res != null && res.isObject() && res.size() > 0;
    } catch (Exception e) {
      System.err.println("Could not continue the AI conversation " + id + ": " + e);
      return false;
    } finally {
      replaying = false;
    }
  }

  private void failTurn(Throwable t) {
    onEdt(() -> {
      finishTurn();
      String msg = t.getMessage() == null ? t.toString() : t.getMessage();
      appendError(tr("The AI request failed: ") + msg);
      appendInfo(I18n.format(tr("Details are in the log file: {0}"), HermesBundle.getDefault().getLogFile().getAbsolutePath()));
    });
  }

  private void finishTurn() {
    setBusy(false, null);
    lastWasAssistantText = false;
    reloadSketchFromDisk();
    // after the lines the caller adds ("Stopped.", errors)
    SwingUtilities.invokeLater(this::saveHistory);
  }

  private void stop() {
    if (busy && client != null && sessionId != null) {
      client.cancel(sessionId);
      statusLabel.setText(tr("Stopping..."));
    }
  }

  private void newConversation() {
    if (busy) {
      stop();
      return;
    }
    if (transcript.getDocument().getLength() > 0 && !confirmNewConversation()) {
      return;
    }
    sessionId = null;
    contextSent = false;
    transcript.setText("");
    lastWasAssistantText = false;
    statusLabel.setText(" ");
    // the saved conversation goes too
    history.clear();
    historyDirty = false;
    resumeSessionId = null;
    File file = historyFile();
    if (file != null) {
      ChatHistory.delete(file);
    }
    focusInput();
  }

  /** New Chat throws the conversation away for good: ask first, with "No" as the default button. */
  private boolean confirmNewConversation() {
    String message = historyFile() != null
        ? tr("Starting a new conversation deletes the conversation saved in this sketch, and the AI will not remember it.\nContinue?")
        : tr("Starting a new conversation clears the current conversation, and the AI will not remember it.\nContinue?");
    Object[] options = {javax.swing.UIManager.getString("OptionPane.yesButtonText"),
                        javax.swing.UIManager.getString("OptionPane.noButtonText")};
    int answer = JOptionPane.showOptionDialog(editor, message, tr("New Chat"), JOptionPane.YES_NO_OPTION,
                                              JOptionPane.WARNING_MESSAGE, null, options, options[1]);
    return answer == 0;
  }

  /** Stops Hermes. Called when the window closes or the settings change. */
  public void shutdownClient() {
    AcpClient c = client;
    client = null;
    sessionId = null;
    contextSent = false;
    if (c != null) {
      Thread t = new Thread(c::close, "acp-close");
      t.setDaemon(true);
      t.start();
    }
  }

  /** Stops Hermes and waits for it (the editor window is closing). */
  public void closeAssistant() {
    saveHistory(); // a turn that is still running is saved as far as it got
    AcpClient c = client;
    client = null;
    if (c != null) {
      c.close();
    }
  }

  // ------------------------------------------------------------------ ACP callbacks

  @Override
  public void onSessionUpdate(JsonNode update) {
    if (replaying) {
      return; // the history of a continued session: already on screen
    }
    String kind = update.path("sessionUpdate").asText("");
    switch (kind) {
      case "agent_message_chunk": {
        JsonNode content = update.path("content");
        if ("text".equals(content.path("type").asText())) {
          String text = content.path("text").asText("");
          onEdt(() -> appendAssistant(text));
        }
        break;
      }
      case "agent_thought_chunk":
        onEdt(() -> {
          if (busy) {
            statusLabel.setText(tr("Thinking..."));
          }
        });
        break;
      case "tool_call": {
        String id = update.path("toolCallId").asText("");
        String toolKind = update.path("kind").asText("");
        List<File> files = new ArrayList<>();
        for (JsonNode loc : update.path("locations")) {
          if (loc.hasNonNull("path")) {
            files.add(new File(loc.get("path").asText()));
          }
        }
        if ("edit".equals(toolKind) || "delete".equals(toolKind) || "move".equals(toolKind)) {
          editToolCalls.put(id, files);
        }
        String line = describeTool(toolKind, update.path("title").asText(""), files);
        toolCallTitles.put(id, line);
        onEdt(() -> {
          appendTool("• " + line);
          statusLabel.setText(line);
        });
        break;
      }
      case "tool_call_update": {
        String id = update.path("toolCallId").asText("");
        String status = update.path("status").asText("");
        if ("completed".equals(status) && editToolCalls.containsKey(id)) {
          // The AI changed a file: show it in the editor right away.
          onEdt(this::reloadSketchFromDisk);
        } else if ("failed".equals(status) && toolCallTitles.containsKey(id)) {
          String text = contentText(update);
          // A denial is already shown as "Denied: ..." by the permission handler.
          if (!text.contains("denied")) {
            String reason = shortenPaths(firstLine(text));
            onEdt(() -> appendTool("  ✗ " + (reason.isEmpty() ? tr("failed") : reason)));
          }
        }
        break;
      }
      default:
        // plan, usage_update, session_info_update, available_commands_update: not shown
    }
  }

  /** The AI may ask for several edits at once: show one dialog at a time. */
  private final Object permissionLock = new Object();

  /** Answers given in the current turn, by request content. */
  private final Map<String, String> answeredThisTurn = new ConcurrentHashMap<>();

  @Override
  public String onPermissionRequest(JsonNode params) {
    synchronized (permissionLock) {
      // The model sometimes sends the very same change twice (parallel tool
      // calls or a retry): give the answer the user already gave.
      JsonNode toolCall = params.path("toolCall");
      String key = toolCall.path("kind").asText() + "\u0000" + toolCall.path("content") + "\u0000"
                   + toolCall.path("rawInput").path("command").asText();
      String previous = answeredThisTurn.get(key);
      if (previous != null) {
        return previous;
      }
      String answer = askPermission(params);
      if (answer != null) {
        answeredThisTurn.put(key, answer);
      }
      return answer;
    }
  }

  private String askPermission(JsonNode params) {
    AtomicReference<String> answer = new AtomicReference<>();
    try {
      SwingUtilities.invokeAndWait(() -> {
        statusLabel.setText(tr("Waiting for your confirmation..."));
        if (!editor.isActive()) {
          editor.toFront();
        }
        String a = AiPermissionDialog.ask(editor, params, editor.getSketch().getFolder());
        answer.set(a);
        String what = AiPermissionDialog.summary(params);
        boolean allowed = a != null && a.startsWith("allow");
        appendTool("  " + (allowed ? "✓ " + tr("Allowed: ") : "✗ " + tr("Denied: ")) + what);
        statusLabel.setText(tr("Thinking..."));
      });
    } catch (Exception e) {
      return null;
    }
    return answer.get();
  }

  @Override
  public void onExit(int exitCode) {
    onEdt(() -> {
      // client is null after an intentional shutdown, and still alive when an
      // older process (replaced by a restart) is the one that ended.
      if (client == null || client.isAlive()) {
        return;
      }
      client = null;
      sessionId = null;
      contextSent = false;
      appendError(I18n.format(tr("The AI process ended (code {0}). It starts again with the next message."), exitCode));
    });
  }

  // ------------------------------------------------------------------ sketch reload

  /**
   * Shows on-disk changes in the editor: new or removed files become tabs,
   * changed files are reloaded. A tab with unsaved typing is only replaced
   * when the user agrees.
   */
  private void reloadSketchFromDisk() {
    Sketch sketch = editor.getSketch();
    if (sketch == null || editor.getTabs().isEmpty()) {
      return;
    }
    try {
      File current = editor.getCurrentTab().getSketchFile().getFile();
      if (sketch.filesChangedOnDisk()) {
        boolean unsaved = false;
        for (EditorTab tab : editor.getTabs()) {
          unsaved |= tab.isModified();
        }
        if (unsaved && !confirmDiscard()) {
          return;
        }
        sketch.reload();
        editor.createTabs();
        int index = editor.findTabIndex(current);
        if (index > 0) {
          editor.selectTab(index);
        }
        return;
      }
      for (EditorTab tab : editor.getTabs()) {
        SketchFile f = tab.getSketchFile();
        if (f.isModifiedOnDisk()) {
          if (tab.isModified() && !confirmDiscard()) {
            continue;
          }
          tab.reload();
        }
      }
    } catch (IOException e) {
      appendError(tr("Could not reload the sketch: ") + e.getMessage());
    }
  }

  private boolean confirmDiscard() {
    return JOptionPane.showConfirmDialog(editor,
        tr("The AI changed files that you also edited.\nLoad the AI's version? Your unsaved typing will be lost."),
        tr("AI Assistant"), JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE) == JOptionPane.YES_OPTION;
  }

  // ------------------------------------------------------------------ transcript

  private static String describeTool(String kind, String title, List<File> files) {
    String name = files.isEmpty() ? "" : files.get(0).getName();
    switch (kind) {
      case "read":
        return name.isEmpty() ? title : tr("Read ") + name;
      case "edit":
        return name.isEmpty() ? title : tr("Edit ") + name;
      case "delete":
        return name.isEmpty() ? title : tr("Delete ") + name;
      case "search":
        return tr("Search: ") + stripToolName(title);
      case "execute":
        return tr("Run: ") + stripToolName(title);
      default:
        return title;
    }
  }

  /** "terminal: ls -la" to "ls -la". */
  private static String stripToolName(String title) {
    int i = title.indexOf(": ");
    return i > 0 && i < 30 ? title.substring(i + 2) : title;
  }

  private static String contentText(JsonNode update) {
    StringBuilder sb = new StringBuilder();
    for (JsonNode c : update.path("content")) {
      if (c.path("content").has("text")) {
        sb.append(c.path("content").path("text").asText());
      }
    }
    return sb.toString();
  }

  private static final java.util.regex.Pattern WINDOWS_PATH =
      java.util.regex.Pattern.compile("[A-Za-z]:[\\\\/](?:[^\\\\/\\s`'\"]+[\\\\/])*([^\\\\/\\s`'\":]+)");

  /** "C:\Users\...\Blink\Blink.ino" to "Blink.ino", so messages stay readable. */
  static String shortenPaths(String s) {
    return WINDOWS_PATH.matcher(s).replaceAll("$1");
  }

  private static String firstLine(String s) {
    String t = s.trim();
    int nl = t.indexOf('\n');
    if (nl >= 0) {
      t = t.substring(0, nl);
    }
    return t.length() > 160 ? t.substring(0, 160) + "..." : t;
  }

  /** True when the transcript is empty or ends with a line break. */
  private boolean atLineStart() {
    StyledDocument doc = transcript.getStyledDocument();
    try {
      return doc.getLength() == 0 || "\n".equals(doc.getText(doc.getLength() - 1, 1));
    } catch (BadLocationException e) {
      return true;
    }
  }

  private void appendUser(String text) {
    append(atLineStart() ? "\n" : "\n\n", userStyle);
    append(tr("You") + "\n", userLabelStyle);
    append(text + "\n", userStyle);
    lastWasAssistantText = false;
    assistantLabelShown = false;
    record(ChatHistory.USER, text);
  }

  private void appendAssistant(String text) {
    boolean newBlock = !lastWasAssistantText;
    if (newBlock) {
      // a new block of AI text: drop the blank lines the model starts with
      text = text.replaceFirst("^\\s+", "");
      if (text.isEmpty()) {
        return;
      }
      if (!assistantLabelShown) {
        append("\n" + tr("AI") + "\n", aiLabelStyle);
        assistantLabelShown = true;
      } else {
        append("\n", aiStyle);
      }
    }
    append(text, aiStyle);
    lastWasAssistantText = true;
    if (newBlock) {
      record(ChatHistory.AI, text);
    } else if (recording && !history.entries.isEmpty()) {
      // the AI's text arrives in pieces: one entry per block
      history.entries.get(history.entries.size() - 1).text += text;
      historyDirty = true;
    }
  }

  private void appendTool(String text) {
    if (!atLineStart()) {
      append("\n", toolStyle);
    }
    append(text + "\n", toolStyle);
    lastWasAssistantText = false;
    record(ChatHistory.TOOL, text);
  }

  private void appendInfo(String text) {
    append((atLineStart() ? "" : "\n") + text + "\n", infoStyle);
    lastWasAssistantText = false;
    record(ChatHistory.INFO, text);
  }

  private void appendError(String text) {
    append((atLineStart() ? "" : "\n") + text + "\n", errorStyle);
    lastWasAssistantText = false;
    record(ChatHistory.ERROR, text);
  }

  private void record(String type, String text) {
    if (recording) {
      history.entries.add(new ChatHistory.Entry(type, text));
      historyDirty = true;
    }
  }

  /** Draws the conversation kept in {@link #history} (after opening, or after old messages were dropped). */
  private void showHistory(String headline) {
    recording = false;
    try {
      transcript.setText("");
      lastWasAssistantText = false;
      assistantLabelShown = false;
      if (headline != null) {
        appendInfo(headline);
      }
      if (history.omitted > 0) {
        appendInfo(I18n.format(tr("{0} earlier messages are not shown."), history.omitted));
      }
      for (ChatHistory.Entry e : history.entries) {
        switch (e.type) {
          case ChatHistory.USER:
            appendUser(e.text);
            break;
          case ChatHistory.AI:
            lastWasAssistantText = false; // each entry is a block of its own
            appendAssistant(e.text);
            break;
          case ChatHistory.TOOL:
            appendTool(e.text);
            break;
          case ChatHistory.ERROR:
            appendError(e.text);
            break;
          default:
            appendInfo(e.text);
        }
      }
      lastWasAssistantText = false;
    } finally {
      recording = true;
    }
  }

  private void append(String text, SimpleAttributeSet style) {
    StyledDocument doc = transcript.getStyledDocument();
    try {
      doc.insertString(doc.getLength(), text, style);
    } catch (BadLocationException e) {
      // cannot happen when appending
    }
    transcript.setCaretPosition(doc.getLength());
  }

  private static void onEdt(Runnable r) {
    if (SwingUtilities.isEventDispatchThread()) {
      r.run();
    } else {
      SwingUtilities.invokeLater(r);
    }
  }
}
