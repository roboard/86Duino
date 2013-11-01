#ifndef __SD_H__
#define __SD_H__

#include <Arduino.h>
#include <dirent.h>

#define FILE_READ O_READ
#define FILE_WRITE (O_READ | O_WRITE | O_CREAT)


// use the gnu style oflag in open()
/** open() oflag for reading */
uint8_t const O_READ = 0X01;
/** open() oflag - same as O_READ */
uint8_t const O_RDONLY = O_READ;
/** open() oflag for write */
uint8_t const O_WRITE = 0X02;
/** open() oflag - same as O_WRITE */
uint8_t const O_WRONLY = O_WRITE;
/** open() oflag for reading and writing */
uint8_t const O_RDWR = (O_READ | O_WRITE);
/** open() oflag mask for access modes */
uint8_t const O_ACCMODE = (O_READ | O_WRITE);
/** The file offset shall be set to the end of the file prior to each write. */
uint8_t const O_APPEND = 0X04;
/** synchronous writes - call sync() after each write */
uint8_t const O_SYNC = 0X08;
/** create the file if nonexistent */
uint8_t const O_CREAT = 0X10;
/** If O_CREAT and O_EXCL are set, open() shall fail if the file exists */
uint8_t const O_EXCL = 0X20;
/** truncate the file to zero length */
uint8_t const O_TRUNC = 0X40;


class File : public Stream{

private:
  char _name[13]; // our name
  FILE *_file;  // underlying file pointer
  DIR  *_dir;   // dir pointer
  int  _type;   // file type
  char _parent[1024]; //parent path
  

public:
  
  
  File(File f, const char *name);     // wraps an underlying SdFile
  File(void);      // 'empty' constructor
  ~File(void);     // destructor
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int read();
  virtual int peek();
  virtual int available();
  virtual void flush();
  int read(void *buf, uint16_t nbyte);
  bool seek(uint32_t pos);
  uint32_t position();
  uint32_t size();
  void close();
  operator bool();
  File& operator=(File);
  char * name();

  bool isDirectory(void);
  File openNextFile(uint8_t mode = O_RDONLY);
  void rewindDirectory(void);
  
  bool open(const char *parent, const char *name, int type, bool dir);
  bool isOpen();
  
  using Print::write;
};

class SDClass {

private:
  // These are required for initialisation and use of sdfatlib
  //Sd2Card card;
  //SdVolume volume;
  //SdFile root;
  
  // my quick&dirty iterator, should be replaced
  File getParentDir(const char *filepath, int *indx);
  
public:
  // This needs to be called to set up the connection to the SD card
  // before other methods are used.
  bool begin(uint8_t csPin = 0);
  
  // Open the specified file/directory with the supplied mode (e.g. read or
  // write, etc). Returns a File object for interacting with the file.
  // Note that currently only one file can be open at a time.
  File open(const char *filename, uint8_t mode = FILE_READ);

  // Methods to determine if the requested file path exists.
  bool exists(char *filepath);

  // Create the requested directory heirarchy--if intermediate directories
  // do not exist they will be created.
  bool mkdir(char *filepath);
  

  // Delete the file.
  bool remove(char *filepath);
  
  bool rmdir(char *filepath);

private:

  // This is used to determine the mode used to open a file
  // it's here because it's the easiest place to pass the 
  // information through the directory walking function. But
  // it's probably not the best place for it.
  // It shouldn't be set directly--it is set via the parameters to `open`.
  int fileOpenMode;
  
  friend class File;
  friend bool callback_openPath(File&, char *, bool, void *); 
};

extern SDClass SD;




#endif
