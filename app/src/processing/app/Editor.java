/* -*- mode: java; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
  Part of the Processing project - http://processing.org

  Copyright (c) 2004-09 Ben Fry and Casey Reas
  Copyright (c) 2001-04 Massachusetts Institute of Technology

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

package processing.app;

import static processing.app.I18n.tr;
import static processing.app.Theme.scale;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.KeyboardFocusManager;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.print.PageFormat;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;
import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.io.FileReader;
import java.io.FileInputStream;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.FileNotFoundException;

import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.ConnectException;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.function.Consumer;
import java.util.function.Predicate;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import javax.swing.AbstractAction;
import javax.swing.Box;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;
import javax.swing.TransferHandler;
import javax.swing.event.MenuEvent;
import javax.swing.event.MenuListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.JTextComponent;
//import javafx.application.Platform;

import org.fife.ui.rsyntaxtextarea.folding.FoldManager;

import com.jcraft.jsch.JSchException;

import cc.arduino.CompilerProgressListener;
import cc.arduino.packages.BoardPort;
import cc.arduino.packages.MonitorFactory;
import cc.arduino.packages.Uploader;
import cc.arduino.packages.uploaders.SerialUploader;
import cc.arduino.view.GoToLineNumber;
import cc.arduino.view.StubMenuListener;
import cc.arduino.view.findreplace.FindReplace;

import jssc.SerialPortException;
import processing.app.debug.RunnerException;
import processing.app.debug.TargetBoard;                                        
import processing.app.forms.PasswordAuthorizationDialog;
import processing.app.helpers.DocumentTextChangeListener;
import processing.app.helpers.Keys;
import processing.app.helpers.OSUtils;
import processing.app.helpers.PreferencesMapException;
import processing.app.helpers.StringReplacer;
import processing.app.legacy.PApplet;
import processing.app.syntax.PdeKeywords;
import processing.app.syntax.SketchTextArea;
import processing.app.tools.MenuScroller;
import processing.app.tools.Tool;
import processing.app.Platform;

import processing.app.helpers.ProcessUtils;

/**
 * Main editor panel for the Processing Development Environment.
 */
@SuppressWarnings("serial")
public class Editor extends JFrame implements RunnerListener {

  public static final int MAX_TIME_AWAITING_FOR_RESUMING_SERIAL_MONITOR = 10000;

  final Platform platform;
  private JMenu recentSketchesMenu;
  private JMenu programmersMenu;
  private JMenu burnbootloaderMenu;                                 
  private final Box upper;
  private ArrayList<EditorTab> tabs = new ArrayList<>();
  private int currentTabIndex = -1;

  private static class ShouldSaveIfModified
      implements Predicate<SketchController> {

    @Override
    public boolean test(SketchController controller) {
      return PreferencesData.getBoolean("editor.save_on_verify")
             && controller.getSketch().isModified()
             && !controller.isReadOnly();
    }
  }

  private static class CanExportInSketchFolder
      implements Predicate<SketchController> {

    @Override
    public boolean test(SketchController controller) {
      if (controller.isReadOnly()) {
        return false;
      }
      if (controller.getSketch().isModified()) {
        return PreferencesData.getBoolean("editor.save_on_verify");
      }
      return true;
    }
  }

  final Base base;

  // otherwise, if the window is resized with the message label
  // set to blank, it's preferredSize() will be fukered
  private static final String EMPTY =
    "                                                                     " +
    "                                                                     " +
    "                                                                     ";

  /** Command on Mac OS X, Ctrl on Windows and Linux */
  private static final int SHORTCUT_KEY_MASK =
    Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
  /** Command-W on Mac OS X, Ctrl-W on Windows and Linux */
  static final KeyStroke WINDOW_CLOSE_KEYSTROKE =
    KeyStroke.getKeyStroke('W', SHORTCUT_KEY_MASK);
  /** Command-Option on Mac OS X, Ctrl-Alt on Windows and Linux */
  static final int SHORTCUT_ALT_KEY_MASK = ActionEvent.ALT_MASK |
    Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();

  /**
   * true if this file has not yet been given a name by the user
   */
  boolean untitled;

  private PageFormat pageFormat;

  // file, sketch, and tools menus for re-inserting items
  private JMenu fileMenu;
  private JMenu toolsMenu;

  private int numTools = 0;

  static public boolean avoidMultipleOperations = false;

  private EditorToolbar toolbar;
  // these menus are shared so that they needn't be rebuilt for all windows
  // each time a sketch is created, renamed, or moved.
  static JMenu toolbarMenu;
  static JMenu sketchbookMenu;
  static JMenu examplesMenu;
  static JMenu importMenu;
  static JMenu appearanceMenu;

  private static JMenu portMenu;

  static public volatile AbstractMonitor serialMonitor;
  static public AbstractMonitor serialPlotter;

  final EditorHeader header;
  EditorStatus status;
  EditorConsole console;

  private JSplitPane splitPane;

  // 86Duino: AI assistant panel on the right of the editor and console
  private JSplitPane aiSplitPane;
  private cc.arduino.ai.AiChatPanel aiPanel;
  private boolean aiPanelVisible;

  // currently opened program
  SketchController sketchController;
  Sketch sketch;

  EditorLineStatus lineStatus;

  //JEditorPane editorPane;

  /** Contains all EditorTabs, of which only one will be visible */
  private JPanel codePanel;

  //Runner runtime;

  private JMenuItem saveMenuItem;
  private JMenuItem saveAsMenuItem;

  //boolean presenting;
  static private boolean uploading;

  // undo fellers
  private JMenuItem undoItem;
  private JMenuItem redoItem;

  private FindReplace find;

  Runnable runHandler;
  Runnable presentHandler;
  private Runnable runAndSaveHandler;
  private Runnable presentAndSaveHandler;
  private UploadHandler uploadHandler;
  private UploadHandler uploadUsingProgrammerHandler;
  private Runnable timeoutUploadHandler;
  private Upload86EVARes upload86EVAres;

  private Tool ArduBlockTool;
  private Tool HMITool;
  private Tool EVATool;
  private Tool LDmicroTool;
  private Map<String, Tool> internalToolCache = new HashMap<String, Tool>();
  private Map<String, JMenuItem> globalToolItems = new HashMap<>();
  private Map<Integer, JMenuItem> removeToolSlots = new HashMap<>();
  private JMenuItem helpItem;

