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
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

import javax.swing.BorderFactory;
import javax.swing.DefaultComboBoxModel;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import processing.app.Base;
import processing.app.I18n;
import processing.app.Theme;

/**
 * The "AI" tab of the Preferences dialog.
 *
 * A new API key is only stored after "Test Connection" succeeded with it and
 * OK was pressed; it replaces the stored key (one key at a time). "Remove API
 * Key" also only takes effect when OK is pressed.
 */
public class AiPreferencesPanel extends JPanel implements javax.swing.Scrollable {

  private static final Color GREEN = new Color(0x10, 0x80, 0x10);
  private static final Color RED = new Color(0xC0, 0x20, 0x20);

  /** Hermes settings being written in the background after OK was pressed. */
  private static volatile CompletableFuture<Void> pendingSave = CompletableFuture.completedFuture(null);

  /** Lets a new AI conversation wait until the latest settings are written. */
  public static void waitForPendingSave() {
    try {
      pendingSave.get(120, TimeUnit.SECONDS);
    } catch (Exception e) {
      // the error was already reported to the user
    }
  }

  private final boolean installed = HermesBundle.getDefault().isInstalled();

  private final JComboBox<AiSettings.Provider> providerCombo = new JComboBox<>(AiSettings.PROVIDERS.toArray(new AiSettings.Provider[0]));
  private final JComboBox<String> modelCombo = new JComboBox<>();
  // Status lines wrap (text areas, not labels): one long line would make the whole page
  // wider than the dialog, and GridBagLayout then shrinks every field to its minimum size.
  private final JTextArea modelStatus = wrappingText();
  private final JLabel baseUrlLabel = new JLabel(tr("Base URL:"));
  private final JTextField baseUrlField = new JTextField(30);
  private final JPasswordField apiKeyField = new JPasswordField(30);
  private final JCheckBox showKeyBox = new JCheckBox(tr("Show"));
  private final JTextArea apiKeyStatus = wrappingText();
  private final JButton removeKeyButton = new JButton(tr("Remove API Key"));
  private final JButton testButton = new JButton(tr("Test Connection"));
  /** Wraps too: long endpoint errors must not be cut off. */
  private final JTextArea testResult = wrappingText();

  /** A read-only text that looks like a label but wraps long lines. */
  private static JTextArea wrappingText() {
    JTextArea t = new JTextArea(" ");
    t.setEditable(false);
    t.setFocusable(false);
    t.setOpaque(false);
    t.setBorder(null);
    t.setLineWrap(true);
    t.setWrapStyleWord(true);
    t.setFont(new JLabel().getFont());
    // a small preferred width: the row stretches it, and long text wraps instead of widening the page
    t.setColumns(20);
    return t;
  }
  private final DefaultListModel<String> skillDirs = new DefaultListModel<>();
  private final JList<String> skillList = new JList<>(skillDirs);
  private final JCheckBox confirmEditsBox = new JCheckBox(tr("Ask before the AI changes a file (shows the changes first)"));
  private final JCheckBox allowTerminalBox = new JCheckBox(tr("Allow the AI to run terminal commands (always asks first)"));
  private final JTextField homeField = new JTextField(30);
  private final char echoChar;

  private AiSettings loaded;
  /** Variable name of the key stored in the selected data folder, or null. */
  private String storedKeyEnv;
  /** Key + provider + base URL that passed "Test Connection". */
  private String verified;
  /** Increased for every model list request, so only the newest one is shown. */
  private int modelRequest;

