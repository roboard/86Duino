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
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.event.HierarchyEvent;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.JTextPane;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;

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

  /** The look and feel's font of the conversation: the default size. */
  /** The conversation's font: ai.font of the theme, or the look and feel's text font. */
  private final Font defaultFont = AiTheme.chatFont(new JTextPane().getFont());
  private final ChatView chatView = new ChatView(defaultFont);
  private final JTextArea input = new JTextArea(3, 20) {
    @Override
    protected void paintComponent(Graphics g) {
      super.paintComponent(g);
      if (getDocument().getLength() == 0) {
        // the hint, until something is typed
        java.awt.Insets in = getInsets();
        g.setColor(AiTheme.mix(getBackground(), getForeground(), 0.7));
        g.setFont(getFont());
        g.drawString(tr("Ask a question..."), in.left, in.top + g.getFontMetrics().getAscent());
      }
    }
  };
  /** "Send", and "Stop" while the AI answers. */
  private final JButton sendButton = AiWidgets.button(tr("Send"), AiTheme.sendBg(), AiTheme.sendFg(),
                                                      AiTheme.mix(AiTheme.sendBg(), AiTheme.sendFg(), 0.3));
  /** The ▶ and ■ of Send and Stop, as tall as the button's text (made with the button). */
  private AiWidgets.ShapeIcon playIcon;
  private AiWidgets.ShapeIcon stopIcon;
  private final JButton newButton = AiWidgets.button(tr("New Chat"), AiTheme.chatBg(), AiTheme.chatFg(),
                                                     AiTheme.mix(AiTheme.chatBg(), AiTheme.chatFg(), 0.25));
  private final JLabel statusLabel = new JLabel(" ", SwingConstants.CENTER);
  /** Sends the error output of the console (e.g. compiler errors) with the next message. */
  private final JCheckBox attachErrors = new JCheckBox(tr("Attach the compile error messages"));
  /** Compile errors sent with a message, at most this long (the first part is kept). */
  private static final int MAX_ERROR_CHARS = 30000;
  /**
   * The model in use, next to Send. As wide as the longest model name, up to a
   * limit; empty when the model list could not be fetched.
   */
  private final JComboBox<String> modelCombo = new JComboBox<String>() {
    @Override
    public Dimension getPreferredSize() {
      Dimension d = super.getPreferredSize();
      d.width = Math.max(Theme.scale(MODEL_MIN_WIDTH), Math.min(Theme.scale(MODEL_MAX_WIDTH), d.width));
      d.height = sendButton.getPreferredSize().height; // as tall as Send
      return d;
    }

    @Override
    public Dimension getMaximumSize() {
      return getPreferredSize();
    }
  };
  private static final int MODEL_MIN_WIDTH = 100;
  private static final int MODEL_MAX_WIDTH = 280;
  /** True while the panel fills the model box (not a choice of the user). */
  private boolean fillingModels;
  /** The model list the model box shows, see {@link #modelLists}. */
  private CompletableFuture<List<String>> shownModels;
  /** "Could not find any models on the network." in the conversation, taken away when the list is fetched again. */
  private Component modelNotice;
  /** "Retry", shown when the model list could not be fetched. */
  private final JButton retryButton = new JButton(tr("Retry"));
  /** A text area, not an HTML label: HTML labels do not wrap Chinese text. */
  private final JTextArea setupText = new JTextArea();
  private final JPanel cards = new JPanel(new CardLayout());

  // Font size of the conversation and the message box (Ctrl + mouse wheel),
  // shared by all windows. The model box and the buttons keep theirs.
  private static final int FONT_MIN = 8;
  private static final int FONT_MAX = 40;
  /** Open AI panels, so a font size change reaches every window. */
  private static final java.util.Set<AiChatPanel> PANELS =
      java.util.Collections.newSetFromMap(new java.util.WeakHashMap<>());
  /** Wheel turns not used yet: a touchpad sends many small ones. */
  private double pendingWheel;

  // The conversation above, the message box below, the border between them can be dragged.
  private JSplitPane chat;
  private JScrollPane transcriptScroll;
  private JPanel inputPanel;
  /** The message box's smallest height (three lines and the row below); it follows the font. */
  private int inputMin;
  /** The message box height the window had last time, used once. */
  private int savedInput;
  private boolean dividerPlaced;

  // Conversation state. Changed on the event thread only, except where noted.
  private AcpClient client;
  private String sessionId;
  private File sessionFolder;
  private String sessionSettings;
  private String sessionBoard;
  /** The model the session uses (it may differ from the one chosen since), or null. */
  private String sessionModel;
  private boolean contextSent;
  private volatile boolean busy;
  /** Stop was pressed during this turn: the turn thread stops at the next step. */
  private volatile boolean stopRequested;
  /** Set by the turn thread once the message is sent, for Stop. */
  private volatile AcpClient turnClient;
  private volatile String turnSession;
  private boolean lastWasAssistantText;
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

    // Header in the toolbar's color: title and buttons, the status line centered below
    Color headerBg = AiTheme.headerBg();
    Color headerFg = AiTheme.headerFg();
    JPanel header = new JPanel(new BorderLayout(0, Theme.scale(2)));
    header.setBackground(headerBg);
    header.setBorder(BorderFactory.createEmptyBorder(Theme.scale(6), Theme.scale(10), Theme.scale(4), Theme.scale(6)));
    JPanel headerTop = new JPanel(new BorderLayout(Theme.scale(6), 0));
    headerTop.setOpaque(false);
    JLabel title = new JLabel(tr("AI Assistant"));
    Font titleFont = AiTheme.inPanelFont(title.getFont());
    title.setFont(titleFont.deriveFont(Font.BOLD, titleFont.getSize2D() + 3));
    title.setForeground(headerFg);
    // New Chat and the X as big as the title
    newButton.setFont(title.getFont().deriveFont(Font.PLAIN));
    // little room around the text, so the buttons are not much bigger than it
    newButton.setBorder(BorderFactory.createEmptyBorder(Theme.scale(2), Theme.scale(6), Theme.scale(2), Theme.scale(6)));
    headerTop.add(title, BorderLayout.WEST);
    JPanel headerButtons = new JPanel();
    headerButtons.setLayout(new BoxLayout(headerButtons, BoxLayout.X_AXIS));
    headerButtons.setOpaque(false);
    newButton.setToolTipText(tr("Start a new conversation"));
    newButton.addActionListener(e -> newConversation());
    headerButtons.add(newButton);
    headerButtons.add(Box.createHorizontalStrut(Theme.scale(6)));
    JButton hide = AiWidgets.button("", AiTheme.chatBg(), AiTheme.chatFg(),
                                    AiTheme.mix(AiTheme.chatBg(), AiTheme.chatFg(), 0.25));
    hide.setToolTipText(tr("Hide the AI Assistant"));
    hide.addActionListener(e -> editor.setAiPanelVisible(false));
    // a square as tall as New Chat, with an X about as big as a capital letter
    // of the title in its middle: no border, and as much room left as right
    int side = newButton.getPreferredSize().height;
    int cross = Math.round(title.getFont().getSize2D() * 0.6f);
    if ((side - cross) % 2 != 0) {
      cross++;
    }
    hide.setIcon(new AiWidgets.ShapeIcon(AiWidgets.Shape.CLOSE, cross));
    hide.setBorder(BorderFactory.createEmptyBorder());
    hide.setPreferredSize(new Dimension(side, side));
    hide.setMinimumSize(hide.getPreferredSize());
    hide.setMaximumSize(hide.getPreferredSize());
    newButton.setMaximumSize(newButton.getPreferredSize());
    headerButtons.add(hide);
    headerTop.add(headerButtons, BorderLayout.EAST);
    header.add(headerTop, BorderLayout.NORTH);
    statusLabel.setForeground(headerFg);
    statusLabel.setFont(AiTheme.inPanelFont(statusLabel.getFont()));
    header.add(statusLabel, BorderLayout.CENTER);
    add(header, BorderLayout.NORTH);

    // Chat card
    transcriptScroll = new JScrollPane(chatView, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                       JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    transcriptScroll.setBorder(null);
    transcriptScroll.getViewport().setBackground(AiTheme.chatBg());
    chatView.addMouseWheelListener(this::mouseWheel);

    Color inputBg = AiTheme.inputBg();
    Color inputFg = AiTheme.inputFg();
    input.setBackground(inputBg);
    input.setForeground(inputFg);
    input.setCaretColor(inputFg);
    input.setSelectionColor(AiTheme.mix(inputBg, AiTheme.sendBg(), 0.7));
    input.setSelectedTextColor(AiTheme.readableOn(input.getSelectionColor()));
    input.setBorder(BorderFactory.createEmptyBorder(Theme.scale(6), Theme.scale(8), Theme.scale(4), Theme.scale(8)));
    input.setLineWrap(true);
    input.setWrapStyleWord(true);
    input.setToolTipText(tr("Enter: send, Shift+Enter: new line"));
    input.addKeyListener(new KeyAdapter() {
      @Override
      public void keyPressed(KeyEvent e) {
        if (e.getKeyCode() != KeyEvent.VK_ENTER) {
          return;
        }
        e.consume();
        if (e.isShiftDown() || e.isControlDown()) {
          // a text area has no binding for Shift+Enter: the line break is added here
          input.replaceSelection("\n");
        } else {
          send();
        }
      }
    });
    input.addMouseWheelListener(this::mouseWheel);
    sendButton.addActionListener(e -> {
      if (busy) {
        stop();
      } else {
        send();
      }
    });
    // The model row is used all the time: Send, the model box and its label are
    // as big as New Chat and the X at the top, so all the buttons look alike.
    Font rowFont = newButton.getFont();
    // "Send ▶" and "Stop ■" take turns on the button: as wide as the wider one, so nothing moves
    sendButton.setFont(rowFont.deriveFont(Font.BOLD));
    int iconSize = textHeight(sendButton.getFont(), tr("Send"));
    playIcon = new AiWidgets.ShapeIcon(AiWidgets.Shape.PLAY, iconSize);
    stopIcon = new AiWidgets.ShapeIcon(AiWidgets.Shape.STOP, iconSize);
    sendButton.setIcon(playIcon);
    // little room around the text and the icon
    sendButton.setBorder(BorderFactory.createEmptyBorder(Theme.scale(2), Theme.scale(8), Theme.scale(2), Theme.scale(8)));
    sendButton.setHorizontalTextPosition(SwingConstants.LEADING);
    sendButton.setIconTextGap(Theme.scale(6));
    Dimension sendSize = widestSize(sendButton, tr("Send"), tr("Stop"));
    sendSize.height = newButton.getPreferredSize().height; // exactly as tall as New Chat
    sendButton.setPreferredSize(sendSize);
    sendButton.setMinimumSize(sendSize);
    sendButton.setMaximumSize(sendSize);
    AiWidgets.styleCombo(modelCombo, AiTheme.modelBg(), AiTheme.modelFg(),
                         AiTheme.mix(AiTheme.modelBg(), AiTheme.modelFg(), 0.3));
    modelCombo.setFont(rowFont);
    modelCombo.setMinimumSize(Theme.scale(new Dimension(60, 0)));
    modelCombo.addActionListener(e -> modelChosen());
    modelCombo.setEnabled(false);
    JLabel modelLabel = new JLabel(tr("Model:"));
    modelLabel.setForeground(AiTheme.modelLabelFg());
    modelLabel.setFont(rowFont);
    modelLabel.setLabelFor(modelCombo);

    // The message box and the row below it (model, Send) are one framed box:
    // a long message scrolls inside it and never covers the row.
    JScrollPane inputScroll = new JScrollPane(input, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                                              JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    inputScroll.setBorder(null);
    inputScroll.getViewport().setBackground(inputBg);
    JPanel inputRow = new JPanel();
    inputRow.setLayout(new BoxLayout(inputRow, BoxLayout.X_AXIS));
    inputRow.setOpaque(false);
    inputRow.setBorder(BorderFactory.createEmptyBorder(Theme.scale(4), Theme.scale(6), Theme.scale(6), Theme.scale(6)));
    inputRow.add(Box.createHorizontalGlue());
    inputRow.add(modelLabel);
    inputRow.add(Box.createHorizontalStrut(Theme.scale(4)));
    inputRow.add(modelCombo);
    inputRow.add(Box.createHorizontalStrut(Theme.scale(8)));
    inputRow.add(sendButton);
    JPanel inputBox = new JPanel(new BorderLayout());
    inputBox.setBackground(inputBg);
    inputBox.setBorder(BorderFactory.createLineBorder(AiTheme.mix(inputBg, inputFg, 0.35)));
    inputBox.add(inputScroll, BorderLayout.CENTER);
    inputBox.add(inputRow, BorderLayout.SOUTH);

    // "Attach the compile error messages" in a light box on a row of its own, above the message box
    attachErrors.setToolTipText(tr("Sends the error messages of the console below the editor (stderr) with your message"));
    attachErrors.setBackground(AiTheme.chatBg());
    attachErrors.setForeground(AiTheme.chatFg());
    attachErrors.setOpaque(true);
    attachErrors.setFocusPainted(false);
    attachErrors.setFont(AiTheme.inPanelFont(attachErrors.getFont()));
    // the look and feel's square is much taller than the text: draw one as tall as the text
    attachErrors.setIcon(new AiWidgets.CheckIcon(attachErrors));
    attachErrors.setRolloverEnabled(true);
    attachErrors.setBorderPainted(true);
    attachErrors.setBorder(BorderFactory.createCompoundBorder(
        BorderFactory.createLineBorder(AiTheme.mix(AiTheme.attachBg(), AiTheme.chatFg(), 0.3)),
        BorderFactory.createEmptyBorder(Theme.scale(3), Theme.scale(6), Theme.scale(3), Theme.scale(10))));
    JPanel attachRow = new JPanel(new FlowLayout(FlowLayout.LEFT, 0, 0));
    attachRow.setBackground(AiTheme.attachBg());
    // the same margin as around the message box below, so the two line up
    attachRow.setBorder(BorderFactory.createEmptyBorder(Theme.scale(8), Theme.scale(8), Theme.scale(8), Theme.scale(8)));
    attachRow.add(attachErrors);

    JPanel inputArea = new JPanel(new BorderLayout());
    inputArea.setBackground(inputBg);
    // as far from the top and the bottom as from the sides
    inputArea.setBorder(BorderFactory.createEmptyBorder(Theme.scale(8), Theme.scale(8), Theme.scale(8), Theme.scale(8)));
    inputArea.add(inputBox, BorderLayout.CENTER);
    inputPanel = new JPanel(new BorderLayout());
    inputPanel.add(attachRow, BorderLayout.NORTH);
    inputPanel.add(inputArea, BorderLayout.CENTER);

    applyFontSize(); // the size chosen last time, before the message box is measured

    // The border between the conversation and the message box can be dragged:
    // the message box keeps at least its height with the row below, and takes at most
    // three quarters of the height.
    inputMin = inputPanel.getPreferredSize().height;
    inputPanel.setMinimumSize(new Dimension(0, inputMin));
    chat = new JSplitPane(JSplitPane.VERTICAL_SPLIT, transcriptScroll, inputPanel);
    chat.setContinuousLayout(true);
    chat.setResizeWeight(1D); // a taller window gives the conversation the room
    chat.setBorder(null);
    chat.setDividerSize(Theme.scale(6));
    // the divider shows the split pane's color: the attach row's, so it looks part of that row
    chat.setBackground(AiTheme.attachBg());
    // the divider draws no lines of its own (the conversation's bottom line is
    // the only separator); it still shows the resize cursor and can be dragged
    if (chat.getUI() instanceof javax.swing.plaf.basic.BasicSplitPaneUI) {
      ((javax.swing.plaf.basic.BasicSplitPaneUI) chat.getUI()).getDivider().setBorder(null);
    }
    Keys.killBinding(chat, Keys.ctrl(KeyEvent.VK_TAB));
    Keys.killBinding(chat, Keys.ctrlShift(KeyEvent.VK_TAB));
    // read now: the split pane's own first layout moves the divider before it is placed
    savedInput = PreferencesData.getInteger(AiSettings.PREF_INPUT_HEIGHT, inputMin);
    chat.addComponentListener(new java.awt.event.ComponentAdapter() {
      @Override
      public void componentResized(java.awt.event.ComponentEvent e) {
        placeDivider();
      }
    });
    chat.addPropertyChangeListener(JSplitPane.DIVIDER_LOCATION_PROPERTY, e -> {
      int input = chat.getHeight() - chat.getDividerLocation() - chat.getDividerSize();
      if (dividerPlaced && input >= inputMin) {
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

    // the model list is fetched when the panel is opened (again, after it failed)
    addHierarchyListener(e -> {
      if ((e.getChangeFlags() & HierarchyEvent.SHOWING_CHANGED) != 0 && isShowing()) {
        AiSettings settings = AiSettings.load();
        HermesBundle bundle = HermesBundle.getDefault();
        if (setupProblem(settings, bundle) == null) {
          showModelList(settings, bundle, true);
        }
      }
    });
    PANELS.add(this);
    refreshSetupState();
  }

  /**
   * Keeps the message box between its smallest height and three quarters of
   * the height; the first time, at the height it had last time.
   */
  private void placeDivider() {
    int height = chat.getHeight();
    if (height <= 0) {
      return;
    }
    int maxInput = Math.max(inputMin, height * 3 / 4);
    transcriptScroll.setMinimumSize(new Dimension(0, height - maxInput - chat.getDividerSize()));
    int input = dividerPlaced ? height - chat.getDividerLocation() - chat.getDividerSize() : savedInput;
    input = Math.max(inputMin, Math.min(maxInput, input));
    dividerPlaced = true;
    chat.setDividerLocation(height - input - chat.getDividerSize());
  }

  // ------------------------------------------------------------------ font size

  /** The conversation's font size: the one chosen with Ctrl + mouse wheel, or the default. */
  private int fontSize() {
    int size = PreferencesData.getInteger(AiSettings.PREF_FONT_SIZE, defaultFont.getSize());
    return Math.max(FONT_MIN, Math.min(FONT_MAX, size));
  }

  /**
   * Ctrl + mouse wheel makes the conversation and the message box bigger
   * (wheel up) or smaller, in every window; other turns scroll as usual.
   */
  private void mouseWheel(java.awt.event.MouseWheelEvent e) {
    if (!e.isControlDown()) {
      // a listener here keeps the scroll pane from getting the turn: pass it on
      java.awt.Component parent = e.getComponent().getParent();
      if (parent != null) {
        parent.dispatchEvent(e);
      }
      return;
    }
    e.consume();
    double turn = e.getPreciseWheelRotation();
    if (turn == 0) {
      return;
    }
    if (pendingWheel != 0 && Math.signum(turn) != Math.signum(pendingWheel)) {
      pendingWheel = 0; // the other direction starts afresh
    }
    pendingWheel += turn;
    // a whole notch is one point; a touchpad's small turns add up to one
    int steps = (int) pendingWheel;
    if (steps == 0) {
      return;
    }
    pendingWheel -= steps;
    int size = Math.max(FONT_MIN, Math.min(FONT_MAX, fontSize() - steps));
    if (size != fontSize()) {
      PreferencesData.setInteger(AiSettings.PREF_FONT_SIZE, size);
      for (AiChatPanel panel : new ArrayList<>(PANELS)) {
        panel.applyFontSize();
      }
    }
  }

  /** Shows the conversation and the message box in the chosen font size. */
  private void applyFontSize() {
    int size = fontSize();
    Font font = defaultFont.deriveFont((float) size);
    // the conversation's labels and tool lines are one point smaller
    chatView.setFonts(font);
    input.setFont(font);
    if (chat == null) {
      return; // being made: the message box is measured next
    }
    // the message box keeps room for three lines in the new size
    inputMin = inputPanel.getPreferredSize().height;
    inputPanel.setMinimumSize(new Dimension(0, inputMin));
    placeDivider();
    revalidate();
  }

  /** How tall the letters of the text really are in this font (not the line height). */
  private static int textHeight(Font font, String text) {
    java.awt.font.FontRenderContext frc = new java.awt.font.FontRenderContext(null, true, true);
    double h = font.createGlyphVector(frc, text).getVisualBounds().getHeight();
    return Math.max(Theme.scale(6), (int) Math.round(h));
  }

  /** The size of the button with the longest of these texts. */
  private static Dimension widestSize(JButton button, String... texts) {
    String text = button.getText();
    Dimension size = new Dimension();
    for (String t : texts) {
      button.setText(t);
      Dimension d = button.getPreferredSize();
      size.width = Math.max(size.width, d.width);
      size.height = Math.max(size.height, d.height);
    }
    button.setText(text);
    return size;
  }

  /**
   * Draws the bar between the editor and this panel in the theme's color (a
   * pale blue) instead of the look and feel's gray. Call it right after the
   * split pane is made, before its other settings.
   */
  public static void styleDivider(JSplitPane split) {
    AiWidgets.styleDivider(split, AiTheme.dividerBg());
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
      if (isShowing()) {
        showModelList(settings, bundle, false);
      }
      ((CardLayout) cards.getLayout()).show(cards, CARD_CHAT);
      newButton.setEnabled(true);
    } else {
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

  // ------------------------------------------------------------------ model box

  /**
   * The models each key can use, by {@link #pickAttempt} (provider, key, Base
   * URL), shared by all editor windows. A failed list is fetched again when a
   * panel is opened.
   */
  private static final Map<String, CompletableFuture<List<String>>> modelLists = new ConcurrentHashMap<>();

  /** Fills the model box from the list for these settings, fetching it first when needed. */
  private void showModelList(AiSettings settings, HermesBundle bundle, boolean retryFailed) {
    String key = pickAttempt(settings, bundle);
    CompletableFuture<List<String>> list = modelLists.get(key);
    if (list != null && retryFailed && list.isCompletedExceptionally()) {
      modelLists.remove(key, list);
    }
    list = modelLists.computeIfAbsent(key, k -> fetchModels(settings, bundle));
    if (list == shownModels) {
      if (list.isDone() && !list.isCompletedExceptionally()) {
        fillModels(list.join()); // the chosen model may have changed
      }
      return;
    }
    shownModels = list;
    // a new list: the old "no models" line no longer holds (it comes back if this one fails too)
    chatView.removeNote(modelNotice);
    modelNotice = null;
    // until the list arrives: the model in use
    fillModels(java.util.Collections.singletonList(settings.model.trim()));
    CompletableFuture<List<String>> shown = list;
    list.whenComplete((models, error) -> SwingUtilities.invokeLater(() -> {
      if (shownModels != shown) {
        return; // the settings changed meanwhile
      }
      if (error != null) {
        fillModels(java.util.Collections.emptyList());
        chatView.removeNote(modelNotice);
        modelNotice = appendNotice(tr("Could not find any models on the network."));
      } else {
        fillModels(models);
      }
    }));
  }

  /**
   * OK in Preferences: every model list is fetched again the next time a panel
   * shows it, a good one too (the same key may work again: renewed, topped up,
   * the network back), and a failed automatic model choice is tried again.
   * Call it once the new settings are written, so the lists use the new key.
   */
  static void forgetModelLists() {
    modelLists.clear();
    pickFailedFor = null;
    pickFailedError = "";
  }

  private static CompletableFuture<List<String>> fetchModels(AiSettings settings, HermesBundle bundle) {
    AiSettings.Provider p = settings.getProvider();
    String baseUrl = settings.baseUrl;
    return CompletableFuture.supplyAsync(() -> {
      HermesBundle.ModelList list;
      try {
        list = bundle.listModels(p.id, bundle.readEnvValue(p.apiKeyEnv), baseUrl);
      } catch (IOException | InterruptedException e) {
        throw new CompletionException(e);
      }
      if (list.models.isEmpty()) {
        throw new IllegalStateException(list.error == null ? "no models" : list.error);
      }
      return list.models;
    });
  }

  /** Shows these models (and the chosen one, when the list misses it) in the model box. */
  private void fillModels(List<String> models) {
    AiSettings settings = AiSettings.load();
    String current = settings.model.trim();
    List<String> items = new ArrayList<>(models);
    if (!items.isEmpty() && !current.isEmpty() && !items.contains(current)) {
      items.add(0, current);
    }
    fillingModels = true;
    try {
      modelCombo.setModel(new DefaultComboBoxModel<>(items.toArray(new String[0])));
      modelCombo.setSelectedItem(items.contains(current) ? current : null);
    } finally {
      fillingModels = false;
    }
    modelCombo.setToolTipText(items.contains(current) ? settings.getProvider().label + " / " + current : null);
    modelCombo.setEnabled(!busy && !items.isEmpty());
    modelCombo.revalidate(); // its width follows the longest name
  }

  /**
   * A model was chosen: saved for every window. The conversation switches to
   * it with the next message and keeps what was said (session/set_model).
   */
  private void modelChosen() {
    Object item = modelCombo.getSelectedItem();
    if (fillingModels || item == null) {
      return;
    }
    String model = item.toString().trim();
    if (!model.isEmpty() && !model.equals(AiSettings.load().model.trim())) {
      AiPreferencesPanel.saveModel(model);
    }
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
    if (saved.sessionId != null && saved.sameAi(AiSettings.load())) {
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
    return s.processSignature() + "|" + bundle.apiKeyFingerprint(s.getProvider().apiKeyEnv);
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

  /** While the AI answers, Send becomes Stop; it is Send again once the turn has ended. */
  private void setBusy(boolean b, String status) {
    busy = b;
    stopRequested = false;
    sendButton.setText(b ? tr("Stop") : tr("Send"));
    sendButton.setIcon(b ? stopIcon : playIcon);
    // Hermes switches models only between turns
    modelCombo.setEnabled(!b && modelCombo.getItemCount() > 0);
    statusLabel.setText(status == null ? " " : status);
  }

  private void send() {
    String text = input.getText().trim();
    // Ticked: the console's errors (stderr) go along when there are any. Errors
    // or a message is enough to send; with neither, Send does nothing.
    String errors = attachErrors.isSelected() ? editor.getConsoleErrorText().trim() : "";
    if ((text.isEmpty() && errors.isEmpty()) || busy) {
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
    if (resume != null && !history.sameAi(settings)) {
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
    String attached = null;
    if (!errors.isEmpty()) {
      attached = attachErrorBlock(errors, blocks);
    }
    if (!text.isEmpty()) {
      blocks.add(text);
    }

    input.setText(""); // the box stays ticked: every message takes the console's errors along
    appendUser(text);
    if (attached != null) {
      appendAttachment(attached);
    }
    editToolCalls.clear();
    toolCallTitles.clear();
    answeredThisTurn.clear();
    setBusy(true, restart ? tr("Starting the AI...") : tr("Thinking..."));

    AcpClient currentClient = restart ? null : client;
    String currentSession = newSession ? null : sessionId;
    String currentModel = newSession ? null : sessionModel;
    if (restart) {
      shutdownClient();
    }
    String resumeId = resume;
    Thread worker = new Thread(() -> runTurn(bundle, settings, folder, currentClient, currentSession, currentModel,
                                             resumeId, board, blocks), "ai-turn");
    worker.setDaemon(true);
    worker.start();
  }

  /**
   * Adds the console's error output to the message; returns the line shown
   * under your message. Very long output is cut after the first lines (the
   * first errors are the ones that matter).
   */
  private static String attachErrorBlock(String errors, List<String> blocks) {
    String[] lines = errors.split("\r?\n", -1);
    String sent = errors;
    int sentLines = lines.length;
    if (errors.length() > MAX_ERROR_CHARS) {
      int cut = errors.lastIndexOf('\n', MAX_ERROR_CHARS);
      sent = errors.substring(0, cut > 0 ? cut : MAX_ERROR_CHARS);
      sentLines = sent.split("\r?\n", -1).length;
    }
    StringBuilder block = new StringBuilder();
    block.append("[Compile error messages from the IDE console (stderr), attached by the user]\n");
    block.append(sent).append('\n');
    if (sentLines < lines.length) {
      block.append("[... ").append(lines.length - sentLines).append(" more lines not sent]\n");
    }
    block.append("[End of the compile error messages]");
    blocks.add(block.toString());
    return sentLines < lines.length
        ? I18n.format(tr("Attached the compile error messages (the first {0} of {1} lines; the rest was too long)"),
                      sentLines, lines.length)
        : I18n.format(tr("Attached the compile error messages ({0} lines)"), lines.length);
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

  /** Stop was pressed before the message was sent. */
  private static class TurnStopped extends Exception {
  }

  private void checkStop() throws TurnStopped {
    if (stopRequested) {
      throw new TurnStopped();
    }
  }

  /**
   * Runs on a background thread. A null client means start Hermes first; a
   * null session id means create a new conversation first. sessionModel is
   * the model the session uses, or null when not known.
   */
  private void runTurn(HermesBundle bundle, AiSettings settings, File folder, AcpClient c,
                       String sid, String sessionModel, String resumeId, String board, List<String> blocks) {
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
      checkStop();
      if (sid == null) {
        // the conversation saved in the sketch first: Hermes keeps it in the AI workspace
        JsonNode loaded = resumeId != null ? resumeSession(c, resumeId, folder) : null;
        if (loaded != null) {
          sid = resumeId;
          sessionModel = currentModel(loaded);
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
          sessionModel = currentModel(res);
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
      checkStop();
      // the model chosen since (in any window), or one the session was not made with
      String model = settings.model.trim();
      if (!sameModel(sessionModel, model)) {
        onEdt(() -> statusLabel.setText(tr("Switching the model...")));
        // "provider:model" when the name itself has a colon (Ollama's "qwen:7b" would read as a provider)
        String modelId = model.contains(":") ? settings.getProvider().id + ":" + model : model;
        try {
          c.setModel(sid, modelId).get(120, TimeUnit.SECONDS);
        } catch (ExecutionException e) {
          Throwable why = e.getCause() != null ? e.getCause() : e;
          String msg = why.getMessage() == null ? why.toString() : why.getMessage();
          onEdt(() -> {
            finishTurn();
            appendError(I18n.format(tr("Could not switch to the model {0}: {1}"), model, msg)
                        + "\n" + tr("Choose another model below."));
          });
          return;
        }
        // a session that was just made silently gets the chosen model
        boolean announce = sessionModel != null;
        onEdt(() -> {
          this.sessionModel = model;
          if (announce) {
            appendInfo(I18n.format(tr("Switched to the model {0}."), model));
          }
        });
      } else {
        onEdt(() -> this.sessionModel = model);
      }
      checkStop();
      onEdt(() -> statusLabel.setText(tr("Thinking...")));
      turnClient = c;
      turnSession = sid;
      CompletableFuture<JsonNode> answer = c.prompt(sid, blocks);
      if (stopRequested) {
        c.cancel(sid); // Stop was pressed just before the message went out
      }
      JsonNode result = answer.get();
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
    } catch (TurnStopped e) {
      onEdt(() -> {
        finishTurn();
        appendInfo(tr("Stopped."));
      });
    } catch (ExecutionException e) {
      failTurn(e.getCause() != null ? e.getCause() : e);
    } catch (Exception e) {
      failTurn(e);
    }
  }

  /**
   * Asks Hermes to continue a saved session. Its history replay is not shown:
   * the panel already shows the saved conversation. Null when Hermes does not
   * have the session (another computer or AI workspace) or cannot load it.
   */
  private JsonNode resumeSession(AcpClient c, String id, File folder) {
    replaying = true;
    try {
      JsonNode res = c.loadSession(id, folder).get(120, TimeUnit.SECONDS);
      // an unknown session is answered with null or {} (a loaded one lists its modes and models)
      return res != null && res.isObject() && res.size() > 0 ? res : null;
    } catch (Exception e) {
      System.err.println("Could not continue the AI conversation " + id + ": " + e);
      return null;
    } finally {
      replaying = false;
    }
  }

  /** The model of a session/new or session/load answer ("provider:model"), or null. */
  private static String currentModel(JsonNode res) {
    String id = res.path("models").path("currentModelId").asText("");
    return id.isEmpty() ? null : id;
  }

  /** The session's model (plain, or "provider:model" as Hermes reports it) is this model. */
  private static boolean sameModel(String sessionModel, String model) {
    return sessionModel != null && (sessionModel.equals(model) || sessionModel.endsWith(":" + model));
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
    turnClient = null;
    turnSession = null;
    setBusy(false, null);
    lastWasAssistantText = false;
    reloadSketchFromDisk();
    // after the lines the caller adds ("Stopped.", errors)
    SwingUtilities.invokeLater(this::saveHistory);
  }

  /**
   * Stops the turn. Before the message is sent (Hermes still starting) the
   * turn thread stops at its next step. Pressing Stop again sends another cancel.
   */
  private void stop() {
    if (!busy) {
      return;
    }
    stopRequested = true;
    AcpClient c = turnClient;
    String sid = turnSession;
    if (c != null && sid != null) {
      c.cancel(sid);
    }
    statusLabel.setText(tr("Stopping..."));
  }

  private void newConversation() {
    if (busy) {
      stop();
      return;
    }
    if (!chatView.isEmpty() && !confirmNewConversation()) {
      return;
    }
    sessionId = null;
    contextSent = false;
    chatView.clear();
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
    PANELS.remove(this);
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

  private void appendUser(String text) {
    chatView.addUser(text);
    lastWasAssistantText = false;
    record(ChatHistory.USER, text);
  }

  /** The line under your message saying what was sent with it. */
  private void appendAttachment(String note) {
    chatView.addAttachment(note);
    record(ChatHistory.ATTACH, note);
  }

  private void appendAssistant(String text) {
    boolean newBlock = !lastWasAssistantText;
    if (newBlock) {
      // a new block of AI text: drop the blank lines the model starts with
      text = text.replaceFirst("^\\s+", "");
      if (text.isEmpty()) {
        return;
      }
    }
    chatView.appendAi(text, newBlock);
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
    chatView.addTool(text);
    lastWasAssistantText = false;
    record(ChatHistory.TOOL, text);
  }

  private void appendInfo(String text) {
    chatView.addNote(text, false);
    lastWasAssistantText = false;
    record(ChatHistory.INFO, text);
  }

  /** An info line that is not kept in the saved conversation; returned so it can be taken away. */
  private Component appendNotice(String text) {
    Component note = chatView.addNote(text, false);
    lastWasAssistantText = false;
    return note;
  }

  private void appendError(String text) {
    chatView.addNote(text, true);
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
      chatView.clear();
      lastWasAssistantText = false;
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
          case ChatHistory.ATTACH:
            appendAttachment(e.text);
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

  private static void onEdt(Runnable r) {
    if (SwingUtilities.isEventDispatchThread()) {
      r.run();
    } else {
      SwingUtilities.invokeLater(r);
    }
  }
}