  public Editor(Base ibase, File file, int[] storedLocation, int[] defaultLocation, Platform platform) throws Exception {
    super("Arduino");
    this.base = ibase;
    this.platform = platform;

    Base.setIcon(this);

    // Install default actions for Run, Present, etc.
    resetHandlers();

    // add listener to handle window close box hit event
    addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {
          base.handleClose(Editor.this);
        }
      });
    // don't close the window when clicked, the app will take care
    // of that via the handleQuitInternal() methods
    // http://dev.processing.org/bugs/show_bug.cgi?id=440
    setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

    // When bringing a window to front, let the Base know
    addWindowListener(new WindowAdapter() {
        public void windowActivated(WindowEvent e) {
          base.handleActivated(Editor.this);
        }

        // added for 1.0.5
        // http://dev.processing.org/bugs/show_bug.cgi?id=1260
        public void windowDeactivated(WindowEvent e) {
          List<Component> toolsMenuItemsToRemove = new LinkedList<>();
          for (Component menuItem : toolsMenu.getMenuComponents()) {
            if (menuItem instanceof JComponent) {
              Object removeOnWindowDeactivation = ((JComponent) menuItem).getClientProperty("removeOnWindowDeactivation");
              if (removeOnWindowDeactivation != null && Boolean.valueOf(removeOnWindowDeactivation.toString())) {
                toolsMenuItemsToRemove.add(menuItem);
              }
            }
          }
          for (Component menuItem : toolsMenuItemsToRemove) {
            toolsMenu.remove(menuItem);
          }
          toolsMenu.remove(portMenu);
        }
      });

    //PdeKeywords keywords = new PdeKeywords();
    //sketchbook = new Sketchbook(this);

    buildMenuBar();

    // For rev 0120, placing things inside a JPanel
    Container contentPain = getContentPane();
    contentPain.setLayout(new BorderLayout());
    JPanel pane = new JPanel();
    pane.setLayout(new BorderLayout());
    contentPain.add(pane, BorderLayout.CENTER);

    Box box = Box.createVerticalBox();
    upper = Box.createVerticalBox();

    if (toolbarMenu == null) {
      toolbarMenu = new JMenu();
      base.rebuildToolbarMenu(toolbarMenu);
    }
    toolbar = new EditorToolbar(this, toolbarMenu);
    upper.add(toolbar);

    header = new EditorHeader(this);
    upper.add(header);

    // assemble console panel, consisting of status area and the console itself
    JPanel consolePanel = new JPanel();
    consolePanel.setLayout(new BorderLayout());

    status = new EditorStatus(this);
    consolePanel.add(status, BorderLayout.NORTH);

    console = new EditorConsole(base);
    console.setName("console");
    // windows puts an ugly border on this guy
    console.setBorder(null);
    consolePanel.add(console, BorderLayout.CENTER);

    lineStatus = new EditorLineStatus();
    consolePanel.add(lineStatus, BorderLayout.SOUTH);

    codePanel = new JPanel(new BorderLayout());
    upper.add(codePanel);

    splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, upper, consolePanel);

    // repaint child panes while resizing
    splitPane.setContinuousLayout(true);
    // if window increases in size, give all of increase to
    // the textarea in the uppper pane
    splitPane.setResizeWeight(1D);

    // to fix ugliness.. normally macosx java 1.3 puts an
    // ugly white border around this object, so turn it off.
    splitPane.setBorder(null);
    // By default, the split pane binds Ctrl-Tab and Ctrl-Shift-Tab for changing
    // focus. Since we do not use that, but want to use these shortcuts for
    // switching tabs, remove the bindings from the split pane. This allows the
    // events to bubble up and be handled by the EditorHeader.
    Keys.killBinding(splitPane, Keys.ctrl(KeyEvent.VK_TAB));
    Keys.killBinding(splitPane, Keys.ctrlShift(KeyEvent.VK_TAB));

    splitPane.setDividerSize(scale(splitPane.getDividerSize()));

    // the following changed from 600, 400 for netbooks
    // http://code.google.com/p/arduino/issues/detail?id=52
    // 86Duino: as narrow as the toolbar allows (every button and the longest
    // button title) instead of 600, so the AI panel divider can be dragged
    splitPane.setMinimumSize(new Dimension(toolbar.getRequiredWidth(), scale(100)));

    // 86Duino: the AI panel shares the window width with editor + console
    aiPanel = new cc.arduino.ai.AiChatPanel(this, base);
    aiSplitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, splitPane, null);
    aiSplitPane.setContinuousLayout(true);
    aiSplitPane.setResizeWeight(1D);
    aiSplitPane.setBorder(null);
    aiSplitPane.setDividerSize(0);
    Keys.killBinding(aiSplitPane, Keys.ctrl(KeyEvent.VK_TAB));
    Keys.killBinding(aiSplitPane, Keys.ctrlShift(KeyEvent.VK_TAB));
    aiSplitPane.addPropertyChangeListener(JSplitPane.DIVIDER_LOCATION_PROPERTY, e -> {
      int width = aiSplitPane.getWidth() - aiSplitPane.getDividerLocation() - aiSplitPane.getDividerSize();
      if (aiPanelVisible && aiSplitPane.getWidth() > 0 && width >= scale(200)) {
        PreferencesData.setInteger(cc.arduino.ai.AiSettings.PREF_PANEL_WIDTH, width);
      }
    });
    // If a resize (e.g. the window being maximized after it was laid out)
    // squeezed the open panel below its minimum width, give it its width back.
    aiSplitPane.addComponentListener(new java.awt.event.ComponentAdapter() {
      @Override
      public void componentResized(java.awt.event.ComponentEvent e) {
        if (aiPanelVisible && isShowing() && aiPanelSqueezed()) {
          SwingUtilities.invokeLater(() -> placeAiPanel());
        }
      }
    });
    box.add(aiSplitPane);

    // hopefully these are no longer needed w/ swing
    // (har har har.. that was wishful thinking)
    // listener = new EditorListener(this, textarea);
    pane.add(box);

    pane.setTransferHandler(new FileDropHandler());

    // Set the minimum size for the editor window
    setMinimumSize(scale(new Dimension(
        PreferencesData.getInteger("editor.window.width.min"),
        PreferencesData.getInteger("editor.window.height.min"))));

    // Bring back the general options for the editor
    applyPreferences();

    // Finish preparing Editor (formerly found in Base)
    pack();

    // Set the window bounds and the divider location before setting it visible
    setPlacement(storedLocation, defaultLocation);

    // Open the document that was passed in
    boolean loaded = handleOpenInternal(file);
    if (!loaded) sketchController = null;

    // default the console output to the last opened editor
    EditorConsole.setCurrentEditorConsole(console);

    if (PreferencesData.getBoolean(cc.arduino.ai.AiSettings.PREF_PANEL_VISIBLE, false)) {
      setAiPanelVisible(true);
    }
  }

  // 86Duino: AI assistant panel

  public boolean isAiPanelVisible() {
    return aiPanelVisible;
  }

  public void setAiPanelVisible(boolean visible) {
    if (visible == aiPanelVisible) {
      if (visible) {
        aiPanel.focusInput();
      }
      return;
    }
    aiPanelVisible = visible;
    PreferencesData.setBoolean(cc.arduino.ai.AiSettings.PREF_PANEL_VISIBLE, visible);
    if (visible) {
      // the API key may have been saved meanwhile (e.g. from another window)
      aiPanel.refreshSetupState();
      aiSplitPane.setRightComponent(aiPanel);
      aiSplitPane.setDividerSize(scale(6));
      if (isShowing()) {
        SwingUtilities.invokeLater(() -> placeAiPanel());
      } else {
        // The window is still being built (Base shows it later): its size is
        // not final yet, so place the panel once the window is on screen.
        addWindowListener(new WindowAdapter() {
          @Override
          public void windowOpened(WindowEvent e) {
            removeWindowListener(this);
            SwingUtilities.invokeLater(() -> placeAiPanel());
          }
        });
      }
    } else {
      aiSplitPane.setRightComponent(null);
      aiSplitPane.setDividerSize(0);
      getCurrentTab().requestFocusInWindow();
    }
    aiSplitPane.revalidate();
  }

  /** Toggles the panel from the Tools menu. */
  private void toggleAiPanel() {
    if (aiPanelVisible && aiPanelSqueezed()) {
      // marked open but not really visible: show it instead of hiding it
      placeAiPanel();
    } else {
      setAiPanelVisible(!aiPanelVisible);
    }
  }

  /** True when the open panel is narrower than its minimum width. */
  private boolean aiPanelSqueezed() {
    return aiSplitPane.getRightComponent() == aiPanel && aiPanel.getWidth() < aiPanel.getMinimumSize().width;
  }

  /** Gives the open panel its remembered width, widening the window if needed. */
  private void placeAiPanel() {
    if (!aiPanelVisible) {
      return;
    }
    int width = Math.max(scale(260), PreferencesData.getInteger(cc.arduino.ai.AiSettings.PREF_PANEL_WIDTH, scale(380)));
    widenWindowFor(width);
    validate();
    int location = aiSplitPane.getWidth() - width - aiSplitPane.getDividerSize();
    aiSplitPane.setDividerLocation(Math.max(splitPane.getMinimumSize().width, location));
    aiPanel.focusInput();
  }

  /**
   * Makes the window wide enough for the editor (at its minimum width) plus
   * the AI panel, without leaving the screen. A maximized window is kept.
   */
  private void widenWindowFor(int panelWidth) {
    if ((getExtendedState() & MAXIMIZED_HORIZ) != 0) {
      return;
    }
    int needed = splitPane.getMinimumSize().width + panelWidth + aiSplitPane.getDividerSize();
    int missing = needed - aiSplitPane.getWidth();
    if (missing <= 0) {
      return;
    }
    Rectangle screen = getGraphicsConfiguration().getBounds();
    java.awt.Insets insets = Toolkit.getDefaultToolkit().getScreenInsets(getGraphicsConfiguration());
    int screenLeft = screen.x + insets.left;
    int screenWidth = screen.width - insets.left - insets.right;
    int newWidth = Math.min(getWidth() + missing, screenWidth);
    int newX = Math.max(screenLeft, Math.min(getX(), screenLeft + screenWidth - newWidth));
    setBounds(newX, getY(), newWidth, getHeight());
    validate();
  }

  /** True while the AI is working, so the IDE does not ask to reload files it changes. */
  public boolean isAiBusy() {
    return aiPanel != null && aiPanel.isBusy();
  }

  /** The AI settings were saved in Preferences. */
  public void aiSettingsChanged() {
    if (aiPanel != null) {
      aiPanel.settingsChanged();
    }
  }

  /** Stops this window's Hermes process. Called when the window closes. */
  public void closeAiAssistant() {
    if (aiPanel != null) {
      aiPanel.closeAssistant();
    }
  }


  /**
   * Handles files dragged & dropped from the desktop and into the editor
   * window. Dragging files into the editor window is the same as using
   * "Sketch &rarr; Add File" for each file.
   */
  private class FileDropHandler extends TransferHandler {
    public boolean canImport(JComponent dest, DataFlavor[] flavors) {
      return true;
    }

    @SuppressWarnings("unchecked")
    public boolean importData(JComponent src, Transferable transferable) {
      int successful = 0;

      try {
        DataFlavor uriListFlavor =
          new DataFlavor("text/uri-list;class=java.lang.String");

        if (transferable.isDataFlavorSupported(DataFlavor.javaFileListFlavor)) {
          List<File> list = (List<File>)
            transferable.getTransferData(DataFlavor.javaFileListFlavor);
          for (File file : list) {
            if (sketchController.addFile(file)) {
              successful++;
            }
          }
        } else if (transferable.isDataFlavorSupported(uriListFlavor)) {
          // Some platforms (Mac OS X and Linux, when this began) preferred
          // this method of moving files.
          String data = (String)transferable.getTransferData(uriListFlavor);
          String[] pieces = PApplet.splitTokens(data, "\r\n");
          for (String piece : pieces) {
            if (piece.startsWith("#")) continue;

            String path = null;
            if (piece.startsWith("file:///")) {
              path = piece.substring(7);
            } else if (piece.startsWith("file:/")) {
              path = piece.substring(5);
            }
            if (sketchController.addFile(new File(path))) {
              successful++;
            }
          }
        }
      } catch (Exception e) {
        e.printStackTrace();
        return false;
      }

      if (successful == 0) {
        statusError(tr("No files were added to the sketch."));

      } else if (successful == 1) {
        statusNotice(tr("One file added to the sketch."));

      } else {
        statusNotice(I18n.format(tr("{0} files added to the sketch."), successful));
      }
      return true;
    }
  }

  protected void setPlacement(int[] storedLocation, int[] defaultLocation) {
    if (storedLocation.length > 5 && storedLocation[5] != 0) {
      setExtendedState(storedLocation[5]);
      setPlacement(defaultLocation);
    } else {
      setPlacement(storedLocation);
    }
  }

  private void setPlacement(int[] location) {
    setBounds(location[0], location[1], location[2], location[3]);
    if (location[4] != 0) {
      splitPane.setDividerLocation(location[4]);
    }
  }

  protected int[] getPlacement() {
    int[] location = new int[6];

    // Get the dimensions of the Frame
    Rectangle bounds = getBounds();
    location[0] = bounds.x;
    location[1] = bounds.y;
    location[2] = bounds.width;
    location[3] = bounds.height;

    // Get the current placement of the divider
    location[4] = splitPane.getDividerLocation();
    location[5] = getExtendedState() & MAXIMIZED_BOTH;

    return location;
  }


  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


  /**
   * Read and apply new values from the preferences, either because
   * the app is just starting up, or the user just finished messing
   * with things in the Preferences window.
   */
  public void applyPreferences() {
    boolean external = PreferencesData.getBoolean("editor.external");
    saveMenuItem.setEnabled(!external);
    saveAsMenuItem.setEnabled(!external);
    for (EditorTab tab: tabs) {
      tab.applyPreferences();
    }
    console.applyPreferences();
    if (serialMonitor != null) {
      serialMonitor.applyPreferences();
    }
    aiSettingsChanged();
  }


  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


  private void buildMenuBar() {
    JMenuBar menubar = new JMenuBar();
    final JMenu fileMenu = buildFileMenu();
    fileMenu.addMenuListener(new StubMenuListener() {
      @Override
      public void menuSelected(MenuEvent e) {
        List<Component> components = Arrays.asList(fileMenu.getMenuComponents());
        if (!components.contains(sketchbookMenu)) {
          fileMenu.insert(sketchbookMenu, 3);
        }
        if (!components.contains(examplesMenu)) {
          fileMenu.insert(examplesMenu, 4);
        }
        fileMenu.revalidate();
        validate();
      }
    });
    menubar.add(fileMenu);

    menubar.add(buildEditMenu());

    final JMenu sketchMenu = new JMenu(tr("Sketch"));
    sketchMenu.setMnemonic(KeyEvent.VK_S);
    sketchMenu.addMenuListener(new StubMenuListener() {

      @Override
      public void menuSelected(MenuEvent e) {
        buildSketchMenu(sketchMenu);
        sketchMenu.revalidate();
        validate();
      }
    });
    buildSketchMenu(sketchMenu);
    menubar.add(sketchMenu);

    toolsMenu = new JMenu(tr("Tools"));
    toolsMenu.setMnemonic(KeyEvent.VK_T);

    final JMenu toolsMenu = buildToolsMenu();
    toolsMenu.addMenuListener(new StubMenuListener() {
      @Override
      public void menuSelected(MenuEvent e) {
        List<Component> components = Arrays.asList(toolsMenu.getMenuComponents());
        int offset = 0;
        for (JMenu menu : base.getBoardsCustomMenus()) {
          if (!components.contains(menu)) {
            toolsMenu.insert(menu, numTools + offset);
            offset++;
          }
        }
        if (!components.contains(portMenu)) {
          toolsMenu.insert(portMenu, numTools + offset);
        }
        programmersMenu.removeAll();
        base.getProgrammerMenus().forEach(programmersMenu::add);
        toolsMenu.revalidate();
        validate();
      }
    });
    menubar.add(toolsMenu);

    menubar.add(buildHelpMenu());
    setJMenuBar(menubar);
  }


  private JMenu buildFileMenu() {
    JMenuItem item;
    fileMenu = new JMenu(tr("File"));
    fileMenu.setMnemonic(KeyEvent.VK_F);

    item = newJMenuItem(tr("New"), 'N');
    item.addActionListener(event -> {
      try {
        base.handleNew();
      } catch (Exception e1) {
        e1.printStackTrace();
      }
    });
    fileMenu.add(item);

    item = Editor.newJMenuItem(tr("Open..."), 'O');
    item.addActionListener(event -> {
      try {
        base.handleOpenPrompt();
      } catch (Exception e1) {
        e1.printStackTrace();
      }
    });
    fileMenu.add(item);

    base.rebuildRecentSketchesMenuItems();
    recentSketchesMenu = new JMenu(tr("Open Recent"));
    SwingUtilities.invokeLater(() -> rebuildRecentSketchesMenu());
    fileMenu.add(recentSketchesMenu);

    if (sketchbookMenu == null) {
      sketchbookMenu = new JMenu(tr("Sketchbook"));
      MenuScroller.setScrollerFor(sketchbookMenu);
      base.rebuildSketchbookMenu(sketchbookMenu);
    }
    fileMenu.add(sketchbookMenu);

    if (examplesMenu == null) {
      examplesMenu = new JMenu(tr("Examples"));
      MenuScroller.setScrollerFor(examplesMenu);
      base.rebuildExamplesMenu(examplesMenu);
    }
    fileMenu.add(examplesMenu);

    item = Editor.newJMenuItem(tr("Close"), 'W');
    item.addActionListener(event -> base.handleClose(Editor.this));
    fileMenu.add(item);

    saveMenuItem = newJMenuItem(tr("Save"), 'S');
    saveMenuItem.addActionListener(event -> handleSave(false));
    fileMenu.add(saveMenuItem);

    saveAsMenuItem = newJMenuItemShift(tr("Save As..."), 'S');
    saveAsMenuItem.addActionListener(event -> handleSaveAs());
    fileMenu.add(saveAsMenuItem);

    fileMenu.addSeparator();

    item = newJMenuItemShift(tr("Page Setup"), 'P');
    item.addActionListener(event -> handlePageSetup());
    fileMenu.add(item);

    item = newJMenuItem(tr("Print"), 'P');
    item.addActionListener(event -> handlePrint());
    fileMenu.add(item);

    // macosx already has its own preferences and quit menu
    if (!OSUtils.hasMacOSStyleMenus()) {
      fileMenu.addSeparator();

      item = newJMenuItem(tr("Preferences"), ',');
      item.addActionListener(event -> base.handlePrefs());
      fileMenu.add(item);

      // 86Duino
      appearanceMenu = new JMenu(tr("Appearance..."));

      JMenuItem sa = new JMenuItem(tr("Save Current Settings"));
      sa.addActionListener(evant -> saveAppearanceSettings());
      appearanceMenu.add(sa);

      JMenuItem re = new JMenuItem(tr("Reset Settings"));
      re.addActionListener(evant -> resetAppearanceSettings());
      appearanceMenu.add(re);

      fileMenu.add(appearanceMenu);

      fileMenu.addSeparator();

      item = newJMenuItem(tr("Quit"), 'Q');
      item.addActionListener(event -> base.handleQuit());
      fileMenu.add(item);
    }
    return fileMenu;
  }

  public void rebuildRecentSketchesMenu() {
    recentSketchesMenu.removeAll();
    for (JMenuItem recentSketchMenuItem  : base.getRecentSketchesMenuItems()) {
      recentSketchesMenu.add(recentSketchMenuItem);
    }
  }

  private void buildSketchMenu(JMenu sketchMenu) {
    sketchMenu.removeAll();

    JMenuItem item = newJMenuItem(tr("Verify/Compile"), 'R');
    item.addActionListener(event -> handleRun(false, presentHandler, runHandler));
    sketchMenu.add(item);

    item = newJMenuItem(tr("Upload"), 'U');
    item.addActionListener(event -> handleExport(false));
    sketchMenu.add(item);

    // if selected board is not 86Duino, hide the "Upload Using Programmer" option
    if (BaseNoGui.getTargetPlatform().getPreferences().get("name").indexOf("Vortex86")<0) {
        item = newJMenuItemShift(tr("Upload Using Programmer"), 'U');
        item.addActionListener(event -> handleExport(true));
        sketchMenu.add(item);
    }

    item = newJMenuItemAlt(tr("Export compiled Binary"), 'S');
    item.addActionListener(event -> {
      if (!(new CanExportInSketchFolder().test(sketchController))) {
        System.out.println(tr("Export canceled, changes must first be saved."));
        return;
      }
      handleRun(false, new CanExportInSketchFolder(), presentAndSaveHandler, runAndSaveHandler);

    });
    sketchMenu.add(item);

//    item = new JMenuItem("Stop");
//    item.addActionListener(event -> handleStop());
//    sketchMenu.add(item);

    sketchMenu.addSeparator();

    item = newJMenuItem(tr("Show Sketch Folder"), 'K');
    item.addActionListener(event -> Base.openFolder(sketch.getFolder()));
    sketchMenu.add(item);
    item.setEnabled(Base.openFolderAvailable());

    if (importMenu == null) {
      importMenu = new JMenu(tr("Include Library"));
      MenuScroller.setScrollerFor(importMenu);
      base.rebuildImportMenu(importMenu);
    }
    sketchMenu.add(importMenu);

    item = new JMenuItem(tr("Add File..."));
    item.addActionListener(event -> sketchController.handleAddFile());
    sketchMenu.add(item);
  }


  private JMenu buildToolsMenu() {

    addInternalTools(toolsMenu);
    
    JMenuItem item = newJMenuItemShift(tr("Manage Libraries..."), 'I');
    item.addActionListener(e -> base.openLibraryManager("", ""));
    toolsMenu.add(item);

    item = newJMenuItemShift(tr("Serial Monitor"), 'M');
    item.addActionListener(e -> handleSerial());
    toolsMenu.add(item);

    item = newJMenuItemShift(tr("Serial Plotter"), 'L');
    item.addActionListener(e -> handlePlotter());
    toolsMenu.add(item);

    // 86Duino
    item = newJMenuItemShift(tr("AI Assistant"), 'A');
    item.addActionListener(e -> toggleAiPanel());
    toolsMenu.add(item);

    if (BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QEC")) {
      if (globalToolItems.size() > 6) { // loading with high speed
        toolsMenu.addSeparator();
        item = globalToolItems.get("ArduBlock");
        if (item != null) {
          toolsMenu.add(item);
        }
        item = globalToolItems.get("86HMI Editor");
        if (item != null) {
          toolsMenu.add(item);
        }
        item = globalToolItems.get("86EVA (EtherCAT) Configurator");
        if (item != null) {
          toolsMenu.add(item);
        }
        item = globalToolItems.get("LDmicro");
        if (item != null) {
          toolsMenu.add(item);
        }
      } else { // loading with low speed
        addTools(toolsMenu, BaseNoGui.getToolsFolder(), "ArduBlock", true, 0, true); // 86Duino, specify name to sort, put it on UP location, and reverse sort
        addTools(toolsMenu, BaseNoGui.getToolsFolder(), "HMI", true, 0, false);
        addTools(toolsMenu, BaseNoGui.getToolsFolder(), "EcConfig", true, 0, false);
        addTools(toolsMenu, BaseNoGui.getToolsFolder(), "LDmicro", true, 0, false);
      }
    }
    
    File sketchbookTools = new File(BaseNoGui.getSketchbookFolder(), "tools");
    addTools(toolsMenu, sketchbookTools, "", true, 0, true); // 86Duino, all sketchbookTools accept, no reverse

    toolsMenu.addSeparator();

    numTools = toolsMenu.getItemCount();

    // XXX: DAM: these should probably be implemented using the Tools plugin
    // API, if possible (i.e. if it supports custom actions, etc.)

    base.getBoardsCustomMenus().stream().forEach(toolsMenu::add);

    if (portMenu == null)
      portMenu = new JMenu(tr("Port"));
    populatePortMenu();
    toolsMenu.add(portMenu);
    MenuScroller.setScrollerFor(portMenu);
    item = new JMenuItem(tr("Get Board Info"));
    item.addActionListener(e -> handleBoardInfo());
    toolsMenu.add(item);
    // toolsMenu.addSeparator();

    base.rebuildProgrammerMenu();
    programmersMenu = new JMenu(tr("Programmer"));
    MenuScroller.setScrollerFor(programmersMenu);
    base.getProgrammerMenus().stream().forEach(programmersMenu::add);
    if (BaseNoGui.getTargetPlatform().getPreferences().get("name").indexOf("Vortex86") < 0) {
      toolsMenu.add(programmersMenu);
    }

    // item = new JMenuItem(tr("Burn Bootloader"));
    // item.addActionListener(e -> handleBurnBootloader());
    // toolsMenu.add(item);

    // For 86Duino, only QEC
    if (BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QEC")) {
      if (globalToolItems.size() > 6) { // loading with high speed
        toolsMenu.addSeparator();
        item = globalToolItems.get("Upload All Resources");
        if (item != null) {
          toolsMenu.add(item);
        }
        item = globalToolItems.get("Download All Resources");
        if (item != null) {
          toolsMenu.add(item);
        }
        item = globalToolItems.get("Reset EtherCAT Master");
        if (item != null) {
          toolsMenu.add(item);
        }
      } else { // loading with low speed
        addTools(toolsMenu, BaseNoGui.getToolsFolder(), "AllRes", true, 1, true); // Download and Upload are here.
        addTools(toolsMenu, BaseNoGui.getToolsFolder(), "ResetECMaster", true, 0, false);
      }
    }
    
    if (BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QEC")) { // other plug-in, have "upload" in filename.
      addTools(toolsMenu, BaseNoGui.getToolsFolder(), "upload", true, 0, false);
    }
    
    // for 86Duino
    toolsMenu.addSeparator();
    burnbootloaderMenu = new JMenu(tr("Burn Bootloader"));
    rebuildburnbootloaderMenu(burnbootloaderMenu);
    toolsMenu.add(burnbootloaderMenu);
    
    // For 86Duino
    if (BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QEC")) {
      item = globalToolItems.get("Launch Bootloader Menu");
      if (item != null) {
        toolsMenu.add(item);
      } else {
        addTools(toolsMenu, BaseNoGui.getToolsFolder(), "BootMenu", true, 0, false); // specify name to show
      }
    }

    //If the selected borad is not QEC M01, remove this option.
    if (!BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QECM2")) {
      item = globalToolItems.get("Launch Bootloader Menu");
      if (item != null) toolsMenu.remove(item);
    }

    // For 86Duino
    updateToolsName();
    boardSeriesChanged();
    
    toolsMenu.addMenuListener(new StubMenuListener() {
      public JMenuItem getSelectedItemRecursive(JMenu menu) {
        int count = menu.getItemCount();
        for (int i=0; i < count; i++) {
          JMenuItem item = menu.getItem(i);

          if ((item instanceof JMenu))
            item = getSelectedItemRecursive((JMenu)item);

          if (item != null && item.isSelected())
            return item;
        }
        return null;
      }

      public void menuSelected(MenuEvent e) {
        //System.out.println("Tools menu selected.");
        populatePortMenu();
        for (Component c : toolsMenu.getMenuComponents()) {
          if ((c instanceof JMenu) && c.isVisible()) {
            JMenu menu = (JMenu)c;
            String name = menu.getText();
            if (name == null) continue;
            String basename = name;
            int index = name.indexOf(':');
            if (index > 0) basename = name.substring(0, index);

            JMenuItem item = getSelectedItemRecursive(menu);
            String sel = item != null ? item.getText() : null;
            if (sel == null) {
              if (!name.equals(basename)) menu.setText(basename);
            } else {
              if (sel.length() > 50) sel = sel.substring(0, 50) + "...";
              String newname = basename + ": \"" + sel + "\"";
              if (!name.equals(newname)) menu.setText(newname);
            }
          }
        }
      }
    });

    return toolsMenu;
  }
  
  private void rebuildToolsMenu() {
    toolsMenu.removeAll();
    buildToolsMenu();
  }

  // For 86Duino version
  protected void rebuildburnbootloaderMenu(JMenu menu) {
    if (BaseNoGui.getTargetPlatform().getPreferences().get("name").indexOf("Vortex86") >= 0) {
      String myBoardName = "";
      if (BaseNoGui.getTargetBoard().getPreferences().get("aliasname") != null)
        myBoardName = BaseNoGui.getTargetBoard().getPreferences().get("aliasname");
      else
        myBoardName = BaseNoGui.getTargetBoard().getPreferences().get("name");
      
      String firmwareVersion = "";
      File versionFile = base.getArduinoFile("hardware/86duino/x86/bootloaders/niitaka/fwver.v86");
      if (versionFile.exists())
      {
        try
        {
          InputStream is = new FileInputStream(versionFile);
          BufferedReader reader = new BufferedReader(new InputStreamReader(is, "UTF-8"));
          firmwareVersion = reader.readLine();
          reader.close();
        }
        catch (FileNotFoundException e)
        {
        }
        catch (UnsupportedEncodingException e)
        {
        }
        catch (IOException e)
        {
          e.printStackTrace();
        }
      }
      
      JMenuItem item = new JMenuItem(tr(myBoardName + " Firmware") + " " + firmwareVersion);
      item.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          handle86BurnBootloader(BaseNoGui.getTargetBoard().getPreferences().get("name")); // always use name, not alias name
        }
      });
      menu.add(item);
    }
    else
    {
      JMenuItem item = new JMenuItem(tr("Arduino Bootloader"));
      item.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          handleBurnBootloader();
        }
      });
      menu.add(item);
    }
  }

  private void addTools(JMenu menu, File sourceFolder, String startWith, boolean acceptOrNot, int sort, boolean addSeparator) { // 86Duino : String startWith, boolean acceptOrNot, int sort
    if (sourceFolder == null)
      return;

    Map<String, JMenuItem> toolItems = new HashMap<>();

    File[] folders = sourceFolder.listFiles(new FileFilter() {
      public boolean accept(File folder) {
        if (folder.isDirectory()) {
          //System.out.println("checking " + folder);
          File subfolder = new File(folder, "tool");
          return subfolder.exists();
        }
        return false;
      }
    });

    if (folders == null || folders.length == 0) {
      return;
    }

    for (File folder : folders) {
      File toolDirectory = new File(folder, "tool");

      try {
        // add dir to classpath for .classes
        //urlList.add(toolDirectory.toURL());

        // add .jar files to classpath
        File[] archives = toolDirectory.listFiles(new FilenameFilter() {
          public boolean accept(File dir, String name) { // 86Duino
            if ((name.toLowerCase().endsWith(".jar") || name.toLowerCase().endsWith(".zip"))) {
                if (!startWith.isEmpty()) {
                    if (acceptOrNot)
                        if (name.toLowerCase().contains(startWith.toLowerCase())) return true; else return false;
                    else // !acceptOrNot
                        if (name.toLowerCase().contains(startWith.toLowerCase())) return false; else return true;
                }
                return true;
            }
            return false;
          }
        });

        URL[] urlList = new URL[archives.length];
        for (int j = 0; j < urlList.length; j++) {
          urlList[j] = archives[j].toURI().toURL();
        }
        URLClassLoader loader = new URLClassLoader(urlList);

        String className = null;
        for (File archive : archives) {
          className = findClassInZipFile(folder.getName(), archive);
          if (className != null) break;
        }

        /*
        // Alternatively, could use manifest files with special attributes:
        // http://java.sun.com/j2se/1.3/docs/guide/jar/jar.html
        // Example code for loading from a manifest file:
        // http://forums.sun.com/thread.jspa?messageID=3791501
        File infoFile = new File(toolDirectory, "tool.txt");
        if (!infoFile.exists()) continue;

        String[] info = PApplet.loadStrings(infoFile);
        //Main-Class: org.poo.shoe.AwesomerTool
        //String className = folders[i].getName();
        String className = null;
        for (int k = 0; k < info.length; k++) {
          if (info[k].startsWith(";")) continue;

          String[] pieces = PApplet.splitTokens(info[k], ": ");
          if (pieces.length == 2) {
            if (pieces[0].equals("Main-Class")) {
              className = pieces[1];
            }
          }
        }
        */
        // If no class name found, just move on.
        if (className == null) continue;

        Class<?> toolClass = Class.forName(className, true, loader);
        final Tool tool = (Tool) toolClass.newInstance();

        tool.init(Editor.this);

        String title = tool.getMenuTitle();
        JMenuItem item = new JMenuItem(title);
        item.addActionListener(event -> {
          File file;
          String path = getSketchController().getSketch().getFolder().getAbsolutePath();
          if (title.equals("ArduBlock")) {
            file = new File(path + "/data/_ardublock.abp");
          } else if (title.equals("86HMI Editor")) {
            file = new File(path + "/data/_project.hpj");
          } else if (title.equals("86LDmicro")) {
            file = new File(path + "/data/myLD.ld");
          } else {
            file = new File(path + "/data/noname.xxx");
          }
          if (title.contains("Reset EtherCAT Master")) {
            SwingUtilities.invokeLater(tool);
          } else if (!getSketchController().isReadOnly() || file.exists()) {
            SwingUtilities.invokeLater(tool);
          } else {
            Base.showMessage(tr("Sketch is Read-Only"),
                       tr("Some files are marked \"read-only\", so you'll\n" +
                         "need to re-save the sketch in another location,\n" +
                         "and try again."));
            return;
          }
          //new Thread(tool).start();
        });
        //menu.add(item);
        toolItems.put(title, item);
        globalToolItems.put(title, item);
        if (title.equals("ArduBlock")) registerArduBlockTool(tool); // 86Duino
        else if (title.equals("86HMI Editor")) registerHMITool(tool); // 86Duino
        else if (title.equals("86EVA (EtherCAT) Configurator")) registerEVATool(tool); // 86Duino
        else if (title.equals("86LDmicro")) registerLDmicroTool(tool); // 86Duino

      } catch (Exception e) {
        e.printStackTrace();
      }
    }
    ArrayList<String> toolList = new ArrayList<>(toolItems.keySet());
    if (toolList.size() == 0) return;

    if (addSeparator) menu.addSeparator();
    
    if (sort == 0) Collections.sort(toolList);
    else if (sort == 1) Collections.sort(toolList, Collections.reverseOrder()); // 86Duino
    for (String title : toolList) {
      menu.add(toolItems.get(title));
    }
    return;
  }

  private void registerArduBlockTool(Tool tool) {
    ArduBlockTool = tool;
  }
  
  private void registerHMITool(Tool tool) {
    HMITool = tool;
  }
  
  private void registerEVATool(Tool tool) {
    EVATool = tool;
  }

  private void registerLDmicroTool(Tool tool) {
    LDmicroTool = tool;
  }
  
  public Tool getArduBlockTool() {
    return ArduBlockTool;
  }

  public Tool getLDmicroTool() {
    return LDmicroTool;
  }
  
  public Tool getHMITool() {
    return HMITool;
  }
  
  public Tool getEVATool() {
    return EVATool;
  }
  
  private String findClassInZipFile(String base, File file) {
    // Class file to search for
    String classFileName = "/" + base + ".class";

    ZipFile zipFile = null;
    try {
      zipFile = new ZipFile(file);
      Enumeration<?> entries = zipFile.entries();
      while (entries.hasMoreElements()) {
        ZipEntry entry = (ZipEntry) entries.nextElement();

        if (!entry.isDirectory()) {
          String name = entry.getName();
          //System.out.println("entry: " + name);

          if (name.endsWith(classFileName)) {
            //int slash = name.lastIndexOf('/');
            //String packageName = (slash == -1) ? "" : name.substring(0, slash);
            // Remove .class and convert slashes to periods.
            return name.substring(0, name.length() - 6).replace('/', '.');
          }
        }
      }
    } catch (IOException e) {
      //System.err.println("Ignoring " + filename + " (" + e.getMessage() + ")");
      e.printStackTrace();
    } finally {
      if (zipFile != null) {
        try {
           zipFile.close();
         } catch (IOException e) {
           // noop
         }
       }
     }
     return null;
   }

  public void updateKeywords(PdeKeywords keywords) {
    for (EditorTab tab : tabs)
      tab.updateKeywords(keywords);
  }

  JMenuItem createToolMenuItem(String className) {
    try {
      final Tool tool = getOrCreateToolInstance(className);

      JMenuItem item = new JMenuItem(tool.getMenuTitle());

      tool.init(Editor.this);

      item.addActionListener(event -> SwingUtilities.invokeLater(tool));
      return item;

    } catch (Exception e) {
      e.printStackTrace();
      return null;
    }
  }

  private Tool getOrCreateToolInstance(String className) {
    Tool internalTool = internalToolCache.get(className);
    if (internalTool == null) {
      try {
        Class<?> toolClass = Class.forName(className);
        internalTool = (Tool) toolClass.newInstance();
      } catch (Exception e) {
        e.printStackTrace();
        return null;
      }
      internalToolCache.put(className, internalTool);
    }
    return internalTool;
  }

  private void addInternalTools(JMenu menu) {
    JMenuItem item;

    item = createToolMenuItem("cc.arduino.packages.formatter.AStyle");
    if (item == null) {
      throw new NullPointerException("Tool cc.arduino.packages.formatter.AStyle unavailable");
    }
    item.setName("menuToolsAutoFormat");
    int modifiers = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
    item.setAccelerator(KeyStroke.getKeyStroke('T', modifiers));
    menu.add(item);

    //menu.add(createToolMenuItem("processing.app.tools.CreateFont"));
    //menu.add(createToolMenuItem("processing.app.tools.ColorSelector"));
    menu.add(createToolMenuItem("processing.app.tools.Archiver"));
    menu.add(createToolMenuItem("processing.app.tools.FixEncoding"));
  }


  private void selectSerialPort(String name) {
    if(portMenu == null) {
      System.out.println(tr("serialMenu is null"));
      return;
    }
    if (name == null) {
      System.out.println(tr("name is null"));
      return;
    }
    JCheckBoxMenuItem selection = null;
    for (int i = 0; i < portMenu.getItemCount(); i++) {
      JMenuItem menuItem = portMenu.getItem(i);
      if (!(menuItem instanceof JCheckBoxMenuItem)) {
        continue;
      }
      JCheckBoxMenuItem checkBoxMenuItem = ((JCheckBoxMenuItem) menuItem);
      checkBoxMenuItem.setState(false);
      if (name.equals(checkBoxMenuItem.getText())) selection = checkBoxMenuItem;
    }
    if (selection != null) selection.setState(true);
    //System.out.println(item.getLabel());

    BaseNoGui.selectSerialPort(name);
    try {
      boolean reopenMonitor = ((serialMonitor != null && serialMonitor.isVisible()) ||
                                serialPlotter != null && serialPlotter.isVisible());
      if (serialMonitor != null) {
        serialMonitor.close();
      }
      if (serialPlotter != null) {
        serialPlotter.close();
      }
      if (reopenMonitor) {
        handleSerial();
      }
    } catch (Exception e) {
      // ignore
    }

    onBoardOrPortChange();
    base.onBoardOrPortChange();

    //System.out.println("set to " + get("serial.port"));
  }

  class BoardPortJCheckBoxMenuItem extends JCheckBoxMenuItem {
    private BoardPort port;

    public BoardPortJCheckBoxMenuItem(BoardPort port) {
      super();
      this.port = port;
      setText(toString());
      addActionListener(e -> {
        selectSerialPort(port.getAddress());
        base.onBoardOrPortChange();
      });
    }

    @Override
    public String toString() {
      // This is required for serialPrompt()
      String label = port.getLabel();
      if (port.getBoardName() != null && !port.getBoardName().isEmpty()) {
        label += " (" + port.getBoardName() + ")";
      }
      return label;
    }
  }

  private void populatePortMenu() {
    final List<String> PROTOCOLS_ORDER = Arrays.asList("serial", "network");
    final List<String> PROTOCOLS_LABELS = Arrays.asList(tr("Serial ports"), tr("Network ports"));

    portMenu.removeAll();

    String selectedPort = PreferencesData.get("serial.port");

    List<BoardPort> ports = Base.getDiscoveryManager().discovery();

    ports = platform.filterPorts(ports, PreferencesData.getBoolean("serial.ports.showall"));

    ports.stream() //
        .filter(port -> port.getProtocolLabel() == null || port.getProtocolLabel().isEmpty())
        .forEach(port -> {
          int labelIdx = PROTOCOLS_ORDER.indexOf(port.getProtocol());
          if (labelIdx != -1) {
            port.setProtocolLabel(PROTOCOLS_LABELS.get(labelIdx));
          } else {
            port.setProtocolLabel(port.getProtocol());
          }
        });

    Collections.sort(ports, (port1, port2) -> {
      String pr1 = port1.getProtocol();
      String pr2 = port2.getProtocol();
      int prIdx1 = PROTOCOLS_ORDER.contains(pr1) ? PROTOCOLS_ORDER.indexOf(pr1) : 999;
      int prIdx2 = PROTOCOLS_ORDER.contains(pr2) ? PROTOCOLS_ORDER.indexOf(pr2) : 999;
      int r = prIdx1 - prIdx2;
      if (r != 0)
        return r;
      r = port1.getProtocolLabel().compareTo(port2.getProtocolLabel());
      if (r != 0)
        return r;
      return port1.getAddress().compareTo(port2.getAddress());
    });

    String lastProtocol = "";
    String lastProtocolLabel = "";
    for (BoardPort port : ports) {
      if (!port.getProtocol().equals(lastProtocol) || !port.getProtocolLabel().equals(lastProtocolLabel)) {
        if (!lastProtocol.isEmpty()) {
          portMenu.addSeparator();
        }
        lastProtocol = port.getProtocol();
        lastProtocolLabel = port.getProtocolLabel();
        JMenuItem item = new JMenuItem(tr(lastProtocolLabel));
        item.setEnabled(false);
        portMenu.add(item);
      }
      String address = port.getAddress();

      BoardPortJCheckBoxMenuItem item = new BoardPortJCheckBoxMenuItem(port);
      item.setSelected(address.equals(selectedPort));
      portMenu.add(item);
    }

    portMenu.setEnabled(portMenu.getMenuComponentCount() > 0);
  }


  private JMenu buildHelpMenu() {
    JMenu menu = new JMenu(tr("Help"));
    menu.setMnemonic(KeyEvent.VK_H);

    JMenuItem item = new JMenuItem(tr("Getting Started"));
    item.addActionListener(event -> Base.show86DuinoGettingStarted());
    menu.add(item);

    item = new JMenuItem(tr("Environment"));
    item.addActionListener(event -> Base.show86DuinoEnvironment());
    menu.add(item);

    item = new JMenuItem(tr("Troubleshooting"));
    item.addActionListener(event -> Base.show86DuinoForum());
    // menu.add(item);

    // item = new JMenuItem(tr("Reference"));
    // item.addActionListener(event -> Base.openURL("https://www.arduino.cc/reference/en/"));
    menu.add(item);

    menu.addSeparator();
    item = new JMenuItem(tr("Language Reference"));
    item.addActionListener(event -> Base.show86DuinoLanguageReference());
    menu.add(item);

    item = new JMenuItem(tr("Libraries Reference"));
    item.addActionListener(event -> Base.show86DuinoLibrariesference());
    menu.add(item); 

    item = new JMenuItem(tr("Hardware Reference"));
    item.addActionListener(event -> Base.show86DuinoHardwareference());
    menu.add(item);

    menu.addSeparator();
    if (BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QEC")) {
      item = new JMenuItem(tr("Visit qec.tw")); // show one when open IDE
    } else {
      item = new JMenuItem(tr("Visit 86duino.com")); // show one when open IDE
    }
    helpItem = item;
    item.addActionListener(event -> Base.show86DuinoWebLink());
    menu.add(item);

    // macosx already has its own about menu
    if (!OSUtils.hasMacOSStyleMenus()) {
      menu.addSeparator();
      item = new JMenuItem(tr("About 86Duino"));
      item.addActionListener(event -> base.handleAbout());
      menu.add(item);
    }

    return menu;
  }

  /**
   * Runs an Edit menu item. On macOS the screen menu bar also runs the item
   * when its shortcut is typed in another text field of this window (AI
   * Assistant, console): the field has done the action itself, so the code
   * must not change. A mouse click on the menu acts on that field instead,
   * or on the code when onField is null. Windows and Linux: always the code.
   */
  private void runEditAction(ActionEvent event, Runnable onCode, Consumer<JTextComponent> onField) {
    if (!OSUtils.isMacOS()) {
      onCode.run();
      return;
    }
    Component focus = KeyboardFocusManager.getCurrentKeyboardFocusManager().getPermanentFocusOwner();
    JTextComponent field = editMenuField(focus, this, getCurrentTab().getTextArea());
    if (field == null) {
      onCode.run();
    } else if ((event.getModifiers() & SHORTCUT_KEY_MASK) != 0) {
      // typed shortcut: the field already did it (Cmd+Z can even arrive twice)
    } else if (onField != null) {
      onField.accept(field);
    } else {
      onCode.run();
    }
  }

  /** The focused text field of this window other than the code area, or null. */
  static JTextComponent editMenuField(Component focus, Window window, Component codeArea) {
    if (focus instanceof JTextComponent && focus != codeArea
        && SwingUtilities.getWindowAncestor(focus) == window) {
      return (JTextComponent) focus;
    }
    return null;
  }

  private JMenu buildEditMenu() {
    JMenu menu = new JMenu(tr("Edit"));
    menu.setName("menuEdit");
    menu.setMnemonic(KeyEvent.VK_E);

    undoItem = newJMenuItem(tr("Undo"), 'Z');
    undoItem.setName("menuEditUndo");
    undoItem.addActionListener(event -> runEditAction(event, () -> getCurrentTab().handleUndo(), null));
    menu.add(undoItem);

    if (!OSUtils.isMacOS()) {
        redoItem = newJMenuItem(tr("Redo"), 'Y');
    } else {
        redoItem = newJMenuItemShift(tr("Redo"), 'Z');
    }
    redoItem.setName("menuEditRedo");
    redoItem.addActionListener(event -> runEditAction(event, () -> getCurrentTab().handleRedo(), null));
    menu.add(redoItem);

    menu.addSeparator();

    JMenuItem cutItem = newJMenuItem(tr("Cut"), 'X');
    cutItem.addActionListener(event -> runEditAction(event, () -> getCurrentTab().handleCut(), JTextComponent::cut));
    menu.add(cutItem);

    JMenuItem copyItem = newJMenuItem(tr("Copy"), 'C');
    copyItem.addActionListener(event -> runEditAction(event, () -> getCurrentTab().getTextArea().copy(), JTextComponent::copy));
    menu.add(copyItem);

    JMenuItem copyForumItem = newJMenuItemShift(tr("Copy for Forum"), 'C');
    copyForumItem.addActionListener(event -> getCurrentTab().handleDiscourseCopy());
    menu.add(copyForumItem);

    JMenuItem copyHTMLItem = newJMenuItemAlt(tr("Copy as HTML"), 'C');
    copyHTMLItem.addActionListener(event -> getCurrentTab().handleHTMLCopy());
    menu.add(copyHTMLItem);

    JMenuItem pasteItem = newJMenuItem(tr("Paste"), 'V');
    pasteItem.addActionListener(event -> runEditAction(event, () -> getCurrentTab().handlePaste(), JTextComponent::paste));
    menu.add(pasteItem);

    JMenuItem selectAllItem = newJMenuItem(tr("Select All"), 'A');
    selectAllItem.addActionListener(event -> runEditAction(event, () -> getCurrentTab().handleSelectAll(), JTextComponent::selectAll));
    menu.add(selectAllItem);

    JMenuItem gotoLine = newJMenuItem(tr("Go to line..."), 'L');
    gotoLine.addActionListener(event -> {
      GoToLineNumber goToLineNumber = new GoToLineNumber(Editor.this);
      goToLineNumber.setLocationRelativeTo(Editor.this);
      goToLineNumber.setVisible(true);
    });
    menu.add(gotoLine);

    menu.addSeparator();

    JMenuItem commentItem = newJMenuItem(tr("Comment/Uncomment"), PreferencesData.get("editor.keys.shortcut_comment", "/").charAt(0));
    commentItem.addActionListener(event -> getCurrentTab().handleCommentUncomment());
    menu.add(commentItem);

    JMenuItem increaseIndentItem = new JMenuItem(tr("Increase Indent"));
    increaseIndentItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_TAB, 0));
    increaseIndentItem.addActionListener(event -> getCurrentTab().handleIndentOutdent(true));
    menu.add(increaseIndentItem);

    JMenuItem decreseIndentItem = new JMenuItem(tr("Decrease Indent"));
    decreseIndentItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_TAB, InputEvent.SHIFT_MASK));
    decreseIndentItem.setName("menuDecreaseIndent");
    decreseIndentItem.addActionListener(event -> getCurrentTab().handleIndentOutdent(false));
    menu.add(decreseIndentItem);

    menu.addSeparator();

    JMenuItem increaseFontSizeItem = newJMenuItem(tr("Increase Font Size"), KeyEvent.VK_PLUS);
    increaseFontSizeItem.addActionListener(event -> base.handleFontSizeChange(1));
    menu.add(increaseFontSizeItem);
    // Many keyboards have '+' and '=' on the same key. Allowing "CTRL +",
    // "CTRL SHIFT +" and "CTRL =" covers the generally expected behavior.
    KeyStroke ctrlShiftEq = KeyStroke.getKeyStroke(KeyEvent.VK_EQUALS, SHORTCUT_KEY_MASK | ActionEvent.SHIFT_MASK);
    menu.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(ctrlShiftEq, "IncreaseFontSize");
    KeyStroke ctrlEq = KeyStroke.getKeyStroke(KeyEvent.VK_EQUALS, SHORTCUT_KEY_MASK);
    menu.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(ctrlEq, "IncreaseFontSize");
    menu.getActionMap().put("IncreaseFontSize", new AbstractAction() {
      @Override
      public void actionPerformed(ActionEvent e) {
        base.handleFontSizeChange(1);
      }
    });

    JMenuItem decreaseFontSizeItem = newJMenuItem(tr("Decrease Font Size"), KeyEvent.VK_MINUS);
    decreaseFontSizeItem.addActionListener(event -> base.handleFontSizeChange(-1));
    menu.add(decreaseFontSizeItem);

    menu.addSeparator();

    JMenuItem findItem = newJMenuItem(tr("Find..."), 'F');
    findItem.addActionListener(event -> {
      if (find == null) {
        find = new FindReplace(Editor.this, Base.FIND_DIALOG_STATE);
      }
      if (!OSUtils.isMacOS()) {
        find.setFindText(getCurrentTab().getSelectedText());
      }
      find.setLocationRelativeTo(Editor.this);
      find.setVisible(true);
    });
    menu.add(findItem);

    JMenuItem findNextItem = newJMenuItem(tr("Find Next"), 'G');
    findNextItem.addActionListener(event -> {
      if (find != null) {
        find.findNext();
      }
    });
    menu.add(findNextItem);

    JMenuItem findPreviousItem = newJMenuItemShift(tr("Find Previous"), 'G');
    findPreviousItem.addActionListener(event -> {
      if (find != null) {
        find.findPrevious();
      }
    });
    menu.add(findPreviousItem);

    if (OSUtils.isMacOS()) {
      JMenuItem useSelectionForFindItem = newJMenuItem(tr("Use Selection For Find"), 'E');
      useSelectionForFindItem.addActionListener(event -> {
        if (find == null) {
          find = new FindReplace(Editor.this, Base.FIND_DIALOG_STATE);
        }
        find.setFindText(getCurrentTab().getSelectedText());
      });
      menu.add(useSelectionForFindItem);
    }

    menu.addMenuListener(new MenuListener() {
      @Override
      public void menuSelected(MenuEvent e) {
        boolean enabled = getCurrentTab().getSelectedText() != null;
        cutItem.setEnabled(enabled);
        copyItem.setEnabled(enabled);
      }

      @Override
      public void menuDeselected(MenuEvent e) {}

      @Override
      public void menuCanceled(MenuEvent e) {}
    });

    return menu;
  }


  /**
   * A software engineer, somewhere, needs to have his abstraction
   * taken away. In some countries they jail or beat people for writing
   * the sort of API that would require a five line helper function
   * just to set the command key for a menu item.
   */
  static public JMenuItem newJMenuItem(String title, int what) {
    JMenuItem menuItem = new JMenuItem(title);
    menuItem.setAccelerator(KeyStroke.getKeyStroke(what, SHORTCUT_KEY_MASK));
    return menuItem;
  }


  /**
   * Like newJMenuItem() but adds shift as a modifier for the key command.
   */
  // Control + Shift + K seems to not be working on linux (Xubuntu 17.04, 2017-08-19)
  static public JMenuItem newJMenuItemShift(String title, int what) {
    JMenuItem menuItem = new JMenuItem(title);
    menuItem.setAccelerator(KeyStroke.getKeyStroke(what, SHORTCUT_KEY_MASK | ActionEvent.SHIFT_MASK));
    return menuItem;
  }


  /**
   * Same as newJMenuItem(), but adds the ALT (on Linux and Windows)
   * or OPTION (on Mac OS X) key as a modifier.
   */
  private static JMenuItem newJMenuItemAlt(String title, int what) {
    JMenuItem menuItem = new JMenuItem(title);
    menuItem.setAccelerator(KeyStroke.getKeyStroke(what, SHORTCUT_ALT_KEY_MASK));
    return menuItem;
  }


  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


  protected void updateUndoRedoState() {
    SketchTextArea textArea = getCurrentTab().getTextArea();
    undoItem.setEnabled(textArea.canUndo());
    redoItem.setEnabled(textArea.canRedo());
  }


  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


  // these will be done in a more generic way soon, more like:
  // setHandler("action name", Runnable);
  // but for the time being, working out the kinks of how many things to
  // abstract from the editor in this fashion.


  private void resetHandlers() {
    runHandler = new BuildHandler();
    presentHandler = new BuildHandler(true);
    runAndSaveHandler = new BuildHandler(false, true);
    presentAndSaveHandler = new BuildHandler(true, true);
    uploadHandler = new UploadHandler();
    uploadHandler.setUsingProgrammer(false);
    uploadUsingProgrammerHandler = new UploadHandler();
    uploadUsingProgrammerHandler.setUsingProgrammer(true);
    timeoutUploadHandler = new TimeoutUploadHandler();
    upload86EVAres = new Upload86EVARes();
    upload86EVAres.init(Editor.this);
  }


  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


  /**
   * Gets the current sketch controller.
   */
  public SketchController getSketchController() {
    return sketchController;
  }

  /**
   * Gets the current sketch.
   */
  public Sketch getSketch() {
    return sketch;
  }

  /**
   * Gets the currently displaying tab.
   */
  public EditorTab getCurrentTab() {
    return tabs.get(currentTabIndex);
  }

  /**
   * Gets the index of the currently displaying tab.
   */
  public int getCurrentTabIndex() {
    return currentTabIndex;
  }

  /**
   * Returns an (unmodifiable) list of currently opened tabs.
   */
  public List<EditorTab> getTabs() {
    return Collections.unmodifiableList(tabs);
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
  /**
   * Change the currently displayed tab.
   * Note that the GUI might not update immediately, since this needs
   * to run in the Event dispatch thread.
   * @param index The index of the tab to select
   */
  public void selectTab(final int index) {
    currentTabIndex = index;
    updateUndoRedoState();
    updateTitle();
    header.rebuild();
    getCurrentTab().activated();

    // This must be run in the GUI thread
    SwingUtilities.invokeLater(() -> {
      codePanel.removeAll();
      EditorTab selectedTab = tabs.get(index);
      codePanel.add(selectedTab, BorderLayout.CENTER);
      selectedTab.applyPreferences();
      selectedTab.requestFocusInWindow(); // get the caret blinking
      // For some reason, these are needed. Revalidate says it should be
      // automatically called when components are added or removed, but without
      // it, the component switched to is not displayed. repaint() is needed to
      // clear the entire text area of any previous text.
      codePanel.revalidate();
      codePanel.repaint();
    });
  }

  public void selectNextTab() {
    selectTab((currentTabIndex + 1) % tabs.size());
  }

  public void selectPrevTab() {
    selectTab((currentTabIndex - 1 + tabs.size()) % tabs.size());
  }

  public EditorTab findTab(final SketchFile file) {
    return tabs.get(findTabIndex(file));
  }

  /**
   * Finds the index of the tab showing the given file. Matches the file against
   * EditorTab.getSketchFile() using ==.
   *
   * @returns The index of the tab for the given file, or -1 if no such tab was
   *          found.
   */
  public int findTabIndex(final SketchFile file) {
    for (int i = 0; i < tabs.size(); ++i) {
      if (tabs.get(i).getSketchFile() == file)
        return i;
    }
    return -1;
  }

  /**
   * Finds the index of the tab showing the given file. Matches the file against
   * EditorTab.getSketchFile().getFile() using equals.
   *
   * @returns The index of the tab for the given file, or -1 if no such tab was
   *          found.
   */
  public int findTabIndex(final File file) {
    for (int i = 0; i < tabs.size(); ++i) {
      if (tabs.get(i).getSketchFile().getFile().equals(file))
        return i;
    }
    return -1;
  }

  /**
   * Create tabs for each of the current sketch's files, removing any existing
   * tabs.
   */
  public void createTabs() {
    tabs.clear();
    currentTabIndex = -1;
    tabs.ensureCapacity(sketch.getCodeCount());
    for (SketchFile file : sketch.getFiles()) {
      try {
        // For new tabs that have not been saved to disk yet, keep their
        // current in-memory text instead of trying to (re)load from a file
        // that does not exist. Passing null would make EditorTab call
        // file.load(), which throws for a non-existent file and would lose
        // the user's unsaved new tab.
        String contents = file.isNewUnsaved() ? file.getProgram() : null;
        addTab(file, contents);
      } catch(IOException e) {
        // TODO: Improve / move error handling
        System.err.println(e);
      }
    }
    selectTab(0);
  }

  /**
   * Reorders tabs as per current sketch's files order
   */
  public void reorderTabs() {
    Collections.sort(tabs, (x, y) -> Sketch.CODE_DOCS_COMPARATOR.compare(x.getSketchFile(), y.getSketchFile()));
  }

  /**
   * Add a new tab.
   *
   * @param file
   *          The file to show in the tab.
   * @param contents
   *          The contents to show in the tab, or null to load the contents from
   *          the given file.
   * @throws IOException
   */
  protected void addTab(SketchFile file, String contents) throws IOException {
    EditorTab tab = new EditorTab(this, file, contents);
    tab.getTextArea().getDocument()
        .addDocumentListener(new DocumentTextChangeListener(
            () -> updateUndoRedoState()));
    tabs.add(tab);
    reorderTabs();
  }

  protected void removeTab(SketchFile file) throws IOException {
    int index = findTabIndex(file);
    tabs.remove(index);
  }


  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


  void handleFindReference(String text) {
    String referenceFile = base.getPdeKeywords().getReference(text);
    String q;
    if (referenceFile == null) {
      q = text;
    } else if (referenceFile.startsWith("Serial_")) {
      q = referenceFile.substring(7);
    } else {
      q = referenceFile;
    }
    try {
      Base.openURL("https://www.arduino.cc/search?tab=&q="
                   + URLEncoder.encode(q, "UTF-8"));
    } catch (UnsupportedEncodingException e) {
      e.printStackTrace();
    }
  }


  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


  /**
   * Implements Sketch &rarr; Run.
   * @param verbose Set true to run with verbose output.
   * @param verboseHandler
   * @param nonVerboseHandler
   */
  public void handleRun(final boolean verbose, Runnable verboseHandler, Runnable nonVerboseHandler) {
    handleRun(verbose, new ShouldSaveIfModified(), verboseHandler, nonVerboseHandler);
  }

  private void handleRun(final boolean verbose, Predicate<SketchController> shouldSavePredicate, Runnable verboseHandler, Runnable nonVerboseHandler) {
    if (shouldSavePredicate.test(sketchController)) {
      handleSave(true);
    }
    toolbar.activateRun();
    status.progress(tr("Compiling sketch..."));

    // do this to advance/clear the terminal window / dos prompt / etc
    for (int i = 0; i < 10; i++) System.out.println();

    // clear the console on each run, unless the user doesn't want to
    if (PreferencesData.getBoolean("console.auto_clear")) {
      console.clear();
    }

    // Cannot use invokeLater() here, otherwise it gets
    // placed on the event thread and causes a hang--bad idea all around.
    new Thread(verbose ? verboseHandler : nonVerboseHandler).start();
  }

  class BuildHandler implements Runnable {

    private final boolean verbose;
    private final boolean saveHex;

    public BuildHandler() {
      this(false);
    }

    public BuildHandler(boolean verbose) {
      this(verbose, false);
    }

    public BuildHandler(boolean verbose, boolean saveHex) {
      this.verbose = verbose;
      this.saveHex = saveHex;
    }

    @Override
    public void run() {
      try {
        removeAllLineHighlights();
        sketchController.build(verbose, saveHex);
        statusNotice(tr("Done compiling."));
      } catch (PreferencesMapException e) {
        statusError(I18n.format(
                tr("Error while compiling: missing '{0}' configuration parameter"),
                e.getMessage()));
      } catch (Exception e) {
        status.unprogress();
        statusError(e);
      }

      status.unprogress();
      toolbar.deactivateRun();
      avoidMultipleOperations = false;
    }
  }

  public void removeAllLineHighlights() {
    for (EditorTab tab : tabs)
      tab.getTextArea().removeAllLineHighlights();
  }

  public void addLineHighlight(int line) throws BadLocationException {
    SketchTextArea textArea = getCurrentTab().getTextArea();
    FoldManager foldManager = textArea.getFoldManager();
    if (foldManager.isLineHidden(line)) {
      for (int i = 0; i < foldManager.getFoldCount(); i++) {
        if (foldManager.getFold(i).containsLine(line)) {
          foldManager.getFold(i).setCollapsed(false);
        }
      }
    }
    textArea.addLineHighlight(line, new Color(1, 0, 0, 0.2f));
    textArea.setCaretPosition(textArea.getLineStartOffset(line));
  }


  /**
   * Implements Sketch &rarr; Stop, or pressing Stop on the toolbar.
   */
  private void handleStop() {  // called by menu or buttons
//    toolbar.activate(EditorToolbar.STOP);

    toolbar.deactivateRun();
//    toolbar.deactivate(EditorToolbar.STOP);

    // focus the PDE again after quitting presentation mode [toxi 030903]
    toFront();
  }

  /**
   * Check if the sketch is modified and ask user to save changes.
   * @return false if canceling the close/quit operation
   */
  protected boolean checkModified() {
    if (!sketch.isModified())
      return true;

    // As of Processing 1.0.10, this always happens immediately.
    // http://dev.processing.org/bugs/show_bug.cgi?id=1456

    toFront();

    String prompt = I18n.format(tr("Save changes to \"{0}\"?  "),
                                sketch.getName());

    if (!OSUtils.hasMacOSStyleMenus()) {
      int result =
        JOptionPane.showConfirmDialog(this, prompt, tr("Close"),
                                      JOptionPane.YES_NO_CANCEL_OPTION,
                                      JOptionPane.QUESTION_MESSAGE);

      switch (result) {
        case JOptionPane.YES_OPTION:
          return handleSave(true);
        case JOptionPane.NO_OPTION:
          return true;  // ok to continue
        case JOptionPane.CANCEL_OPTION:
        case JOptionPane.CLOSED_OPTION:  // Escape key pressed
          return false;
        default:
          throw new IllegalStateException();
      }

    } else {
      // This code is disabled unless Java 1.5 is being used on Mac OS X
      // because of a Java bug that prevents the initial value of the
      // dialog from being set properly (at least on my MacBook Pro).
      // The bug causes the "Don't Save" option to be the highlighted,
      // blinking, default. This sucks. But I'll tell you what doesn't
      // suck--workarounds for the Mac and Apple's snobby attitude about it!
      // I think it's nifty that they treat their developers like dirt.

      JOptionPane pane =
        new JOptionPane(tr("<html> " +
                          "<head> <style type=\"text/css\">"+
                          "b { font: 13pt \"Lucida Grande\" }"+
                          "p { font: 11pt \"Lucida Grande\"; margin-top: 8px }"+
                          "</style> </head>" +
                          "<b>Do you want to save changes to this sketch<BR>" +
                          " before closing?</b>" +
                          "<p>If you don't save, your changes will be lost."),
                        JOptionPane.QUESTION_MESSAGE);

      String[] options = new String[] {
        tr("Save"), tr("Cancel"), tr("Don't Save")
      };
      pane.setOptions(options);

      // highlight the safest option ala apple hig
      pane.setInitialValue(options[0]);

      JDialog dialog = pane.createDialog(this, null);
      dialog.setVisible(true);

      Object result = pane.getValue();
      if (result == options[0]) {  // save (and close/quit)
        return handleSave(true);
      } else {
        return result == options[2];
      }
    }
  }

  /**
   * Second stage of open, occurs after having checked to see if the
   * modifications (if any) to the previous sketch need to be saved.
   */
  protected boolean handleOpenInternal(File sketchFile) {
    // check to make sure that this .pde file is
    // in a folder of the same name
    String fileName = sketchFile.getName();

    File file = Sketch.checkSketchFile(sketchFile);

    if (file == null) {
      if (!fileName.endsWith(".ino") && !fileName.endsWith(".pde")) {

        Base.showWarning(tr("Bad file selected"), tr("86Duino can only open its own sketches\n" +
          "and other files ending in .ino or .pde"), null);
        return false;

      } else {
        String properParent = fileName.substring(0, fileName.length() - 4);

        Object[] options = {tr("OK"), tr("Cancel")};
        String prompt = I18n.format(tr("The file \"{0}\" needs to be inside\n" +
            "a sketch folder named \"{1}\".\n" +
            "Create this folder, move the file, and continue?"),
          fileName,
          properParent);

        int result = JOptionPane.showOptionDialog(this, prompt, tr("Moving"), JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE, null, options, options[0]);

        if (result != JOptionPane.YES_OPTION) {
          return false;
        }

        // create properly named folder
        File properFolder = new File(sketchFile.getParent(), properParent);
        if (properFolder.exists()) {
          Base.showWarning(tr("Error"), I18n.format(tr("A folder named \"{0}\" already exists. " +
            "Can't open sketch."), properParent), null);
          return false;
        }
        if (!properFolder.mkdirs()) {
          //throw new IOException("Couldn't create sketch folder");
          Base.showWarning(tr("Error"), tr("Could not create the sketch folder."), null);
          return false;
        }
        // copy the sketch inside
        File properPdeFile = new File(properFolder, sketchFile.getName());
        try {
          Base.copyFile(sketchFile, properPdeFile);
        } catch (IOException e) {
          Base.showWarning(tr("Error"), tr("Could not copy to a proper location."), e);
          return false;
        }

        // remove the original file, so user doesn't get confused
        sketchFile.delete();

        // update with the new path
        file = properPdeFile;

      }
    }

    try {
      sketch = new Sketch(file);
    } catch (IOException e) {
      Base.showWarning(tr("Error"), tr("Could not create the sketch."), e);
      return false;
    }
    sketchController = new SketchController(this, sketch);
    createTabs();

    // Disable untitled setting from previous document, if any
    untitled = false;

    // opening was successful
    return true;
  }

  public static String processCompileFile(String directoryPath) {

    Path filePath = Paths.get(directoryPath, "lib/version_name.txt");

    try (BufferedReader reader = Files.newBufferedReader(filePath)) {
      String firstLine = reader.readLine();
      String secondLine = reader.readLine();

      if (firstLine == null) {
          return "";
      }
      if (secondLine == null) {
          secondLine = "";
      }

      if (firstLine.toLowerCase().contains("beta")) {
          return secondLine + firstLine;
      } else {
          return secondLine;
      }

    } catch (IOException e) {
      System.err.println("Fail to read lib/version_name.txt" + e.getMessage());
      return "Error: Can not read file.";
    }
  }

  public void updateTitle() {
    if (sketchController == null) {
      return;
    }
    SketchFile current = getCurrentTab().getSketchFile();
    String customFormat = PreferencesData.get("editor.custom_title_format");
    if (customFormat != null && !customFormat.trim().isEmpty()) {
      Map<String, String> titleMap = new HashMap<String, String>();
      titleMap.put("file", current.getFileName());
      String path = sketch.getFolder().getAbsolutePath();
      titleMap.put("folder", path);
      titleMap.put("path", path);
      titleMap.put("project", sketch.getName());
      titleMap.put("version", tr(BaseNoGui.VERSION_NAME_LONG));

      setTitle(StringReplacer.replaceFromMapping(customFormat, titleMap));
    } else {
      // Read lib/version_name.txt from the IDE folder (APP_DIR), not the working
      // directory: a macOS app started from Finder runs in "/".
      String MyVersionName = processCompileFile(BaseNoGui.getContentFile("").getPath());
      if ("Error: Can not read file.".equals(MyVersionName)) {
        MyVersionName = BaseNoGui.VERSION_NAME_LONG;
      }

      if (current.isPrimary()) {
        setTitle(I18n.format(tr("{0} | 86Duino {1}"), sketch.getName(),
                             tr(MyVersionName)));
      } else {
        setTitle(I18n.format(tr("{0} - {1} | 86Duino {2}"), sketch.getName(),
                             current.getFileName(), tr(MyVersionName)));
      }
    }
  }


  /**
   * Actually handle the save command. If 'immediately' is set to false,
   * this will happen in another thread so that the message area
   * will update and the save button will stay highlighted while the
   * save is happening. If 'immediately' is true, then it will happen
   * immediately. This is used during a quit, because invokeLater()
   * won't run properly while a quit is happening. This fixes
   * <A HREF="http://dev.processing.org/bugs/show_bug.cgi?id=276">Bug 276</A>.
   */
  public boolean handleSave(boolean immediately) {
    //stopRunner();
    handleStop();  // 0136
    removeAllLineHighlights();

    if (untitled) {
      return handleSaveAs();
      // need to get the name, user might also cancel here

    } else if (immediately) {
      return handleSave2();

    } else {
      SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            handleSave2();
          }
        });
    }
    return true;
  }


  private boolean handleSave2() {
    toolbar.activateSave();
    statusNotice(tr("Saving..."));
    boolean saved = false;
    try {
      if (PreferencesData.getBoolean("editor.autoformat_currentfile_before_saving")) {
        Tool formatTool = getOrCreateToolInstance("cc.arduino.packages.formatter.AStyle");
        formatTool.run();
      }

      boolean wasReadOnly = sketchController.isReadOnly();
      String previousMainFilePath = sketch.getMainFilePath();
      saved = sketchController.save();
      if (saved) {
        statusNotice(tr("Done Saving."));
        if (wasReadOnly) {
          base.removeRecentSketchPath(previousMainFilePath);
        }
        base.storeRecentSketches(sketchController);
        base.rebuildRecentSketchesMenuItems();
      } else {
        statusEmpty();
      }
      // rebuild sketch menu in case a save-as was forced
      // Disabling this for 0125, instead rebuild the menu inside
      // the Save As method of the Sketch object, since that's the
      // only one who knows whether something was renamed.
      //sketchbook.rebuildMenus();
      //sketchbook.rebuildMenusAsync();

    } catch (Exception e) {
      // show the error as a message in the window
      statusError(e);

      // zero out the current action,
      // so that checkModified2 will just do nothing
      //checkModifiedMode = 0;
      // this is used when another operation calls a save
    }
    //toolbar.clear();
    toolbar.deactivateSave();
    return saved;
  }


  public boolean handleSaveAs() {
    //stopRunner();  // formerly from 0135
    handleStop();

    toolbar.activateSave();

    //SwingUtilities.invokeLater(new Runnable() {
    //public void run() {
    statusNotice(tr("Saving..."));
    try {
      if (sketchController.saveAs()) {
        base.storeRecentSketches(sketchController);
        base.rebuildRecentSketchesMenuItems();
        statusNotice(tr("Done Saving."));
        // 86Duino -- start to delete prev sketch file including ino.cpp, .o, .d
        String buildSketchPath = sketch.getBuildPath().getAbsolutePath() + "/sketch";
        File sketchDir = new File(buildSketchPath);
        if (sketchDir.exists() && sketchDir.isDirectory()) {
            File[] listFiles = sketchDir.listFiles();
            for (File sf : listFiles)
                sf.delete();
        }
        // 86Duino -- end
        
        // Disabling this for 0125, instead rebuild the menu inside
        // the Save As method of the Sketch object, since that's the
        // only one who knows whether something was renamed.
        //sketchbook.rebuildMenusAsync();
      } else {
        statusNotice(tr("Save Canceled."));
        return false;
      }
    } catch (Exception e) {
      // show the error as a message in the window
      statusError(e);

    } finally {
      // make sure the toolbar button deactivates
      toolbar.deactivateSave();

      // Update editor window title in case of "Save as..."
      updateTitle();
      header.rebuild();
    }

    return true;
  }


  private boolean serialPrompt() {
    List<BoardPortJCheckBoxMenuItem> items = new ArrayList<>();
    for (int i = 0; i < portMenu.getItemCount(); i++) {
      if (portMenu.getItem(i) instanceof BoardPortJCheckBoxMenuItem)
        items.add((BoardPortJCheckBoxMenuItem) portMenu.getItem(i));
    }

    String port = PreferencesData.get("serial.port");
    String title;
    if (port == null || port.isEmpty()) {
      title = tr("Serial port not selected.");
    } else {
      title = I18n.format(tr("Serial port {0} not found."), port);
    }
    String question = tr("Retry the upload with another serial port?");
    BoardPortJCheckBoxMenuItem result = (BoardPortJCheckBoxMenuItem) JOptionPane
        .showInputDialog(this, title + "\n" + question, title,
                         JOptionPane.PLAIN_MESSAGE, null, items.toArray(), 0);
    if (result == null)
      return false;
    result.doClick();
    base.onBoardOrPortChange();
    return true;
  }

  /**
   * Called by Sketch &rarr; Export.
   * Handles calling the export() function on sketch, and
   * queues all the gui status stuff that comes along with it.
   * <p/>
   * Made synchronized to (hopefully) avoid problems of people
   * hitting export twice, quickly, and horking things up.
   */
  /**
   * Handles calling the export() function on sketch, and
   * queues all the gui status stuff that comes along with it.
   *
   * Made synchronized to (hopefully) avoid problems of people
   * hitting export twice, quickly, and horking things up.
   */
  synchronized public void handleExport(final boolean usingProgrammer) {
    if (PreferencesData.getBoolean("editor.save_on_verify")) {
      if (sketch.isModified() && !sketchController.isReadOnly()) {
        handleSave(true);
      }
    }
    toolbar.activateExport();
    console.clear();
    status.progress(tr("Uploading to I/O Board..."));

    avoidMultipleOperations = true;

    new Thread(timeoutUploadHandler).start();
    new Thread(usingProgrammer ? uploadUsingProgrammerHandler : uploadHandler).start();
  }

  class UploadHandler implements Runnable {
    boolean usingProgrammer = false;

    public void setUsingProgrammer(boolean usingProgrammer) {
      this.usingProgrammer = usingProgrammer;
    }

    public void run() {
      try {
        uploading = true;

        removeAllLineHighlights();
        if (serialMonitor != null) {
          serialMonitor.suspend();
        }
        if (serialPlotter != null) {
          serialPlotter.suspend();
        }

        boolean success = sketchController.exportApplet(usingProgrammer);
        if (success) {
          statusNotice(tr("Done uploading."));
        }
      } catch (SerialNotFoundException e) {
        if (portMenu.getItemCount() == 0) {
          statusError(tr("Serial port not selected."));
        } else {
          if (serialPrompt()) {
            run();
          } else {
            statusNotice(tr("Upload canceled."));
          }
        }
      } catch (PreferencesMapException e) {
        statusError(I18n.format(
                    tr("Error while uploading: missing '{0}' configuration parameter"),
                    e.getMessage()));
      } catch (RunnerException e) {
        //statusError("Error during upload.");
        //e.printStackTrace();
        status.unprogress();
        statusError(e);
      } catch (Exception e) {
        e.printStackTrace();
      } finally {
        populatePortMenu();
        avoidMultipleOperations = false;
      }
      status.unprogress();
      uploading = false;
      //toolbar.clear();
      toolbar.deactivateExport();

      resumeOrCloseSerialMonitor();
      resumeOrCloseSerialPlotter();
      base.onBoardOrPortChange();
    }
  }

  static public boolean isUploading() {
    return uploading;
  }

  public void resumeOrCloseSerialMonitor() {
    // Return the serial monitor window to its initial state
    if (serialMonitor != null) {
      try {
        Thread.sleep(200);
      } catch (InterruptedException e) {
          // noop
      }
      BoardPort boardPort = BaseNoGui.getDiscoveryManager().find(PreferencesData.get("serial.port"));
      long sleptFor = 0;
      while (boardPort == null && sleptFor < MAX_TIME_AWAITING_FOR_RESUMING_SERIAL_MONITOR) {
        try {
          Thread.sleep(100);
          sleptFor += 100;
          boardPort = BaseNoGui.getDiscoveryManager().find(PreferencesData.get("serial.port"));
        } catch (InterruptedException e) {
          // noop
        }
      }
      try {
        if (serialMonitor != null) {
          serialMonitor.resume(boardPort);
          if (boardPort == null) {
            serialMonitor.close();
            handleSerial();
          } else {
            serialMonitor.resume(boardPort);
          }
        }
      } catch (Exception e) {
        statusError(e);
      }
   }
  }

  public void resumeOrCloseSerialPlotter() {
    // Return the serial plotter window to its initial state
    if (serialPlotter != null) {
      BoardPort boardPort = BaseNoGui.getDiscoveryManager().find(PreferencesData.get("serial.port"));
      try {
        if (serialPlotter != null)
          serialPlotter.resume(boardPort);
        if (boardPort == null) {
          serialPlotter.close();
          handlePlotter();
        } else {
          serialPlotter.resume(boardPort);
        }
      } catch (Exception e) {
        statusError(e);
      }
   }
  }

  class TimeoutUploadHandler implements Runnable {

    public void run() {
      try {
        //10 seconds, than reactivate upload functionality and let the programmer pid being killed
        Thread.sleep(1000 * 10);
        if (uploading) {
          avoidMultipleOperations = false;
        }
      } catch (InterruptedException e) {
          // noop
      }
    }
  }

  // 86Duino Start ---
  class Upload86EVARes implements Runnable {
    Editor editor;

    private int listenOnProcess(String[] arguments) {
      try {
        final Process p = ProcessUtils.exec(arguments);
        
        // stdout
        Thread thread1 = new Thread() {
          public void run() {
            try {
              int c;
              InputStreamReader reader = new InputStreamReader(p.getInputStream());
              while ((c = reader.read()) != -1)
                  System.out.print((char) c);
              reader.close();
            } catch (Exception e){}
          }
        };
        
        //stderr
        Thread thread2 = new Thread() {
          public void run() {
            try {
              int c;
              InputStreamReader reader = new InputStreamReader(p.getErrorStream());
              while ((c = reader.read()) != -1)
                  System.err.print((char) c);
              reader.close();
            } catch (Exception e){}
          }
        };
        
        thread1.start();
        thread2.start();
        int res = p.waitFor();
        thread1.join();
        thread2.join();
        return res;
      } catch (Exception e){
        return -1;
      }
    }

    private void sysExec(final String[] arguments){
        Thread thread = new Thread() {
            public void run() {
                try {
                    if(listenOnProcess(arguments) != 0) {
                        editor.statusError("Bootloader Upload failed!");
                    } else {
                        editor.statusNotice("Uplaoding Bootloader ... Done");
                    }
                } catch (Exception e){
                    editor.statusError("fail to run v86dude!");
                }
            }
        };
        thread.start();
    }

    private void copyFileUsingJava7Files(File source, File dest) throws IOException {   
        Files.copy(source.toPath(), dest.toPath());
    }
    
    private void clearAllFile(File destFolder) throws IOException {
        File[] files = destFolder.listFiles();
        
        for (File file : files) {
          if (!file.isDirectory()) {
            if (file.getName().indexOf(".png") != -1) file.delete();
                    else if (file.getName().indexOf(".PNG") != -1) file.delete();
                    else if (file.getName().indexOf(".txt") != -1) file.delete();
                    else if (file.getName().indexOf("v86boot") != -1) file.delete();
                    else if (file.getName().indexOf("86eva") != -1) file.delete();
                    else if (file.getName().indexOf("_fecfg") != -1) file.delete();
                    else if (file.getName().indexOf("_fblui") != -1) file.delete();
                    else if (file.getName().indexOf(".v86") != -1) file.delete();
          } else {
            clearAllFile(file);
          }
        }
    }
    
    private void copyResourceTo86EVA(File source, File target) throws IOException {
        for (File file : source.listFiles()) {
          if (!file.isDirectory()) {
                    File srcDir  = new File(source.getAbsolutePath() + "/" + file.getName());
                File destDir = new File(target.getAbsolutePath() + "/" + file.getName());
            copyFileUsingJava7Files(srcDir, destDir);
          } else {
            // do nothing
          }
        }
    }

    private Map<String, String> _86boardparser;

    private void parser86Board() {
      _86boardparser = new HashMap<>();
      String path = BaseNoGui.getHardwarePath() + "/86duino/x86/variants/boards.txt";
      try(BufferedReader in = new BufferedReader(new FileReader(path))) {
          String str;
          while ((str = in.readLine()) != null) {
              String[] tokens = str.split(",");
              _86boardparser.put(tokens[0], tokens[1]);
          }
          in.close();
      }
      catch (IOException e) {
          System.err.println(path + ": File Read Error\n");
      }
    }

    private String get86BoardString(String _86board) {
      if (_86boardparser.containsKey(_86board))
        return _86boardparser.get(_86board);
      else
        return _86board;
    }

    public void init(Editor editor) {
      if (this.editor == null) {
        this.editor = editor;
      }
    }
    
    public void run() {
      // !!!!!!!!!!!!!if re-start HMI editor
      
      String programName = "v86dude.exe";
      String dudeDir = BaseNoGui.getHardwarePath() + "/tools/x86";

      File tool = new File(dudeDir, programName);
      if (!tool.exists()) {
        editor.statusError("Openfile Error: v86dude.exe not found!");
        return;
      } else {
        String boardName1 = BaseNoGui.getTargetBoard().getPreferences().get("name").substring(8);
        
        if (!boardName1.contains("QEC") && !boardName1.contains("CYF")) {
            editor.statusError("Need the PL2303 chip on board. (EX. QEC series)");
            return;
        }

        parser86Board();

        String boardName = get86BoardString(BaseNoGui.getTargetBoard().getPreferences().get("name"));
        
        // M2 need the many special steps
        String rpath      = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka";
        String destPath   = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/86eva";
        String srcPath    = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/resource"; // PNG files
        String src1Path   = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/" + boardName.toLowerCase() + "/86eva.exe";
        String src2Path   = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/" + boardName.toLowerCase() + "/v86boot.exe";
        String src3Path   = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/" + boardName.toLowerCase() + "/_bldver.v86";
        String src4Path   = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/" + boardName.toLowerCase() + "/_fecfg.exe";
        String src5Path   = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/" + boardName.toLowerCase() + "/_fblui.exe";
        String src6Path   = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/fwver.v86";
        String dest1Path  = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/86eva/86eva.exe";
        String dest2Path  = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/86eva/v86boot.exe";
        String dest3Path  = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/86eva/_bldver.v86";
        String dest4Path  = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/86eva/_fecfg.exe";
        String dest5Path  = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/86eva/_fblui.exe";
        String dest6Path  = BaseNoGui.getHardwarePath() + "/86duino/x86/bootloaders/niitaka/86eva/fwver.v86";

        File srcFolder    = new File(srcPath);
        File destFolder   = new File(destPath);
        File src1File     = new File(src1Path);
        File src2File     = new File(src2Path);
        File src3File     = new File(src3Path);
        File src4File     = new File(src4Path);
        File src5File     = new File(src5Path);
        File src6File     = new File(src6Path);

        File dest1File    = new File(dest1Path);
        File dest2File    = new File(dest2Path);
        File dest3File    = new File(dest3Path);
        File dest4File    = new File(dest4Path);
        File dest5File    = new File(dest5Path);
        File dest6File    = new File(dest6Path);
        
        File f = new File(rpath);
        if (!f.exists()) {
            editor.statusError("86EVA folder is not found!");
            return;
        }
        
        try {
            // Clear ALL files in 86EVA except Autoupload.ini, firmware.exe, _fecfg.exe
            clearAllFile(destFolder);
            
            // If board is M01, don't copy resource png to 86EVA directory
            if (!boardName.equals("QECM2") && !boardName.equals("QECM02")) {
                copyResourceTo86EVA(srcFolder, destFolder);
            }
            
            // Copy the nesseary file to 86EVA directory
            // !!! Note: If have the same filename in dest folder, we will get IOException.
            if (src1File.exists()) copyFileUsingJava7Files(src1File, dest1File);
            if (src2File.exists()) copyFileUsingJava7Files(src2File, dest2File);
            if (src3File.exists()) copyFileUsingJava7Files(src3File, dest3File);
            if (src4File.exists()) copyFileUsingJava7Files(src4File, dest4File);
            if (src5File.exists()) copyFileUsingJava7Files(src5File, dest5File);
            if (src6File.exists()) copyFileUsingJava7Files(src6File, dest6File);
        } catch (IOException e) {
            editor.statusError("Copy file error!");
            return;
        }
        
        // show message box
        editor.statusNotice("Uplaoding bootloader resource ... waiting about 20 sec.");
        
        String uploadPort = PreferencesData.get("serial.port");
        
        try {
            if (editor.serialMonitor != null) {
              editor.serialMonitor.close();
              editor.serialMonitor.setVisible(false);
            }
            if (editor.serialPlotter != null) {
              editor.serialPlotter.close();
              editor.serialPlotter.setVisible(false);
            }
        } catch (PreferencesMapException e) {
            editor.statusError(I18n.format(
                      tr("Error while uploading: missing '{0}' configuration parameter"),
                      e.getMessage()));
        } catch (Exception e) {
            e.printStackTrace();
        }
      
        try {
            Thread.sleep(500);
        }catch(Exception e) {
            System.out.println(e);
        }
        
        // run v86dude.exe
        sysExec(new String[]{tool.getAbsolutePath(), uploadPort, Integer.toString(0x22), src1Path, boardName, rpath}); // 不用下命令 0x25 檢查來源和目標空間大小，因為我自己知道 86EVA 的內容有沒有超過 disk 限制
      }
    }
  }
  // 86Duino End ---
  
  public void handleSerial() {
    if(serialPlotter != null) {
      if(serialPlotter.isClosed()) {
        serialPlotter = null;
      } else {
        statusError(tr("Serial monitor not available while plotter is open"));
        return;
      }
    }

    if (serialMonitor != null) {
      // The serial monitor already exists

      if (serialMonitor.isClosed()) {
        serialMonitor.dispose();
        // If it's closed, clear the refrence to the existing
        // monitor and create a new one
        serialMonitor = null;
      }
      else {
        // If it's not closed, give it the focus
        try {
          serialMonitor.toFront();
          serialMonitor.requestFocus();
          return;
        } catch (Exception e) {
          // noop
        }
      }
    }

    BoardPort port = Base.getDiscoveryManager().find(PreferencesData.get("serial.port"));

    if (port == null) {
      statusError(I18n.format(tr("Board at {0} is not available"), PreferencesData.get("serial.port")));
      return;
    }

    serialMonitor = new MonitorFactory().newMonitor(port);

    if (serialMonitor == null) {
      String board = port.getPrefs().get("board");
      String boardName = BaseNoGui.getPlatform().resolveDeviceByBoardID(BaseNoGui.packages, board);
      statusError(I18n.format(tr("Serial monitor is not supported on network ports such as {0} for the {1} in this release"), PreferencesData.get("serial.port"), boardName));
      return;
    }

    base.addEditorFontResizeListeners(serialMonitor);
    Base.setIcon(serialMonitor);

    // If currently uploading, disable the monitor (it will be later
    // enabled when done uploading)
    if (uploading || avoidMultipleOperations) {
      try {
        serialMonitor.suspend();
      } catch (Exception e) {
        statusError(e);
      }
    }

    boolean success = false;
    do {
      if (serialMonitor.requiresAuthorization() && !PreferencesData.has(serialMonitor.getAuthorizationKey())) {
        PasswordAuthorizationDialog dialog = new PasswordAuthorizationDialog(this, tr("Type board password to access its console"));
        dialog.setLocationRelativeTo(this);
        dialog.setVisible(true);

        if (dialog.isCancelled()) {
          statusNotice(tr("Unable to open serial monitor"));
          return;
        }

        PreferencesData.set(serialMonitor.getAuthorizationKey(), dialog.getPassword());
      }

      try {
        if (!avoidMultipleOperations) {
          serialMonitor.open();
        }
        serialMonitor.setVisible(true);
        success = true;
        statusEmpty();
      } catch (ConnectException e) {
        statusError(tr("Unable to connect: is the sketch using the bridge?"));
      } catch (JSchException e) {
        statusError(tr("Unable to connect: wrong password?"));
      } catch (SerialException e) {
        String errorMessage = e.getMessage();
        if (e.getCause() != null && e.getCause() instanceof SerialPortException) {
          errorMessage += " (" + ((SerialPortException) e.getCause()).getExceptionType() + ")";
        }
        statusError(errorMessage);
        // Close and release the half-opened monitor *before* dropping the
        // reference, so the next click starts fresh instead of taking the
        // "already open" path and silently doing nothing.
        try {
          serialMonitor.close();
        } catch (Exception e1) {
          // noop
        }
        serialMonitor = null;
      } catch (Exception e) {
        statusError(e);
      } finally {
        if (serialMonitor != null && serialMonitor.requiresAuthorization() && !success) {
          PreferencesData.remove(serialMonitor.getAuthorizationKey());
        }
      }

    } while (serialMonitor != null && serialMonitor.requiresAuthorization() && !success);

  }

  public void handlePlotter() {
    if(serialMonitor != null) {
      if(serialMonitor.isClosed()) {
        serialMonitor = null;
      } else {
        statusError(tr("Plotter not available while serial monitor is open"));
        return;
      }
    }

    if (serialPlotter != null) {
      // The serial plotter already exists

      if (serialPlotter.isClosed()) {
        // If it's closed, clear the refrence to the existing
        // plotter and create a new one
        serialPlotter.dispose();
        serialPlotter = null;
      }
      else {
        // If it's not closed, give it the focus
        try {
          serialPlotter.toFront();
          serialPlotter.requestFocus();
          return;
        } catch (Exception e) {
          // noop
        }
      }
    }

    BoardPort port = Base.getDiscoveryManager().find(PreferencesData.get("serial.port"));

    if (port == null) {
      statusError(I18n.format(tr("Board at {0} is not available"), PreferencesData.get("serial.port")));
      return;
    }

    serialPlotter = new SerialPlotter(port);
    Base.setIcon(serialPlotter);

    // If currently uploading, disable the plotter (it will be later
    // enabled when done uploading)
    if (uploading) {
      try {
        serialPlotter.suspend();
      } catch (Exception e) {
        statusError(e);
      }
    }

    boolean success = false;
    do {
      if (serialPlotter.requiresAuthorization() && !PreferencesData.has(serialPlotter.getAuthorizationKey())) {
        PasswordAuthorizationDialog dialog = new PasswordAuthorizationDialog(this, tr("Type board password to access its console"));
        dialog.setLocationRelativeTo(this);
        dialog.setVisible(true);

        if (dialog.isCancelled()) {
          statusNotice(tr("Unable to open serial plotter"));
          return;
        }

        PreferencesData.set(serialPlotter.getAuthorizationKey(), dialog.getPassword());
      }

      try {
        serialPlotter.open();
        serialPlotter.setVisible(true);
        success = true;
        statusEmpty();
      } catch (ConnectException e) {
        statusError(tr("Unable to connect: is the sketch using the bridge?"));
      } catch (JSchException e) {
        statusError(tr("Unable to connect: wrong password?"));
      } catch (SerialException e) {
        String errorMessage = e.getMessage();
        if (e.getCause() != null && e.getCause() instanceof SerialPortException) {
          errorMessage += " (" + ((SerialPortException) e.getCause()).getExceptionType() + ")";
        }
        statusError(errorMessage);
        // Close and release the half-opened plotter *before* dropping the
        // reference, so the next click starts fresh instead of taking the
        // "already open" path and silently doing nothing.
        try {
          serialPlotter.close();
        } catch (Exception e1) {
          // noop
        }
        serialPlotter = null;
      } catch (Exception e) {
        statusError(e);
      } finally {
        if (serialPlotter != null && serialPlotter.requiresAuthorization() && !success) {
          PreferencesData.remove(serialPlotter.getAuthorizationKey());
        }
      }

    } while (serialPlotter != null && serialPlotter.requiresAuthorization() && !success);

  }

  private void handleBurnBootloader() {
    console.clear();
    EditorConsole.setCurrentEditorConsole(this.console);
    statusNotice(tr("Burning bootloader to I/O Board (this may take a minute)..."));
    new Thread(() -> {
      try {
        Uploader uploader = new SerialUploader();
        if (uploader.burnBootloader()) {
          SwingUtilities.invokeLater(() -> statusNotice(tr("Done burning bootloader.")));
        } else {
          SwingUtilities.invokeLater(() -> statusError(tr("Error while burning bootloader.")));
          // error message will already be visible
        }
      } catch (SerialNotFoundException e) {
        SwingUtilities.invokeLater(() -> statusError(tr("Error while burning bootloader: please select a serial port.")));
      } catch (PreferencesMapException e) {
        SwingUtilities.invokeLater(() -> {
          statusError(I18n.format(
            tr("Error while burning bootloader: missing '{0}' configuration parameter"),
            e.getMessage()));
        });
      } catch (RunnerException e) {
        SwingUtilities.invokeLater(() -> statusError(e.getMessage()));
      } catch (Exception e) {
        SwingUtilities.invokeLater(() -> statusError(tr("Error while burning bootloader.")));
        e.printStackTrace();
      }
    }).start();
  }

  private void saveAppearanceSettings() {
    base.storeSketchLocation(false);
    statusNotice(tr("Done Saving."));
  }

  private void resetAppearanceSettings() {
    base.storeSketchLocation(true);
    statusNotice(tr("Done Reset."));
  }

  private void handle86BurnBootloader(String _86board) {
    console.clear();
    statusNotice(tr("Burning bootloader to I/O Board (this may take a minute)..."));
    
    if (_86board.contains("QEC")) {
          new Thread(upload86EVAres).start();
          return;
    }
    
    new Thread(() -> {
      try {
        Uploader uploader = new SerialUploader();
        if (uploader.burn86Bootloader(_86board)) {
          SwingUtilities.invokeLater(() -> statusNotice(tr("Done burning bootloader.")));
        } else {
          SwingUtilities.invokeLater(() -> statusError(tr("Error while burning bootloader.")));
          // error message will already be visible
        }
      } catch (PreferencesMapException e) {
        SwingUtilities.invokeLater(() -> {
          statusError(I18n.format(
            tr("Error while burning bootloader: missing '{0}' configuration parameter"),
            e.getMessage()));
        });
      } catch (RunnerException e) {
        SwingUtilities.invokeLater(() -> statusError(e.getMessage()));
      } catch (Exception e) {
        SwingUtilities.invokeLater(() -> statusError(tr("Error while burning bootloader.")));
        e.printStackTrace();
      }
    }).start();
  }
  private void handleBoardInfo() {
    console.clear();

    String selectedPort = PreferencesData.get("serial.port");
    List<BoardPort> ports = Base.getDiscoveryManager().discovery();

    String label = "";
    String vid = "";
    String pid = "";
    String iserial = "";
    String protocol = "";
    boolean found = false;

    for (BoardPort port : ports) {
      if (port.getAddress().equals(selectedPort)) {
        label = port.getBoardName();
        vid = port.getPrefs().get("vid");
        pid = port.getPrefs().get("pid");
        iserial = port.getPrefs().get("iserial");
        protocol = port.getProtocol();
        found = true;
        break;
      }
    }

    if (!found) {
      statusNotice(tr("Please select a port to obtain board info"));
      return;
    }

    if (protocol.equals("network")) {
      statusNotice(tr("Network port, can't obtain info"));
      return;
    }

    if (vid == null || vid.equals("") || vid.equals("0000")) {
      statusNotice(tr("Native serial port, can't obtain info"));
      return;
    }

    if (iserial == null || iserial.equals("")) {
      iserial = tr("Upload any sketch to obtain it");
    }

    if (label == null) {
      label = tr("Unknown board");
    }

    String infos = I18n.format("BN: {0}\nVID: {1}\nPID: {2}\nSN: {3}", label, vid, pid, iserial);
    JTextArea textArea = new JTextArea(infos);

    JOptionPane.showMessageDialog(this, textArea, tr("Board Info"), JOptionPane.PLAIN_MESSAGE);
  }

  /**
   * Handler for File &rarr; Page Setup.
   */
  private void handlePageSetup() {
    PrinterJob printerJob = PrinterJob.getPrinterJob();
    if (pageFormat == null) {
      pageFormat = printerJob.defaultPage();
    }
    pageFormat = printerJob.pageDialog(pageFormat);
  }


  /**
   * Handler for File &rarr; Print.
   */
  private void handlePrint() {
    statusNotice(tr("Printing..."));
    //printerJob = null;
    PrinterJob printerJob = PrinterJob.getPrinterJob();
    if (pageFormat != null) {
      //System.out.println("setting page format " + pageFormat);
      printerJob.setPrintable(getCurrentTab().getTextArea(), pageFormat);
    } else {
      printerJob.setPrintable(getCurrentTab().getTextArea());
    }
    // set the name of the job to the code name
    printerJob.setJobName(getCurrentTab().getSketchFile().getPrettyName());

    if (printerJob.printDialog()) {
      try {
        printerJob.print();
        statusNotice(tr("Done printing."));

      } catch (PrinterException pe) {
        statusError(tr("Error while printing."));
        pe.printStackTrace();
      }
    } else {
      statusNotice(tr("Printing canceled."));
    }
    //printerJob = null;  // clear this out?
  }


  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .


  /**
   * Show an error int the status bar.
   */
  public void statusError(String what) {
    System.err.println(what);
    status.error(what);
    //new Exception("deactivating RUN").printStackTrace();
    toolbar.deactivateRun();
  }


  /**
   * Show an exception in the editor status bar.
   */
  public void statusError(Exception e) {
    e.printStackTrace();
//    if (e == null) {
//      System.err.println("Editor.statusError() was passed a null exception.");
//      return;
//    }

    if (e instanceof RunnerException) {
      RunnerException re = (RunnerException) e;
      if (re.hasCodeFile()) {
        selectTab(findTabIndex(re.getCodeFile()));
      }
      if (re.hasCodeLine()) {
        int line = re.getCodeLine();
        // subtract one from the end so that the \n ain't included
        if (line >= getCurrentTab().getTextArea().getLineCount()) {
          // The error is at the end of this current chunk of code,
          // so the last line needs to be selected.
          line = getCurrentTab().getTextArea().getLineCount() - 1;
          if (getCurrentTab().getLineText(line).length() == 0) {
            // The last line may be zero length, meaning nothing to select.
            // If so, back up one more line.
            line--;
          }
        }
        if (line < 0 || line >= getCurrentTab().getTextArea().getLineCount()) {
          System.err.println(I18n.format(tr("Bad error line: {0}"), line));
        } else {
          try {
            addLineHighlight(line);
          } catch (BadLocationException e1) {
            e1.printStackTrace();
          }
        }
      }
    }

    // Since this will catch all Exception types, spend some time figuring
    // out which kind and try to give a better error message to the user.
    String mess = e.getMessage();
    if (mess != null) {
      String javaLang = "java.lang.";
      if (mess.indexOf(javaLang) == 0) {
        mess = mess.substring(javaLang.length());
      }
      String rxString = "RuntimeException: ";
      if (mess.indexOf(rxString) == 0) {
        mess = mess.substring(rxString.length());
      }
      statusError(mess);
    }
//    e.printStackTrace();
  }


  /**
   * Show a notice message in the editor status bar.
   */
  public void statusNotice(String msg) {
    status.notice(msg);
  }


  /**
   * Clear the status area.
   */
  private void statusEmpty() {
    statusNotice(EMPTY);
  }

  // . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

  String privBoardName = "";
  public boolean boardSeriesChanged() {
    String[] nowBoardName;
    
    if (BaseNoGui.getTargetBoard().getAliasName() != null)
        nowBoardName = BaseNoGui.getTargetBoard().getAliasName().split(" ");
    else
        nowBoardName = BaseNoGui.getTargetBoard().getName().split(" ");
    
    if (!nowBoardName[0].equals(privBoardName)) {
        privBoardName = nowBoardName[0];
        return true;
    }
    
    return false;
  }

  public void updateToolsName() {
    JMenuItem item;
    item = globalToolItems.get("Upload All Resources");
    if (item != null) {
      item.setText(tr("Upload Resources to " + BaseNoGui.getTargetBoard().getAliasName()));
    }
    item = globalToolItems.get("Download All Resources");
    if (item != null) {
      item.setText(tr("Download Resources from " + BaseNoGui.getTargetBoard().getAliasName()));
    }
    item = globalToolItems.get("Reset EtherCAT Master");
    if (item != null) {
      item.setText(tr("Reset the " + BaseNoGui.getTargetBoard().getAliasName()));
    }

    //If the selected borad is QEC M01, add "Launch Bootloader Menu" option. If not, remove it.
    item = globalToolItems.get(tr("Launch Bootloader Menu"));
    if (BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QECM2") ||
        BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QECM02")) {
      if (item != null) toolsMenu.add(item);
    } else {
      if (item != null) toolsMenu.remove(item);
    }
  }

  public void updateHelpName() {
    if (BaseNoGui.getTargetBoard().getPreferences().get("name").contains("QEC")) {
      if (helpItem != null) helpItem.setText(tr("Visit qec.tw"));
    } else {
      if (helpItem != null) helpItem.setText(tr("Visit 86duino.com"));
    }
  }
  
  protected void onBoardOrPortChange() {
    TargetBoard board = BaseNoGui.getTargetBoard();
    if (board != null) {
      if (board.getAliasName() != null) {// 86Duino
        lineStatus.setBoardName(board.getAliasName()); // 86Duino
      } else {
        lineStatus.setBoardName(board.getName());
      }
    } else
      lineStatus.setBoardName("-");
    lineStatus.setPort(PreferencesData.get("serial.port"));
    lineStatus.repaint();
    // if select 86Duino board, update menu bar
    if (BaseNoGui.getTargetPlatform().getPreferences().get("name").indexOf("Vortex86") >= 0) {
      // rebuild a part of Skecth menu
      final JMenu sketchMenu = new JMenu(tr("Sketch"));
      sketchMenu.setMnemonic(KeyEvent.VK_S);
      buildSketchMenu(sketchMenu);
      sketchMenu.revalidate();
      validate();

      toolsMenu.remove(programmersMenu);
      
    } else { // Arduino
      toolsMenu.add(programmersMenu);
      programmersMenu.removeAll();
      base.getProgrammerMenus().forEach(programmersMenu::add);
    }

    updateToolsName();
    updateHelpName();

    // 86Duino special function
    if (boardSeriesChanged()) {
        rebuildToolsMenu();
        toolbar.rebuildTouchBar();
        
        // update all icon button, must repaint splitPane
        List<Editor> editors = Base.INSTANCE.getEditors();
        for (Editor editor : editors) {
            editor.getSplitPane().repaint();
        }
    }
    
    burnbootloaderMenu.removeAll();
    rebuildburnbootloaderMenu(burnbootloaderMenu);
  }

  // 86Duino special function
  public JSplitPane getSplitPane() {
    return splitPane;
  }
  
  public void addCompilerProgressListener(CompilerProgressListener listener){
    this.status.addCompilerProgressListener(listener);
  }

}