  public AiPreferencesPanel() {
    super(new GridBagLayout());
    setBorder(BorderFactory.createEmptyBorder(Theme.scale(8), Theme.scale(8), Theme.scale(8), Theme.scale(8)));
    echoChar = apiKeyField.getEchoChar();
    modelCombo.setEditable(true);

    int row = 0;
    if (!installed) {
      JLabel missing = new JLabel(I18n.format(tr("The AI component (Hermes Agent) was not found in: {0}"),
                                              HermesBundle.getDefault().getBundleDir().getAbsolutePath()));
      missing.setForeground(RED);
      add(missing, gbc(0, row++, 3));
    }

    add(new JLabel(tr("Provider:")), label(row));
    add(providerCombo, field(row++));
    providerCombo.addActionListener(e -> providerChanged());

    // Order of use: (Base URL,) API key, Test Connection, then the model this key can use.
    add(baseUrlLabel, label(row));
    add(baseUrlField, field(row++));
    baseUrlField.setToolTipText(tr("Address of an OpenAI-compatible server, for example http://localhost:11434/v1"));

    add(new JLabel(tr("API key:")), label(row));
    JPanel keyPanel = new JPanel(new BorderLayout(Theme.scale(4), 0));
    keyPanel.add(apiKeyField, BorderLayout.CENTER);
    keyPanel.add(showKeyBox, BorderLayout.EAST);
    showKeyBox.addActionListener(e -> apiKeyField.setEchoChar(showKeyBox.isSelected() ? (char) 0 : echoChar));
    add(keyPanel, field(row++));
    apiKeyField.getDocument().addDocumentListener(new DocumentListener() {
      @Override
      public void insertUpdate(DocumentEvent e) {
        keyTyped();
      }

      @Override
      public void removeUpdate(DocumentEvent e) {
        keyTyped();
      }

      @Override
      public void changedUpdate(DocumentEvent e) {
        keyTyped();
      }
    });

    // the status text wraps; the Remove button stays on its right
    JPanel statusPanel = new JPanel(new BorderLayout(Theme.scale(8), 0));
    statusPanel.add(apiKeyStatus, BorderLayout.CENTER);
    removeKeyButton.addActionListener(e -> removeKeyNow());
    JPanel removeWrap = new JPanel(new BorderLayout());
    removeWrap.add(removeKeyButton, BorderLayout.NORTH);
    statusPanel.add(removeWrap, BorderLayout.EAST);
    add(statusPanel, field(row++));

    JPanel testPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 0, 0));
    testPanel.add(testButton);
    testButton.addActionListener(e -> testConnection());
    add(testPanel, field(row++));
    add(testResult, field(row++));

    add(new JLabel(tr("Model:")), label(row));
    add(modelCombo, field(row++));
    modelStatus.setForeground(Color.GRAY);
    add(modelStatus, field(row++));

    // a status that wraps onto more lines may need a taller dialog
    DocumentListener grow = new DocumentListener() {
      @Override
      public void insertUpdate(DocumentEvent e) {
        SwingUtilities.invokeLater(() -> SwingUtilities.invokeLater(AiPreferencesPanel.this::growDialogToFit));
      }

      @Override
      public void removeUpdate(DocumentEvent e) {
      }

      @Override
      public void changedUpdate(DocumentEvent e) {
      }
    };
    apiKeyStatus.getDocument().addDocumentListener(grow);
    modelStatus.getDocument().addDocumentListener(grow);

    add(new JLabel(tr("Skill folders:")), labelTop(row));
    skillList.setVisibleRowCount(3);
    JPanel skillPanel = new JPanel(new BorderLayout(Theme.scale(4), 0));
    JScrollPane skillScroll = new JScrollPane(skillList);
    skillScroll.setPreferredSize(new java.awt.Dimension(Theme.scale(380), Theme.scale(66)));
    skillPanel.add(skillScroll, BorderLayout.CENTER);
    JPanel skillButtons = new JPanel(new java.awt.GridLayout(2, 1, 0, Theme.scale(4)));
    JButton addSkill = new JButton(tr("Add..."));
    addSkill.addActionListener(e -> addSkillFolder());
    JButton removeSkill = new JButton(tr("Remove"));
    removeSkill.addActionListener(e -> {
      for (String s : skillList.getSelectedValuesList()) {
        skillDirs.removeElement(s);
      }
    });
    skillButtons.add(addSkill);
    skillButtons.add(removeSkill);
    JPanel skillButtonsWrap = new JPanel(new BorderLayout());
    skillButtonsWrap.add(skillButtons, BorderLayout.NORTH);
    skillPanel.add(skillButtonsWrap, BorderLayout.EAST);
    add(skillPanel, field(row++));

    add(confirmEditsBox, gbc(0, row++, 3));
    add(allowTerminalBox, gbc(0, row++, 3));

    add(new JLabel(tr("The AI can read (read-only):")), labelTop(row));
    JTextArea refs = new JTextArea(4, 0);
    refs.setEditable(false);
    refs.setText(referenceText());
    refs.setCaretPosition(0);
    refs.setBackground(getBackground());
    JScrollPane refsScroll = new JScrollPane(refs);
    // long paths scroll instead of widening the whole Preferences dialog
    refsScroll.setPreferredSize(new java.awt.Dimension(Theme.scale(420), refs.getPreferredSize().height + Theme.scale(22)));
    add(refsScroll, field(row++));

    add(new JLabel(tr("AI workspace:")), label(row));
    JPanel homePanel = new JPanel(new BorderLayout(Theme.scale(4), 0));
    homeField.setPreferredSize(new java.awt.Dimension(Theme.scale(380), homeField.getPreferredSize().height));
    homeField.setToolTipText(tr("Where the AI keeps its API key, settings and conversations"));
    homeField.addActionListener(e -> homeChanged());
    homeField.addFocusListener(new FocusAdapter() {
      @Override
      public void focusLost(FocusEvent e) {
        homeChanged();
      }
    });
    homePanel.add(homeField, BorderLayout.CENTER);
    JButton chooseHome = new JButton(tr("Select..."));
    chooseHome.addActionListener(e -> chooseHomeFolder());
    homePanel.add(chooseHome, BorderLayout.EAST);
    add(homePanel, field(row++));

    // push everything to the top
    GridBagConstraints filler = gbc(0, row, 3);
    filler.weighty = 1;
    add(new JLabel(), filler);

    for (JComponent c : new JComponent[]{providerCombo, modelCombo, baseUrlField, apiKeyField, showKeyBox,
                                         removeKeyButton, testButton, skillList, addSkill, removeSkill,
                                         confirmEditsBox, allowTerminalBox, homeField, chooseHome}) {
      c.setEnabled(installed);
    }
    load();
  }

  private static GridBagConstraints gbc(int x, int y, int width) {
    GridBagConstraints c = new GridBagConstraints();
    c.gridx = x;
    c.gridy = y;
    c.gridwidth = width;
    c.anchor = GridBagConstraints.WEST;
    c.fill = GridBagConstraints.HORIZONTAL;
    c.insets = new Insets(Theme.scale(3), Theme.scale(3), Theme.scale(3), Theme.scale(3));
    return c;
  }

  private static GridBagConstraints label(int row) {
    GridBagConstraints c = gbc(0, row, 1);
    c.fill = GridBagConstraints.NONE;
    return c;
  }

  private static GridBagConstraints labelTop(int row) {
    GridBagConstraints c = label(row);
    c.anchor = GridBagConstraints.NORTHWEST;
    return c;
  }

  private static GridBagConstraints field(int row) {
    GridBagConstraints c = gbc(1, row, 2);
    c.weightx = 1;
    return c;
  }

  private String referenceText() {
    StringBuilder sb = new StringBuilder();
    for (Map.Entry<String, File> e : AiContextBuilder.referenceFolders().entrySet()) {
      sb.append(e.getValue().getAbsolutePath()).append('\n');
    }
    return sb.toString().trim();
  }

  // ------------------------------------------------------------------ state

  private void load() {
    loading = true;
    loaded = AiSettings.load();
    homeField.setText(HermesBundle.forHome(loaded.homePath).getHomeDir().getAbsolutePath());
    homeField.setCaretPosition(0);
    storedKeyEnv = currentBundle().storedApiKeyEnv();
    baseUrlField.setText(loaded.baseUrl);
    providerCombo.setSelectedItem(loaded.getProvider());
    providerChanged();
    // the saved model stays shown; the list comes from the saved key (it passed a test before)
    modelCombo.setSelectedItem(loaded.model);
    skillDirs.clear();
    for (String d : loaded.skillDirs) {
      skillDirs.addElement(d);
    }
    confirmEditsBox.setSelected(loaded.confirmEdits);
    allowTerminalBox.setSelected(loaded.allowTerminal);
    loading = false;
    loadModelsWithSavedKey(false);
  }

  /** True while load() fills the fields, so providerChanged() does not clear or load anything. */
  private boolean loading;

  private HermesBundle currentBundle() {
    return HermesBundle.forHome(homeField.getText());
  }

  private AiSettings.Provider selectedProvider() {
    return (AiSettings.Provider) providerCombo.getSelectedItem();
  }

  private void providerChanged() {
    AiSettings.Provider p = selectedProvider();
    if (p == null) {
      return;
    }
    // No built-in list: the box stays empty until this provider's key lists its models.
    modelRequest++; // a list still loading for the previous provider is dropped
    modelCombo.setModel(new DefaultComboBoxModel<>());
    modelCombo.setSelectedItem("");
    baseUrlLabel.setVisible(p.needsBaseUrl);
    baseUrlField.setVisible(p.needsBaseUrl);
    fillTrialKey(p);
    updateKeyStatus();
    if (!loading) {
      // with a saved key for this provider, list its models and pick the newest
      loadModelsWithSavedKey(true);
    }
    revalidate();
    // the Base URL row may have appeared: grow the dialog instead of scrolling
    SwingUtilities.invokeLater(() -> SwingUtilities.invokeLater(this::growDialogToFit));
  }

  private void keyTyped() {
    if (!typedKey().isEmpty()) {
      testResult.setForeground(Color.GRAY);
      testResult.setText(tr("Press Test Connection to check this API key before saving it."));
    } else {
      testResult.setText(" ");
    }
    updateKeyStatus();
  }

  /** A saved key belongs to this provider. */
  private boolean hasSavedKeyFor(AiSettings.Provider p) {
    return p != null && p.apiKeyEnv.equals(storedKeyEnv);
  }

  /** Shown with the free trial key: the users' code goes to Google's free tier. */
  private static String trialDataNotice() {
    return tr("Please note: under the terms of Google's free tier, what you send (including your code) may be used by Google to improve its products.");
  }

  private static boolean isTrialKey(String key) {
    for (AiSettings.Provider p : AiSettings.PROVIDERS) {
      if (p.trialKey != null && p.trialKey.equals(key)) {
        return true;
      }
    }
    return false;
  }

  /**
   * Fills in 86Duino's free trial key when this provider has one and the user
   * has no own key for it; removes a trial key that belongs to another provider.
   */
  private void fillTrialKey(AiSettings.Provider p) {
    String typed = typedKey();
    if (p.trialKey != null && !hasSavedKeyFor(p) && typed.isEmpty()) {
      apiKeyField.setText(p.trialKey);
    } else if (isTrialKey(typed) && !typed.equals(p.trialKey)) {
      apiKeyField.setText("");
    }
  }

  /** Lists the models only with a saved key: a typed key is listed after Test Connection. */
  private void loadModelsWithSavedKey(boolean pickNewest) {
    if (hasSavedKeyFor(selectedProvider())) {
      loadModels(pickNewest);
    } else {
      modelStatus.setForeground(Color.GRAY);
      modelStatus.setText(tr("After Test Connection succeeds, the models this API key can use are listed here."));
    }
  }

  private void homeChanged() {
    String env = currentBundle().storedApiKeyEnv();
    boolean same = env == null ? storedKeyEnv == null : env.equals(storedKeyEnv);
    storedKeyEnv = env;
    if (!same) {
      fillTrialKey(selectedProvider());
      updateKeyStatus();
      loadModelsWithSavedKey(false);
    } else {
      updateKeyStatus();
    }
  }

  private static String providerLabelForEnv(String env) {
    List<String> names = new ArrayList<>();
    for (AiSettings.Provider p : AiSettings.PROVIDERS) {
      if (p.apiKeyEnv.equals(env)) {
        names.add(p.label);
      }
    }
    return String.join(" / ", names);
  }

  private void updateKeyStatus() {
    AiSettings.Provider p = selectedProvider();
    if (p == null) {
      return;
    }
    String typed = typedKey();
    if (p.trialKey != null && hasSavedKeyFor(p) && typed.isEmpty()
               && p.trialKey.equals(currentBundle().readEnvValue(p.apiKeyEnv))) {
      apiKeyStatus.setForeground(GREEN);
      apiKeyStatus.setText(tr("Using the free trial API key provided by 86Duino (shared usage limits).") + "\n" + trialDataNotice());
      removeKeyButton.setVisible(true);
    } else if (p.trialKey != null && p.trialKey.equals(typed)) {
      apiKeyStatus.setForeground(GREEN);
      apiKeyStatus.setText(tr("The free trial API key provided by 86Duino is filled in (shared usage limits). Press Test Connection, then OK.")
                           + "\n" + trialDataNotice());
      removeKeyButton.setVisible(false);
    } else if (storedKeyEnv == null) {
      apiKeyStatus.setForeground(Color.GRAY);
      apiKeyStatus.setText(tr("No API key saved yet. Enter one, press Test Connection, then OK."));
      removeKeyButton.setVisible(false);
    } else if (storedKeyEnv.equals(p.apiKeyEnv)) {
      apiKeyStatus.setForeground(GREEN);
      apiKeyStatus.setText(tr("An API key is saved. Leave the field empty to keep it."));
      removeKeyButton.setVisible(true);
    } else {
      apiKeyStatus.setForeground(RED);
      apiKeyStatus.setText(I18n.format(tr("The saved API key is for {0}. Enter a {1} API key (it replaces the saved one)."),
                                       providerLabelForEnv(storedKeyEnv), p.label));
      removeKeyButton.setVisible(true);
    }
  }

  private String typedKey() {
    return new String(apiKeyField.getPassword()).trim();
  }

  private String verificationKey(String key) {
    AiSettings.Provider p = selectedProvider();
    return key + "\u0000" + (p == null ? "" : p.id) + "\u0000" + (p != null && p.needsBaseUrl ? baseUrlField.getText().trim() : "");
  }

  /** The key to use for listing models and testing: typed, else the stored one. */
  private String keyForRequests() {
    String key = typedKey();
    if (!key.isEmpty()) {
      return key;
    }
    AiSettings.Provider p = selectedProvider();
    if (hasSavedKeyFor(p)) {
      String stored = currentBundle().readEnvValue(p.apiKeyEnv);
      return stored == null ? "" : stored;
    }
    return "";
  }

  // ------------------------------------------------------------------ model list

  /**
   * Loads the models this key can use (first ten, newest first) in the
   * background. Only lists from the provider are shown, never a built-in one.
   * With pickNewest the newest regular model is selected unless the current
   * one is listed. Called when the page opens and when the provider changes
   * (both with a saved key only), and after a successful Test Connection; Test
   * Connection also loads it when no model is chosen yet.
   */
  private void loadModels(boolean pickNewest) {
    AiSettings.Provider p = selectedProvider();
    if (p == null || !installed) {
      return;
    }
    String key = keyForRequests();
    if (key.isEmpty() || (p.needsBaseUrl && baseUrlField.getText().trim().isEmpty())) {
      modelStatus.setForeground(Color.GRAY);
      modelStatus.setText(tr("After Test Connection succeeds, the models this API key can use are listed here."));
      return;
    }
    int request = ++modelRequest;
    String baseUrl = baseUrlField.getText().trim();
    HermesBundle bundle = currentBundle();
    modelStatus.setForeground(Color.GRAY);
    modelStatus.setText(tr("Loading the model list..."));
    Thread t = new Thread(() -> {
      HermesBundle.ModelList list;
      try {
        list = bundle.listModels(p.id, key, baseUrl);
      } catch (Exception e) {
        list = null;
      }
      HermesBundle.ModelList result = list;
      SwingUtilities.invokeLater(() -> showModels(request, p, result, pickNewest));
    }, "ai-list-models");
    t.setDaemon(true);
    t.start();
  }

  private void showModels(int request, AiSettings.Provider p, HermesBundle.ModelList list, boolean pickNewest) {
    if (request != modelRequest || p != selectedProvider()) {
      return; // a newer request or another provider is shown now
    }
    applyModelList(p, list, pickNewest);
  }

  /** Puts a loaded list into the model box; returns false when it had no models. */
  private boolean applyModelList(AiSettings.Provider p, HermesBundle.ModelList list, boolean pickNewest) {
    modelStatus.setForeground(Color.GRAY);
    if (list == null) {
      modelStatus.setText(tr("Could not get the model list."));
      return false;
    }
    if (list.models.isEmpty()) {
      modelStatus.setText(list.error.isEmpty() ? tr("The server did not list any models.")
                          : I18n.format(tr("Could not get the model list: {0}"), shorten(list.error, 150)));
      return false;
    }
    Object current = modelCombo.getEditor().getItem();
    String currentModel = current == null ? "" : current.toString().trim();
    modelCombo.setModel(new DefaultComboBoxModel<>(list.models.toArray(new String[0])));
    if (list.models.contains(currentModel) || (!pickNewest && !currentModel.isEmpty())) {
      // keep the user's model, typed by hand too (the box stays editable)
      modelCombo.setSelectedItem(currentModel);
    } else {
      modelCombo.setSelectedItem(list.recommended.isEmpty() ? list.models.get(0) : list.recommended);
    }
    modelStatus.setText(p.needsBaseUrl
        ? I18n.format(tr("Models your API key can use at {0} (first {1}, in the server's order)."), p.label, list.models.size())
        : I18n.format(tr("Models your API key can use at {0} (first {1}, newest first)."), p.label, list.models.size()));
    return true;
  }

  /** The model chosen in the box. */
  private String selectedModel() {
    Object m = modelCombo.getEditor().getItem();
    return m == null ? "" : m.toString().trim();
  }

  // ------------------------------------------------------------------ actions

  private void addSkillFolder() {
    JFileChooser chooser = new JFileChooser();
    chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
    chooser.setDialogTitle(tr("Select a skill folder"));
    if (chooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
      String path = chooser.getSelectedFile().getAbsolutePath();
      if (!skillDirs.contains(path)) {
        skillDirs.addElement(path);
      }
    }
  }

  private void chooseHomeFolder() {
    JFileChooser chooser = new JFileChooser();
    chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
    chooser.setDialogTitle(tr("Select the AI workspace"));
    File current = new File(homeField.getText().trim());
    chooser.setCurrentDirectory(current.isDirectory() ? current : current.getParentFile());
    chooser.setSelectedFile(current);
    if (chooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
      homeField.setText(chooser.getSelectedFile().getAbsolutePath());
      homeField.setCaretPosition(0);
      homeChanged();
    }
  }

  private AiSettings collect() {
    AiSettings s = new AiSettings();
    s.provider = selectedProvider().id;
    Object model = modelCombo.getEditor().getItem();
    s.model = model == null ? "" : model.toString().trim();
    s.baseUrl = baseUrlField.getText().trim();
    List<String> dirs = new ArrayList<>();
    for (int i = 0; i < skillDirs.size(); i++) {
      dirs.add(skillDirs.get(i));
    }
    s.skillDirs = dirs;
    s.confirmEdits = confirmEditsBox.isSelected();
    s.allowTerminal = allowTerminalBox.isSelected();
    // the default folder is stored as "" so it follows %APPDATA% on every computer
    File home = new File(homeField.getText().trim());
    s.homePath = home.getAbsoluteFile().equals(HermesBundle.defaultHomeDir().getAbsoluteFile()) ? "" : home.getAbsolutePath();
    return s;
  }

  /** Returns an error message, or null when the settings can be saved. */
  public String validateSettings() {
    if (!installed) {
      return null;
    }
    // No model or Base URL yet is fine: OK saves what is set, the AI panel asks for the rest.
    String key = typedKey();
    // A key the user typed must pass Test Connection before it is saved; say so. The
    // auto-filled trial key is simply not saved until it was tested (it is filled in again).
    if (!key.isEmpty() && !isTrialKey(key) && !verificationKey(key).equals(verified)) {
      return tr("The new API key has not passed Test Connection yet.\nPress Test Connection first; it is saved after the test succeeds and you press OK.");
    }
    File home = new File(homeField.getText().trim());
    if (!home.isAbsolute() || (!home.isDirectory() && !home.mkdirs())) {
      return tr("The AI workspace cannot be used. Please choose another folder.");
    }
    return null;
  }

  /** Called when OK is pressed (after validateSettings succeeded). */
  public void save() {
    if (!installed) {
      return;
    }
    AiSettings s = collect();
    s.save();
    String typed = typedKey();
    // only a key that passed Test Connection is stored (an untested trial key is not)
    String key = !typed.isEmpty() && verificationKey(typed).equals(verified) ? typed : "";
    boolean hermesChanged = !s.runtimeSignature().equals(loaded.runtimeSignature()) || !key.isEmpty();
    if (!hermesChanged) {
      return;
    }
    HermesBundle bundle = HermesBundle.forHome(s.homePath);
    pendingSave = CompletableFuture.runAsync(() -> {
      try {
        bundle.applySettings(s, key, false);
      } catch (Exception e) {
        SwingUtilities.invokeLater(() -> Base.showWarning(tr("AI Assistant"),
            tr("Could not save the AI settings:") + "\n" + e.getMessage(), null));
        throw new RuntimeException(e);
      }
    });
    // The AI panels show "saving" now (OK refreshes every editor), and the
    // chat or the API key hint as soon as the settings are written.
    pendingSave.whenComplete((v, t) -> SwingUtilities.invokeLater(AiPreferencesPanel::refreshAllEditors));
  }

  /**
   * "Remove API Key": asks, then removes the stored key right away (OK is not
   * needed), and every open AI panel goes back to the "no API key" hint.
   */
  private void removeKeyNow() {
    int answer = javax.swing.JOptionPane.showConfirmDialog(this,
        tr("Remove the saved API key now?\nThe AI Assistant cannot be used until an API key is saved again."),
        tr("Remove API Key"), javax.swing.JOptionPane.YES_NO_OPTION, javax.swing.JOptionPane.WARNING_MESSAGE);
    if (answer != javax.swing.JOptionPane.YES_OPTION) {
      return; // keep everything as it is
    }
    HermesBundle bundle = currentBundle();
    removeKeyButton.setEnabled(false);
    apiKeyStatus.setForeground(Color.GRAY);
    apiKeyStatus.setText(tr("Removing the API key..."));
    pendingSave = CompletableFuture.runAsync(() -> {
      try {
        bundle.removeApiKeys();
      } catch (Exception e) {
        SwingUtilities.invokeLater(() -> Base.showWarning(tr("AI Assistant"),
            tr("Could not remove the API key:") + "\n" + e.getMessage(), null));
        throw new RuntimeException(e);
      }
    });
    pendingSave.whenComplete((v, t) -> SwingUtilities.invokeLater(() -> {
      removeKeyButton.setEnabled(!testing);
      storedKeyEnv = currentBundle().storedApiKeyEnv();
      verified = null;
      // the listed models came from the removed key
      modelRequest++;
      String model = selectedModel();
      modelCombo.setModel(new DefaultComboBoxModel<>());
      modelCombo.setSelectedItem(model);
      loadModelsWithSavedKey(false);
      fillTrialKey(selectedProvider());
      updateKeyStatus();
      refreshAllEditors();
    }));
  }

  /** A Test Connection is running. */
  private boolean testing;

  /** While a test runs, neither another test nor removing the key (which the test may be using) can start. */
  private void setTesting(boolean running) {
    testing = running;
    testButton.setEnabled(!running);
    removeKeyButton.setEnabled(!running);
  }

  /** True while settings saved with OK are still being written. */
  public static boolean isSaving() {
    return !pendingSave.isDone();
  }

  /** Lets every open editor window update its AI panel. */
  static void refreshAllEditors() {
    for (java.awt.Frame f : java.awt.Frame.getFrames()) {
      if (f instanceof processing.app.Editor && f.isDisplayable()) {
        ((processing.app.Editor) f).aiSettingsChanged();
      }
    }
  }

  /**
   * Tests the settings with one short question. When no model is chosen yet,
   * the models this key can use are loaded first and the newest regular one is
   * picked (a custom endpoint: the server's first).
   */
  private void testConnection() {
    AiSettings s = collect();
    AiSettings.Provider p = s.getProvider();
    if (p.needsBaseUrl && s.baseUrl.isEmpty()) {
      showTestResult(false, tr("Please enter the Base URL of the AI server."));
      return;
    }
    String typed = typedKey();
    String key = keyForRequests();
    if (key.isEmpty()) {
      showTestResult(false, tr("Please enter an API key first."));
      return;
    }
    boolean needModel = s.model.isEmpty();
    String verification = verificationKey(typed);
    HermesBundle bundle = currentBundle();
    setTesting(true);
    testResult.setForeground(Color.GRAY);
    testResult.setText(needModel ? tr("Loading the model list...") : tr("Testing... (this can take up to a minute)"));
    long start = System.currentTimeMillis();
    Thread t = new Thread(() -> {
      if (needModel) {
        HermesBundle.ModelList list;
        try {
          list = bundle.listModels(p.id, key, s.baseUrl);
        } catch (Exception e) {
          list = new HermesBundle.ModelList(false, java.util.Collections.emptyList(), "", String.valueOf(e.getMessage()));
        }
        HermesBundle.ModelList loadedList = list;
        boolean[] gotModel = new boolean[1];
        try {
          SwingUtilities.invokeAndWait(() -> {
            modelRequest++; // a list still loading must not replace this one
            gotModel[0] = applyModelList(p, loadedList, true);
            if (gotModel[0]) {
              testResult.setText(tr("Testing... (this can take up to a minute)"));
            }
          });
        } catch (Exception e) {
          gotModel[0] = false;
        }
        if (!gotModel[0]) {
          String why = loadedList.error.isEmpty() ? tr("The server did not list any models.") : loadedList.error;
          SwingUtilities.invokeLater(() -> {
            setTesting(false);
            showTestResult(false, I18n.format(tr("Could not get the model list from the server: {0}\nYou can also type the model name in the Model box."), shorten(why, 400)));
          });
          return;
        }
        s.model = loadedList.recommended.isEmpty() ? loadedList.models.get(0) : loadedList.recommended;
      }
      String text;
      boolean ok;
      try {
        HermesBundle.Result r = bundle.testConnection(s, key);
        ok = r.ok() && r.output.contains("OK");
        text = ok ? I18n.format(tr("Connected ({0} s)"), (System.currentTimeMillis() - start) / 1000)
                  : failureText(r.output, s.model, needModel);
      } catch (Exception e) {
        ok = false;
        text = tr("Failed: ") + e.getMessage();
      }
      boolean success = ok;
      // the model was picked for the user: say so, it is easy to change before OK
      String picked = p.needsBaseUrl
          ? tr("The first model of the server \"{0}\" was selected automatically; you can choose another one in the Model box.")
          : "gemini".equals(p.id)
          ? tr("The newest flash model \"{0}\" was selected automatically; you can choose another one in the Model box.")
          : tr("The newest model \"{0}\" was selected automatically; you can choose another one in the Model box.");
      String message = success && needModel ? text + "\n" + I18n.format(picked, s.model) : text;
      SwingUtilities.invokeLater(() -> {
        setTesting(false);
        if (success && !typed.isEmpty() && typed.equals(typedKey())) {
          // this key may now be saved with OK
          verified = verification;
          showTestResult(true, message + "\n" + tr("Press OK to save this API key."));
        } else {
          showTestResult(success, message);
        }
        if (success && !needModel) {
          loadModels(false); // the models this key can use (already loaded when needModel)
        }
      });
    }, "ai-test-connection");
    t.setDaemon(true);
    t.start();
  }

  private void showTestResult(boolean ok, String message) {
    testResult.setForeground(ok ? GREEN : RED);
    testResult.setText(message);
    revalidate();
    // once the wrapped text has its height, make the dialog taller if the page no longer fits
    SwingUtilities.invokeLater(() -> SwingUtilities.invokeLater(this::growDialogToFit));
  }

  private void growDialogToFit() {
    java.awt.Window w = SwingUtilities.getWindowAncestor(this);
    if (w == null || !(getParent() instanceof javax.swing.JViewport)) {
      return;
    }
    // How much taller the page is than the tab shows. (The dialog's own preferred size
    // cannot be used: the scroll pane stops the change from reaching the outer layout.)
    int missing = getPreferredSize().height - ((javax.swing.JViewport) getParent()).getExtentSize().height;
    if (missing <= 0) {
      return;
    }
    java.awt.Rectangle screen = w.getGraphicsConfiguration().getBounds();
    Insets insets = java.awt.Toolkit.getDefaultToolkit().getScreenInsets(w.getGraphicsConfiguration());
    int top = screen.y + insets.top;
    int maxHeight = screen.height - insets.top - insets.bottom;
    int height = Math.min(w.getHeight() + missing, maxHeight);
    int y = Math.max(top, Math.min(w.getY(), top + maxHeight - height));
    w.setBounds(w.getX(), y, w.getWidth(), height);
    w.validate();
  }

  // Scrollable: the page fills the tab's width; it scrolls vertically only when the dialog is too short

  @Override
  public java.awt.Dimension getPreferredScrollableViewportSize() {
    return getPreferredSize();
  }

  @Override
  public int getScrollableUnitIncrement(java.awt.Rectangle visible, int orientation, int direction) {
    return Theme.scale(16);
  }

  @Override
  public int getScrollableBlockIncrement(java.awt.Rectangle visible, int orientation, int direction) {
    return Math.max(Theme.scale(16), visible.height - Theme.scale(32));
  }

  @Override
  public boolean getScrollableTracksViewportWidth() {
    return true;
  }

  @Override
  public boolean getScrollableTracksViewportHeight() {
    // fill the tab when it is taller than the page, scroll when it is shorter
    return getParent() != null && getParent().getHeight() > getPreferredSize().height;
  }

  /**
   * Errors a server gives when the model cannot do tool calling (function
   * calling), which the AI Assistant always needs: e.g. llama.cpp's "Unable to
   * generate parser for this template" for Llama 3.1, or vLLM's "auto tool
   * choice requires --enable-auto-tool-choice".
   */
  private static final java.util.regex.Pattern TOOL_CALL_UNSUPPORTED = java.util.regex.Pattern.compile(
      "(?i)tool[-_ ]?calls?\\b|tool[-_ ]?choice|function[-_ ]?call|tools? (are|is) not supported"
      + "|does(n't| not) support tools|parser for this template");

  /**
   * The text shown when Test Connection fails.
   *
   * 1. A hint first when it is worth acting on: the model was picked
   *    automatically (the first of the list), and/or the error says the model
   *    cannot do tool calling.
   * 2. The endpoint's own error ("Provider said: ..." in Hermes' output).
   * 3. Hermes' explanation, except for tool-calling errors: Hermes then says
   *    "temporarily unavailable", which is wrong (that model always fails).
   *
   * Hermes words every HTTP 400 as "rejected this request as malformed", which
   * hides the real reason (e.g. "No connected db.").
   */
  static String failureText(String output, String model, boolean autoPicked) {
    String o = output == null ? "" : output.trim();
    boolean toolIssue = TOOL_CALL_UNSUPPORTED.matcher(o).find();
    String hint = "";
    if (autoPicked && toolIssue) {
      hint = I18n.format(tr("The model \"{0}\" was chosen automatically and may not support the tool calling (function calling) the AI Assistant needs. Choose another model in the Model box and try again."), model) + "\n";
    } else if (autoPicked) {
      hint = I18n.format(tr("The model \"{0}\" was chosen automatically. Choose another model in the Model box and try again."), model) + "\n";
    } else if (toolIssue) {
      hint = I18n.format(tr("The model \"{0}\" may not support the tool calling (function calling) the AI Assistant needs. Choose another model in the Model box and try again."), model) + "\n";
    }
    return hint + endpointText(o, !toolIssue);
  }

  private static String endpointText(String o, boolean withExplanation) {
    if (o.isEmpty()) {
      return tr("Failed: ") + tr("no answer");
    }
    String said = null;
    StringBuilder rest = new StringBuilder();
    for (String line : o.split("\\r?\\n")) {
      String l = line.trim();
      if (said == null && l.startsWith("Provider said:")) {
        said = l.substring("Provider said:".length()).trim();
      } else if (!l.isEmpty()) {
        if (rest.length() > 0) {
          rest.append(' ');
        }
        rest.append(l);
      }
    }
    String explanation = shorten(rest.toString(), 400);
    if (said == null) {
      return tr("Failed: ") + explanation;
    }
    return tr("Failed: ") + I18n.format(tr("the endpoint answered \"{0}\""), shorten(said, 400))
           + (explanation.isEmpty() || !withExplanation ? "" : "\n" + explanation);
  }

  private static String shorten(String s, int max) {
    return s.length() > max ? s.substring(0, max) + "..." : s;
  }
}
