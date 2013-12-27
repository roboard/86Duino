/* -*- mode: java; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
  Part of the 86Duino project - http://www.86duino.com/

  Copyright (c) 2013    Oswald Kuo <oswald@dmp.com.tw>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

package processing.app.debug;

import static processing.app.I18n._;

import java.io.*;
import java.nio.channels.FileChannel;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileInputStream;  
import java.io.FileOutputStream;  
import java.io.InputStreamReader;  
import java.io.PrintWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;



import processing.app.Base;
import processing.app.I18n;
import processing.app.Preferences;
import processing.app.Sketch;
import processing.app.SketchCode;
import processing.app.helpers.PreferencesMap;
import processing.app.helpers.StringReplacer;
import processing.core.PApplet;
import processing.app.Preferences;



public class DuinoCompiler implements MessageConsumer {
  static final String BUGS_URL =
    _("http://github.com/arduino/Arduino/issues");
  static final String SUPER_BADNESS =
    I18n.format(_("Compiler error, please submit this code to {0}"), BUGS_URL);

  private Sketch sketch;

  private List<File> objectFiles;

  private PreferencesMap prefs;
  private boolean verbose;
  private boolean sketchIsCompiled;
	
  private RunnerException exception;
  
  private boolean Verbord;
  private String strVerbord="Error compiling.";
    
  List<String> file_path = new ArrayList<String>();
  List<String> file_name = new ArrayList<String>();
  List<String> IncludeName = new ArrayList<String>();
  List<String> Includeliba = new ArrayList<String>();  
  String primaryClassName;
  String tmppath;
  String dosboxpath;
  String djgpppath;
  String standapipath;
  String standlibrariepath;
  String makefilesourcepath="";
  String dosbox_mount;
  String ofile_path;
  String str_libfiles="";
  TargetPlatform tp;
  String idepath;
  String idepath_nochande;
  MessageConsumer consumer;
  /**
   * Compile sketch.
   *
   * @param _sketch Sketch object to be compiled.
   * @param _buildPath Where the temporary files live and will be built from.
   * @param _primaryClassName the name of the combined sketch file w/ extension
   * @return true if successful.
   * @throws RunnerException Only if there's a problem. Only then.
   */
  public boolean compile(Sketch _sketch, String _buildPath,
                         String _primaryClassName, boolean _verbose)
      throws RunnerException {
    sketch = _sketch;
    verbose = _verbose;
    sketchIsCompiled = false;
    objectFiles = new ArrayList<File>();
    primaryClassName= _primaryClassName;

    prefs = createBuildPreferences(_buildPath, _primaryClassName);

    // 0. include paths for core + all libraries
    sketch.setCompilingProgress(20);
    List<String> includePaths = new ArrayList<String>();
    includePaths.add(prefs.get("build.core.path"));
    
    //PATH
    tmppath           =_buildPath;
    idepath           = prefs.get("runtime.ide.path");
    dosboxpath        ="/DOSBox-0.74/";
    djgpppath         ="/DJGPP";
    standapipath      ="/hardware/86duino/"+tp.getName()+"/cores/arduino";
    standlibrariepath ="/hardware/86duino/"+tp.getName()+"/libraries";
    
    chngeIncludeData(idepath+djgpppath);
    chngeIncludelibaData(idepath+djgpppath);
    idepath_nochande=idepath;
    idepath=idepath.replace("\\","/");
    
    if (prefs.get("build.variant.path").length() != 0)
      includePaths.add(prefs.get("build.variant.path")); 
    for (File file : sketch.getImportedLibraries()) {
      includePaths.add(file.getPath());
    }
    
    // 1. compile the sketch (already in the buildPath)
    sketch.setCompilingProgress(30);
    mainHave(_buildPath);
    dosbox_mount="d:/";
    ofile_path="d:/";
    compileSketch(includePaths);
    sketchIsCompiled = true;

    // 2. compile the libraries, outputting .o files to: <buildPath>/<library>/
    // Doesn't really use configPreferences
    sketch.setCompilingProgress(40);
    makefilesourcepath="";
    compileLibraries(includePaths);

    // 3. compile the core, outputting .o files to <buildPath> and then
    // collecting them into the core.a library file.
    sketch.setCompilingProgress(50);
    dosbox_mount="h:/";
    ofile_path="c:/compile/";
    compileCore();

    // 4. link it all together into the .elf file
    sketch.setCompilingProgress(60);
    writeDosboxconf(idepath+dosboxpath,_buildPath);
    writeMakefile(_buildPath); 
    if(Base.isLinux()){
      String[] cmd_linux={"dosbox","-conf" ,idepath+dosboxpath+"dosbox-86duino.conf","-noconsole"};
      execAsynchronously(cmd_linux);}
    else{             
      String[] cmd_windows={idepath+dosboxpath+"DOSBox.exe","-conf" ,idepath+dosboxpath+"dosbox-86duino.conf","-noconsole"};
      execAsynchronously(cmd_windows);}
    
    
    Verbord=true;    
    if(!compileSusses(_buildPath))
      Verbord=false;
    else{
      String[] cmdd={idepath+dosboxpath+"upx309w/upx.exe", tmppath+"/86duino.exe"};
      execAsynchronously(cmdd);
    }
     
    writeMakeMessage(_buildPath + File.separator + "MESSAGE.TXT");
    
    sketch.setCompilingProgress(90);
    return true;
  }

  private PreferencesMap createBuildPreferences(String _buildPath,
                                                String _primaryClassName)
      throws RunnerException {
    
    if (Base.getBoardPreferences() == null) {
      RunnerException re = new RunnerException(
          _("No board selected; please choose a board from the Tools > Board menu."));
      re.hideStackTrace();
      throw re;
    }

    TargetPlatform targetPlatform = Base.getTargetPlatform();

    // Merge all the global preference configuration in order of priority
    PreferencesMap p = new PreferencesMap();
    p.putAll(Preferences.getMap());
    p.putAll(targetPlatform.getPreferences());
    p.putAll(Base.getBoardPreferences());
    for (String k : p.keySet()) {
      if (p.get(k) == null)
        p.put(k, "");
    }

    p.put("build.path", _buildPath);
    p.put("build.project_name", _primaryClassName);
    
    if (!p.containsKey("compiler.path"))
      p.put("compiler.path", Base.getAvrBasePath());

    // Core folder
    String core = p.get("build.core");
    //TargetPlatform tp;
    if (!core.contains(":")) {
      tp = targetPlatform;
    } else {
      String[] split = core.split(":", 2);
      tp = Base.getTargetPlatform(split[0], Preferences.get("target_platform"));
      core = split[1];
    }
    File coreFolder = new File(tp.getFolder(), "cores");
    coreFolder = new File(coreFolder, core);
    p.put("build.core.path", coreFolder.getAbsolutePath());
    
    // System Folder
    File systemFolder = targetPlatform.getFolder();
    

    systemFolder = new File(systemFolder, "system");
    p.put("build.system.path", systemFolder.getAbsolutePath());
    
    // Variant Folder
    String variant = p.get("build.variant");
    if (variant != null) {
      TargetPlatform t;
      if (!variant.contains(":")) {
        t = targetPlatform;
      } else {
        String[] split = variant.split(":", 2);
        t = Base
            .getTargetPlatform(split[0], Preferences.get("target_platform"));
        variant = split[1];
      }
      File variantFolder = new File(t.getFolder(), "variants");
      variantFolder = new File(variantFolder, variant);
      p.put("build.variant.path", variantFolder.getAbsolutePath());
    } else {
      p.put("build.variant.path", "");
    }
    
    return p;
  }

  private List<File> compileFiles(String outputPath, File sourcePath,
                                  boolean recurse, List<String> includePaths)
      throws RunnerException {
    List<File> hSources = findFilesInFolder(sourcePath, "h", recurse);
    List<File> sSources = findFilesInFolder(sourcePath, "S", recurse);
    List<File> cSources = findFilesInFolder(sourcePath, "c", recurse);
    List<File> cppSources = findFilesInFolder(sourcePath, "cpp", recurse);
    List<File> objectPaths = new ArrayList<File>();
    
 		for (File file : hSources) {    
      if((dosbox_mount.compareTo("d:/"))==0){
        chngeInclude(outputPath,file.getName());
      }
                     
    }
    for (File file : sSources) {
      if((dosbox_mount.compareTo("d:/"))==0){
        chngeInclude(outputPath,file.getName());
      }
      file_path.add(ofile_path+file.getName());
      file_name.add(dosbox_mount+file.getName());
      
    }
    for (File file : cSources) {
      if((dosbox_mount.compareTo("d:/"))==0){
        chngeInclude(outputPath,file.getName());
      }
      file_path.add(ofile_path+file.getName());
      file_name.add(dosbox_mount+file.getName());
    
    }

    for (File file : cppSources) {
      if((dosbox_mount.compareTo("d:/"))==0){
        chngeInclude(outputPath,file.getName());
      }
      if((file.getName().compareTo(primaryClassName))!=0){
        file_path.add(ofile_path+file.getName());
        file_name.add(dosbox_mount+file.getName());
      }
      else{
        file_path.add(ofile_path+file.getName());
        file_name.add(dosbox_mount+file.getName());
      }
    }
    
    return objectPaths;
  }

  private boolean is_already_compiled(File src, File obj, File dep, Map<String, String> prefs) {
    boolean ret=true;
    try {
      if (!obj.exists()) return false;  // object file (.o) does not exist
      if (!dep.exists()) return false;  // dep file (.d) does not exist
      long src_modified = src.lastModified();
      long obj_modified = obj.lastModified();
      if (src_modified >= obj_modified) return false;  // source modified since object compiled
      if (src_modified >= dep.lastModified()) return false;  // src modified since dep compiled
      BufferedReader reader = new BufferedReader(new FileReader(dep.getPath()));
      String line;
      boolean need_obj_parse = true;
      while ((line = reader.readLine()) != null) {
        if (line.endsWith("\\")) {
          line = line.substring(0, line.length() - 1);
        }
        line = line.trim();
        if (line.length() == 0) continue; // ignore blank lines
        if (need_obj_parse) {
          // line is supposed to be the object file - make sure it really is!
          if (line.endsWith(":")) {
            line = line.substring(0, line.length() - 1);
            String objpath = obj.getCanonicalPath();
            File linefile = new File(line);
            String linepath = linefile.getCanonicalPath();
            if (objpath.compareTo(linepath) == 0) {
              need_obj_parse = false;
              continue;
            } else {
              ret = false;  // object named inside .d file is not the correct file!
              break;
            }
          } else {
            ret = false;  // object file supposed to end with ':', but didn't
            break;
          }
        } else {
          // line is a prerequisite file
          File prereq = new File(line);
          if (!prereq.exists()) {
            ret = false;  // prerequisite file did not exist
            break;
          }
          if (prereq.lastModified() >= obj_modified) {
            ret = false;  // prerequisite modified since object was compiled
            break;
          }
        }
      }
      reader.close();
    } catch (Exception e) {
      return false;  // any error reading dep file = recompile it
    }
    if (ret && (verbose || Preferences.getBoolean("build.verbose"))) {
      System.out.println("  Using previously compiled: " + obj.getPath());
    }
    return ret;
  }

  boolean firstErrorFound;
  boolean secondErrorFound;

  /**
   * Either succeeds or throws a RunnerException fit for public consumption.
   */
  private void execAsynchronously(String[] command) throws RunnerException {

    // eliminate any empty array entries
    List<String> stringList = new ArrayList<String>();
    for (String string : command) {
      string = string.trim();
      if (string.length() != 0)
        stringList.add(string);
    }
    command = stringList.toArray(new String[stringList.size()]);
    if (command.length == 0)
      return;
    int result = 0;

    if (verbose || Preferences.getBoolean("build.verbose")) {
      for (String c : command)
        //System.out.print(c + " ");
      System.out.println();
    }

    firstErrorFound = false;  // haven't found any errors yet
    secondErrorFound = false;

    Process process;
    
    try {
      
      process = Runtime.getRuntime().exec(command);
    } catch (IOException e) {
      RunnerException re = new RunnerException(e.getMessage());
      re.hideStackTrace();
      throw re;
    }

    MessageSiphon in = new MessageSiphon(process.getInputStream(), this);
    MessageSiphon err = new MessageSiphon(process.getErrorStream(), this);

    // wait for the process to finish.  if interrupted
    // before waitFor returns, continue waiting
    boolean compiling = true;
    while (compiling) {
      try {
        in.join();
        err.join();
        result = process.waitFor();
        compiling = false;
      } catch (InterruptedException ignored) { }
    }

    // an error was queued up by message(), barf this back to compile(),
    // which will barf it back to Editor. if you're having trouble
    // discerning the imagery, consider how cows regurgitate their food
    // to digest it, and the fact that they have five stomaches.
    //
    if (exception != null)
      throw exception;

    if (result > 1) {
      // a failure in the tool (e.g. unable to locate a sub-executable)
      System.err
          .println(I18n.format(_("{0} returned {1}"), command[0], result));
    }

    if (result != 0) {
      RunnerException re = new RunnerException(_("Error compiling."));
      re.hideStackTrace();
      throw re;
    }
  }

  /**
   * Part of the MessageConsumer interface, this is called
   * whenever a piece (usually a line) of error message is spewed
   * out from the compiler. The errors are parsed for their contents
   * and line number, which is then reported back to Editor.
   */
  public void message(String s) {
    int i;
    // remove the build path so people only see the filename
    // can't use replaceAll() because the path may have characters in it which
    // have meaning in a regular expression.
    if (!verbose) {
      String buildPath = prefs.get("build.path");
      while ((i = s.indexOf(buildPath + File.separator)) != -1) {
        s = s.substring(0, i) + s.substring(i + (buildPath + File.separator).length());
      }
    }
    // look for error line, which contains file name, line number,
    // and at least the first line of the error message
    String errorFormat = "([\\w\\d_]+.\\w+):(\\d+):\\s*error:\\s*(.*)\\s*";
    String[] pieces = PApplet.match(s, errorFormat);

     if (pieces != null){
      
    }

//    if (pieces != null && exception == null) {
//      exception = sketch.placeException(pieces[3], pieces[1], PApplet.parseInt(pieces[2]) - 1);
//      if (exception != null) exception.hideStackTrace();
//    }
    

    
    if (pieces != null) {
      String error = pieces[3], msg = "";
      
      if (pieces[3].trim().equals("SPI.h: No such file or directory")) {
        strVerbord = _("Please import the SPI library from the Sketch > Import Library menu.");
        msg = _("\nAs of Arduino 0019, the Ethernet library depends on the SPI library." +
              "\nYou appear to be using it or another library that depends on the SPI library.\n\n");
      }
      
      if (pieces[3].trim().equals("'BYTE' was not declared in this scope")) {
        strVerbord = _("The 'BYTE' keyword is no longer supported.");
        msg = _("\nAs of Arduino 1.0, the 'BYTE' keyword is no longer supported." +
              "\nPlease use Serial.write() instead.\n\n");
      }
      
      if (pieces[3].trim().equals("no matching function for call to 'Server::Server(int)'")) {
        strVerbord = _("The Server class has been renamed EthernetServer.");
        msg = _("\nAs of Arduino 1.0, the Server class in the Ethernet library " +
              "has been renamed to EthernetServer.\n\n");
      }
      
      if (pieces[3].trim().equals("no matching function for call to 'Client::Client(byte [4], int)'")) {
        strVerbord = _("The Client class has been renamed EthernetClient.");
        msg = _("\nAs of Arduino 1.0, the Client class in the Ethernet library " +
              "has been renamed to EthernetClient.\n\n");
      }
      
      if (pieces[3].trim().equals("'Udp' was not declared in this scope")) {
        strVerbord = _("The Udp class has been renamed EthernetUdp.");
        msg = _("\nAs of Arduino 1.0, the Udp class in the Ethernet library " +
              "has been renamed to EthernetUdp.\n\n");
      }
      
      if (pieces[3].trim().equals("'class TwoWire' has no member named 'send'")) {
        strVerbord = _("Wire.send() has been renamed Wire.write().");
        msg = _("\nAs of Arduino 1.0, the Wire.send() function was renamed " +
              "to Wire.write() for consistency with other libraries.\n\n");
      }
      
      if (pieces[3].trim().equals("'class TwoWire' has no member named 'receive'")) {
        strVerbord = _("Wire.receive() has been renamed Wire.read().");
        msg = _("\nAs of Arduino 1.0, the Wire.receive() function was renamed " +
              "to Wire.read() for consistency with other libraries.\n\n");
      }

      if (pieces[3].trim().equals("'Mouse' was not declared in this scope")) {
        strVerbord = _("'Mouse' only supported on the Arduino Leonardo");
        msg = _("\nThe 'Mouse' class is only supported on the Arduino Leonardo.\n\n");
      }
      
      if (pieces[3].trim().equals("'Keyboard' was not declared in this scope")) {
        strVerbord = _("'Keyboard' only supported on the Arduino Leonardo");
        msg = _("\nThe 'Keyboard' class is only supported on the Arduino Leonardo.\n\n");
      }
      if (pieces[3].trim().equals("'Servoo' was not declared in this scope")) {
        strVerbord = _("'Servo1' only supported on the Arduino Leonardo");
        msg = _("\nThe 'Keyboard' class is only supported on the Arduino Leonardo.\n\n");
      }
      
      RunnerException e = null;
      if (sketchIsCompiled) {
        // Place errors when compiling the sketch, but never while compiling libraries
        // or the core.  The user's sketch might contain the same filename!
        e = sketch.placeException(error, pieces[1], PApplet.parseInt(pieces[2]) - 1);
      }
      // replace full file path with the name of the sketch tab (unless we're
      // in verbose mode, in which case don't modify the compiler output)
      if (e != null && !verbose) {
        SketchCode code = sketch.getCode(e.getCodeIndex());
        String fileName = (code.isExtension("ino") || code.isExtension("pde")) ? code.getPrettyName() : code.getFileName();
        int lineNum = e.getCodeLine() + 1;
        s = fileName + ":" + lineNum + ": error: " + pieces[3] + msg;        
      }
            
      if (exception == null && e != null) {
        exception = e;
        exception.hideStackTrace();
      }      
    } 
    
    System.err.println(s);
  }

  private String[] getCommandCompilerS(List<String> includePaths,
                                       String sourceName, String objectName)
      throws RunnerException {
    String includes = preparePaths(includePaths);
    PreferencesMap dict = new PreferencesMap(prefs);
    dict.put("ide_version", "" + Base.REVISION);
    dict.put("includes", includes);
    dict.put("source_file", sourceName);
    dict.put("object_file", objectName);

    try {
      String cmd = prefs.get("recipe.S.o.pattern");
      return StringReplacer.formatAndSplit(cmd, dict, true);
    } catch (Exception e) {
      throw new RunnerException(e);
    }
  }

  private String[] getCommandCompilerC(List<String> includePaths,
                                       String sourceName, String objectName)
      throws RunnerException {
    String includes = preparePaths(includePaths);

    PreferencesMap dict = new PreferencesMap(prefs);
    dict.put("ide_version", "" + Base.REVISION);
    dict.put("includes", includes);
    dict.put("source_file", sourceName);
    dict.put("object_file", objectName);

    String cmd = prefs.get("recipe.c.o.pattern");
    try {
      return StringReplacer.formatAndSplit(cmd, dict, true);
    } catch (Exception e) {
      throw new RunnerException(e);
    }
  }

  private String[] getCommandCompilerCPP(List<String> includePaths,
                                         String sourceName, String objectName)
      throws RunnerException {
    String includes = preparePaths(includePaths);

    PreferencesMap dict = new PreferencesMap(prefs);
    dict.put("ide_version", "" + Base.REVISION);
    dict.put("includes", includes);
    dict.put("source_file", sourceName);
    dict.put("object_file", objectName);

    String cmd = prefs.get("recipe.cpp.o.pattern");
    try {
      return StringReplacer.formatAndSplit(cmd, dict, true);
    } catch (Exception e) {
      throw new RunnerException(e);
    }
  }

  /////////////////////////////////////////////////////////////////////////////

  private void createFolder(File folder) throws RunnerException {
    if (folder.isDirectory())
      return;
    if (!folder.mkdir())
      throw new RunnerException("Couldn't create: " + folder);
  }

  static public List<File> findFilesInFolder(File folder, String extension,
                                             boolean recurse) {
    List<File> files = new ArrayList<File>();

    if (folder.listFiles() == null)
      return files;

    for (File file : folder.listFiles()) {
      if (file.getName().startsWith("."))
        continue; // skip hidden files

      if (file.getName().endsWith("." + extension))
        files.add(file);

      if (recurse && file.isDirectory()) {
        files.addAll(findFilesInFolder(file, extension, true));
      }
    }

    return files;
  }
  
  // 1. compile the sketch (already in the buildPath)
  void compileSketch(List<String> includePaths) throws RunnerException {
    String buildPath = prefs.get("build.path");
    
    
    objectFiles.addAll(compileFiles(buildPath, new File(buildPath), false,
                                    includePaths));
                                    
  }

  // 2. compile the libraries, outputting .o files to:
  // <buildPath>/<library>/
  void compileLibraries(List<String> includePaths) throws RunnerException {

    
    for (File libraryFolder : sketch.getImportedLibraries()) {
      String outputPath = prefs.get("build.path");
      File outputFolder = new File(outputPath, libraryFolder.getName());
      File utilityFolder = new File(libraryFolder, "utility");
      createFolder(outputFolder);
      
      String aaa= libraryFolder.getAbsolutePath();
      aaa=aaa.replace("\\","/");
      String lib_same=idepath+"/libraries/";
      String[] strArray0 = aaa.split(libraryFolder.getName());
                             
      // this library can use includes in its utility/ folder
      includePaths.add(utilityFolder.getAbsolutePath());
      
      ofile_path="d:/"+libraryFolder.getName()+"/";
      
      if(strArray0[0].compareTo(lib_same) == 0)
        dosbox_mount="f:/"+libraryFolder.getName()+"/";
      else
        dosbox_mount="g:/"+libraryFolder.getName()+"/";
        
      makefilesourcepath=makefilesourcepath+"-I"+dosbox_mount+" -I"+dosbox_mount+"utility ";
      objectFiles.addAll(compileFiles(outputFolder.getAbsolutePath(),
                                      libraryFolder, false, includePaths));
      outputFolder = new File(outputFolder, "utility");
      createFolder(outputFolder);
      
      ofile_path="d:/"+libraryFolder.getName()+"/utility/";
      if(strArray0[0].compareTo(lib_same) == 0)
        dosbox_mount="f:/"+libraryFolder.getName()+"/utility/";
      else
        dosbox_mount="g:/"+libraryFolder.getName()+"/utility/";
      objectFiles.addAll(compileFiles(outputFolder.getAbsolutePath(),
                                      utilityFolder, false, includePaths));
      // other libraries should not see this library's utility/ folder
      includePaths.remove(includePaths.size() - 1);
    }
  }
	
  // 3. compile the core, outputting .o files to <buildPath> and then
  // collecting them into the core.a library file.
  void compileCore()
      throws RunnerException {

    String corePath = prefs.get("build.core.path");
    String variantPath = prefs.get("build.variant.path");
    String buildPath = prefs.get("build.path");
    

    List<String> includePaths = new ArrayList<String>();
    includePaths.add(corePath); // include core path only
    if (variantPath.length() != 0)
      includePaths.add(variantPath);

      
    List<File> coreObjectFiles = compileFiles(buildPath, new File(corePath),
                                              true, includePaths);
    if (variantPath.length() != 0)
      coreObjectFiles.addAll(compileFiles(buildPath, new File(variantPath),
                                          true, includePaths));

  }
	
  void writeDosboxconf(String dosboxpath,String buildPath) throws RunnerException {
    String filePath =dosboxpath+"dosbox-86duino.conf";
    File file = new File(filePath);
    if(file.exists()){
      file.delete();
    }
    if(!file.exists())
    {
      try {
        file.createNewFile();
        BufferedWriter out = new BufferedWriter(new FileWriter(file));
        out.write("[sdl]\n\n");
        out.write("fullscreen=false\n");
        out.write("fulldouble=false\n");
        out.write("fullresolution=original\n");
        out.write("windowresolution=original\n");
        out.write("output=surface\n");
        out.write("autolock=true\n");
        out.write("sensitivity=100\n");
        out.write("waitonerror=true\n");
        out.write("priority=higher,normal\n");
        out.write("mapperfile=mapper-0.74.map\n");
        out.write("usescancodes=true\n");
        out.write("[dosbox]\n\n");
        out.write("language=\n");
        out.write("machine=svga_s3\n");
        out.write("captures=capture\n");
        out.write("memsize=64\n");
        out.write("[render]\n\n");        
        out.write("frameskip=0\n");
        out.write("aspect=false\n");
        out.write("scaler=normal2x\n");        
        out.write("[cpu]\n\n");        
        out.write("core=dynamic\n");
        out.write("cputype=auto\n");
        out.write("cycles=10000000\n");
        out.write("cycleup=10\n");
        out.write("cycledown=20\n");       
        out.write("[mixer]\n\n");        
        out.write("nosound=false\n");
        out.write("rate=44100\n");
        out.write("blocksize=1024\n");
        out.write("prebuffer=20     \n");  
        out.write("[midi]\n\n");       
        out.write("mpu401=intelligent\n");     
        out.write("mididevice=default\n");
        out.write("midiconfig=\n");       
        out.write("[sblaster]\n\n");        
        out.write("sbtype=sb16\n");
        out.write("sbbase=220\n");
        out.write("irq=7\n");
        out.write("dma=1\n");
        out.write("hdma=5\n");
        out.write("sbmixer=true\n");
        out.write("oplmode=auto\n");
        out.write("oplemu=default\n");
        out.write("oplrate=44100\n");
        out.write("[gus]\n\n");        
        out.write("gus=false\n");
        out.write("gusrate=44100\n");
        out.write("gusbase=240\n");
        out.write("gusirq=5\n");
        out.write("gusdma=3\n");
        out.write("ultradir=C:"+File.separator+"ULTRASND\n");        
        out.write("[speaker]\n\n");        
        out.write("pcspeaker=true\n");
        out.write("pcrate=44100\n");
        out.write("tandy=auto\n");
        out.write("tandyrate=44100\n");
        out.write("disney=true\n");        
        out.write("[joystick]\n\n");        
        out.write("joysticktype=auto\n");
        out.write("timed=true\n");
        out.write("autofire=false\n");
        out.write("swap34=false\n");
        out.write("buttonwrap=false\n");       
        out.write("[serial]\n\n");        
        out.write("serial1=dummy\n");
        out.write("serial2=dummy\n");
        out.write("serial3=disabled\n");
        out.write("serial4=disabled\n");        
        out.write("[dos]\n\n");
        out.write("xms=true\n");
        out.write("ems=true\n");
        out.write("umb=true\n");
        out.write("keyboardlayout=auto\n");
        out.write("[ipx]\n\n");        
        out.write("ipx=false\n");         
        out.write("[autoexec]\n"); 
        out.write("@ECHO OFF\n");     
        out.write("z:mount d \""+buildPath+"\"\n");
        out.write("z:mount h \""+idepath+standapipath+"\"\n");
        out.write("z:mount g \""+idepath+standlibrariepath+"\"\n");
        out.write("z:mount f \""+idepath+"/libraries\"\n");
        out.write("z:mount e \""+idepath+"/hardware/86duino/x86/variants/"+prefs.get("build.variant")+"\"\n");
        out.write("z:mount c \""+idepath+djgpppath+"\"\n");
        out.write("CLS\n");
        out.write("c:\n");
        out.write("call djgpp.bat\n");
        out.write("d:\n");
        out.write("echo ===============================================================================\n");
        out.write("echo     _____                               __   ___    __                         \n");
        out.write("echo   /'_____`\\                            /\\_\\ /\\_ \\  /\\_\\                        \n");
        out.write("echo  /\\ \\_____/     __   _  __  __   _  ___\\/_/_\\//\\ \\ \\/_/_  _  __      ____      \n");
        out.write("echo  \\ \\ \\        /'__`\\/\\`' _`\\ _`\\/\\ '___`\\/\\ \\ \\ \\ \\  /\\ \\/\\`' _`\\  /'___ \\     \n");
        out.write("echo   \\ \\ \\_____ /\\ \\_\\ \\ \\ \\ \\ \\ \\ \\ \\ \\__\\ \\ \\ \\ \\_\\ \\_\\ \\ \\ \\ \\ \\ \\/\\ \\__\\ \\    \n");
        out.write("echo    \\ \\______`\\ \\____/\\ \\_\\ \\_\\ \\_\\ \\  ___/\\ \\_\\/\\____\\\\ \\_\\ \\_\\ \\_\\ \\____  \\   \n");
        out.write("echo     \\/_______/\\/___/  \\/_/\\/_/\\/_/\\ \\ \\_/  \\/_/\\/____/ \\/_/\\/_/\\/_/\\/____/\\ \\  \n");
        out.write("echo                                    \\ \\ \\                               __\\_\\ \\ \n");
        out.write("echo                                     \\ \\_\\                             /\\______\\\n");
        out.write("echo                                      \\/_/                             \\/______/\n");
        out.write("echo ===============================================================================\n");
        out.write("echo                           Compiling, please wait...                             \n");
        out.write("redir -o message.txt -eo make\n");
        out.write("exit\n");
        out.close(); 
      } 
      catch (IOException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
      }
    }  
  }
  void writeMakefile(String buildPath) throws RunnerException {
    String filePath = buildPath + File.separator + "Makefile";
    File file = new File(filePath);
    if(file.exists()){
      file.delete();
    }
    if(!file.exists())
      {
      try {
        file.createNewFile();
        BufferedWriter out = new BufferedWriter(new FileWriter(file));
        out.write("# Makefile for DX2/EX MCM \n");
        out.write("\n");
        out.write("IFALGS    = -Id:/ -Ie:/ -Ih:/ ");
        out.write(makefilesourcepath+"\n");
        out.write("INIFILES  = ");
        for(int i = 0; i < file_path.size() ; i++){
          String str_standard_path=file_path.get(i).substring(0,file_path.get(i).lastIndexOf("."));
          out.write(str_standard_path+".o" + " ");
        }
        out.write("\n");
        out.write("EXEFILES  = 86duino.exe\n");
        out.write("OBJFILES  = c:/compile/main.o\n");
        out.write("LIBFILES  = -lstd~1 "+str_libfiles+"\n");
        //out.write("LIBFILES  = -lstd~1 -L g:/ethernet/utility -lwatt\n");
        out.write("\n");
        out.write(".PHONY : everything all clean \n\n");
        out.write("everything : $(INIFILES) $(EXEFILES)\n\n");
        out.write("all : clean everything\n\n");
        out.write("clean :\n\t-rm -f $(INIFILES) $(EXEFILES) $(OBJFILES)\n\n");
        out.write("-include $(INIFILES:.o=.d)\n\n");
        String filenamee;
        for(int i = 0; i < file_path.size(); i++){
          out.write(file_path.get(i).substring(0,file_path.get(i).lastIndexOf("."))+".o "+file_path.get(i).substring(0,file_path.get(i).lastIndexOf("."))+".d "+": "+file_name.get(i)+"\n\tgcc -w -c -MMD -fno-exceptions -ffunction-sections -fdata-sections -O1 -o "+file_path.get(i).substring(0,file_path.get(i).lastIndexOf("."))+".o "+"$< $(IFALGS)\n");      
          }
        out.write("\n");
        out.write("86duino.exe : $(INIFILES)\n\tgcc -w -Wl,--gc-sections -O1 -o $@ $(INIFILES) $(LIBFILES)\n");
        out.write("\tstrip -v 86duino.exe");
        out.write("\n");
        out.close(); 
      } 
      catch (IOException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
      }
    }  
  }
  void mainHave(String buildPath) throws RunnerException {
    String filePath = buildPath + File.separator + "86DUINO.EXE";
    File file = new File(filePath);
    if(file.exists()){
      file.delete();
    }
    if(Base.isLinux() || Base.isMacOS()){
      filePath = buildPath + File.separator + "86duino.exe";
      file = new File(filePath);
      if(file.exists()){
        file.delete();
      }
    }
  }
  boolean compileSusses(String buildPath) throws RunnerException {
    String filePath = buildPath + File.separator + "86DUINO.EXE";
    String filePath1 = buildPath + File.separator + "86duino.exe";
    File file = new File(filePath);
    File file1 = new File(filePath1);
    if(file.exists()){
      if(Base.isLinux() || Base.isMacOS()){
        if(file.renameTo(file1)==true) 
          return true;
        else
          return false;
      }  
      return true; 
    }
    else{
      return false;
    }
  }
  void writeMakeMessage(String path) throws RunnerException {
     try{
        Preferences preferences= new Preferences();      
        FileReader fr = new FileReader(path);
        BufferedReader br = new BufferedReader(fr);
        String readoneline;
        while((readoneline = br.readLine()) != null){     
          if(readoneline   !=  null ){     
            readoneline=readoneline.replace("d:/",tmppath.replace("\\","/")+"/");
            readoneline=readoneline.replace("h:/",idepath+standapipath+"/");
            readoneline=readoneline.replace("g:/",idepath+standlibrariepath+"/");
            readoneline=readoneline.replace("f:/",idepath+"/libraries/");
            readoneline=readoneline.replace("e:/",idepath+"/hardware/86duino/x86/variants/"+prefs.get("build.variant")+"/");
            readoneline=readoneline.replace("c:/",idepath+djgpppath+"/");
            readoneline=readoneline.replace("_eman_8",primaryClassName);
            String[] ary_readoneline = readoneline.split(" ");
            if(ary_readoneline[0].compareTo("gcc")==0 || ary_readoneline[0].compareTo("strip")==0 || ary_readoneline[0].compareTo("copy")==0){
              if(preferences.getBoolean("build.verbose"))
                System.out.println(readoneline);
            } 
            else{
              String errorFormat = "[\\w\\d_]+.\\w+:\\s*\\*\\*\\*\\s*Warning:\\s*File\\s*\\`.*\\'\\s*(.*)\\s*\\(.*\\)\\s*";
              String[] pieces = PApplet.match(readoneline, errorFormat);
              //[\\w\\d_]+.\\w+:\\s*\\*\\*\\*\\s*Warning:\\s*File\\s*\\`.*\\'\\s*(.*)\\s*\\(.*\\)\\s*
              //has modification time in the future
              errorFormat="[\\w\\d_]+.\\w+:\\s*warning:\\s*(.*)\\.\\s*.*\\.\\s*";
              String[] piecess = PApplet.match(readoneline, errorFormat);
              //[\w\d_]+.\w+:\s*warning:\s*(.*)\.\s*.*\.\s*
              //Clock skew detected
              if (pieces == null && piecess==null)
                message(readoneline);
              else{                 
                if (pieces != null)
                  if (!pieces[1].trim().equals("has modification time in the future"))
                    message(readoneline);
                else if (piecess != null)
                  if (!piecess[1].trim().equals("Clock skew detected"))
                    message(readoneline);
              }
            }
          }               
        }     
        br.close();                                   
        br.close();
        fr.close();
        if(!Verbord){
          RunnerException re = new RunnerException(_(strVerbord));
          re.hideStackTrace();
          throw re;
        }
    }catch(IOException e){
        e.printStackTrace();
    }
  }
  private void chngeIncludeData(String Path)throws RunnerException{
    try{
      FileReader fr = new FileReader(Path+File.separator+"changeinclude.txt");
      BufferedReader br = new BufferedReader(fr);
      String readoneline;
      while((readoneline = br.readLine()) != null){     
        if(readoneline   !=  null ){     
          IncludeName.add(readoneline);   
        }               
      }     
      br.close();
      fr.close();
    }catch(IOException e){
        e.printStackTrace();
    }
  }
  private void chngeIncludelibaData(String Path)throws RunnerException{
    try{
      FileReader fr = new FileReader(Path+File.separator+"changeliba.txt");
      BufferedReader br = new BufferedReader(fr);
      String readoneline;
      while((readoneline = br.readLine()) != null){     
        if(readoneline   !=  null ){     
          Includeliba.add(readoneline);   
        }               
      }     
      br.close();
      fr.close();
    }catch(IOException e){
        e.printStackTrace();
    }
  }
  public void chngeInclude(String path,String filename) throws RunnerException {
     try{
        String a,b;
        File f = new File(path+File.separator+"aaa.txt");
        FileReader fr = new FileReader(path+File.separator+filename);
        f.createNewFile();
        BufferedWriter out = new BufferedWriter(new FileWriter(f));      
        BufferedReader br = new BufferedReader(fr);
        String readoneline;
        while((readoneline = br.readLine()) != null){     
          if(readoneline   !=  null ){
            for(int i = 0; i < IncludeName.size() ; i+=2){ 
              readoneline=readoneline.replaceAll(IncludeName.get(i),"\n#include <"+IncludeName.get(i+1)+">\n");
              
            }
            for(int i = 0; i < Includeliba.size() ; i+=2){ 
              if(readoneline.matches(Includeliba.get(i))){
                str_libfiles += Includeliba.get(i+1);
                str_libfiles += " ";
              }
            }
            out.write(readoneline+"\n");   
          }               
        }
        br.close();     
        br.close();
        fr.close();
        out.close();
        File file = new File(path+File.separator+filename);
        file.delete();
        f.renameTo(file);
    }catch(IOException e){
        e.printStackTrace();
    }
  } 
  private static String preparePaths(List<String> includePaths) {
    String res = "";
    for (String p : includePaths)
      res += " \"-I" + p + '"';

    // Remove first space
    return res.substring(1);
  }
  
  public PreferencesMap getBuildPreferences() {
    return prefs;
  }
}
