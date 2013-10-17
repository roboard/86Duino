/*

 SD - a slightly more friendly wrapper for sdfatlib

 This library aims to expose a subset of SD card functionality
 in the form of a higher level "wrapper" object.

 License: GNU General Public License V3
          (Because sdfatlib is licensed with this.)

 (C) Copyright 2010 SparkFun Electronics


 This library provides four key benefits:

   * Including `SD.h` automatically creates a global
     `SD` object which can be interacted with in a similar
     manner to other standard global objects like `Serial` and `Ethernet`.

   * Boilerplate initialisation code is contained in one method named 
     `begin` and no further objects need to be created in order to access
     the SD card.

   * Calls to `open` can supply a full path name including parent 
     directories which simplifies interacting with files in subdirectories.

   * Utility methods are provided to determine whether a file exists
     and to create a directory heirarchy.


  Note however that not all functionality provided by the underlying
  sdfatlib library is exposed.

 */

/*

  Implementation Notes

  In order to handle multi-directory path traversal, functionality that 
  requires this ability is implemented as callback functions.

  Individual methods call the `walkPath` function which performs the actual
  directory traversal (swapping between two different directory/file handles
  along the way) and at each level calls the supplied callback function.

  Some types of functionality will take an action at each level (e.g. exists
  or make directory) which others will only take an action at the bottom
  level (e.g. open).

 */

#include "SD.h"
#include <unistd.h>
#include <sys/stat.h>

bool SDClass::begin(uint8_t csPin) {
  /*

    Performs the initialisation required by the sdfatlib library.

    Return true if initialization succeeds, false otherwise.

   */
   return true;
}

File SDClass::open(const char *filepath, uint8_t mode) {
  /*

     Open the supplied file path for reading or writing.

     The file content can be accessed via the `file` property of
     the `SDClass` object--this property is currently
     a standard `SdFile` object from `sdfatlib`.

     Defaults to read only.

     If `write` is true, default action (when `append` is true) is to
     append data to the end of the file.

     If `append` is false then the file will be truncated first.

     If the file does not exist and it is opened for writing the file
     will be created.

     An attempt to open a file for reading that does not exist is an
     error.

   */
  File file;
  
  if(strcmp(filepath, "/") == 0) //root (special case)
  {
	file.open("", filepath, mode, true);
	return File(file, filepath);
  }
  int length = strlen(filepath);
  char *temp_path = (char *)malloc(sizeof(char) * length + 1);
  strncpy(temp_path, filepath, length);
  temp_path[length] = '\0';
  
  //delete '/'
  bool is_dir = false;
  if(temp_path[length - 1] == '/') //directory
  {
	is_dir = true;
	for(int i = length-1;; --i)
	{
		if(temp_path[i] == '/')
		{
			temp_path[i] = '\0';
			length--;
		}
		else
			break;
	}
  }
  
  //get parent path
  char *token = strchr(temp_path, '/');
  char buffer[1024] = {0};
  buffer[0] = '/';
  buffer[1] = '\0';
  int buffer_idx = 1;
  int path_idx = 0;
  while(token != NULL)
  {
	if(token == temp_path)
	{
		path_idx++;
		continue;
	}
		
	strncpy(buffer + buffer_idx, &temp_path[path_idx], token-temp_path - path_idx);
	buffer_idx = buffer_idx + token - temp_path - path_idx;
	buffer[buffer_idx++] = '/';//add '/' if directory
	buffer[buffer_idx] = '\0';
	path_idx = token-temp_path;
	path_idx++;
	token = strtok(NULL, "/");
  }
  if(is_dir)
  {
    file.open(buffer, &filepath[path_idx], mode, true);
  }
  else
  {
	bool result = file.open(buffer, &filepath[path_idx], mode, false);
	if(result==false)
	{ 
	  file.open(buffer, &filepath[path_idx], mode, true);
	} 
  }
  
 
  free(temp_path);
  if(file)
  {
	return File(file, filepath);
  }
  else
	return File();
}

bool SDClass::exists(char *filepath) {
  /*

     Returns true if the supplied file path exists.

   */
  File file = open(filepath, O_READ);
  bool result = ((file.name())[0] != '\0');
  file.close();
  return result;
}

bool mid_mkdir(char *filepath)
{
	bool result = mkdir((const char *)filepath, (mode_t)S_IWUSR);
	return result == 0;
}

bool SDClass::mkdir(char *filepath) {
  /*
  
    Makes a single directory or a heirarchy of directories.

    A rough equivalent to `mkdir -p`.
  
   */
    if(exists(filepath))
		return false;
	char *temp;
	int length = strlen(filepath);
	char *orig_path = (char *)malloc(sizeof(char) * length + 1);
	char *mid_path = (char *)malloc(sizeof(char) * length+ 1);
    strncpy(orig_path, filepath, length);
	orig_path[length] = '\0';
	temp = strtok(orig_path, "/");
	while(temp != NULL)
	{
		strcat(mid_path, temp);
		mid_mkdir(mid_path);
		strcat(mid_path, "/");
		temp = strtok(NULL, "/");
	}
	return exists(filepath);
}

bool mid_rmdir(char *filepath)
{
	return rmdir(filepath);
}

bool SDClass::rmdir(char *filepath) {
  return mid_rmdir(filepath);
}

bool mid_remove(char *filepath)
{
	return (remove(filepath) == 0);
}

bool SDClass::remove(char *filepath) {
  return mid_remove(filepath);
}

// allows you to recurse into a directory
File File::openNextFile(uint8_t mode) {
  if(!_dir)
	return File();
  struct dirent *de;
  char buffer[1024];
  
  File f;
  while(de = readdir(_dir))
  {
	if(de->d_name[0] == '.')
		continue;
	
	if(_parent[0] == '\0') //root without parent
	{
		buffer[0] = '/';
		buffer[1] = '\0';
	}
	else if(strcmp(_parent , "/") == 0)
	{
		sprintf(buffer,"%s%s/", _parent, _name);
	}
	else
		sprintf(buffer, "%s%s/", _parent, _name);
	
	if(!f.open((const char *)buffer, de->d_name, O_READ, true))
	{
		if(!f.open((const char *)buffer, de->d_name, O_READ, false))
		{
			return File();
		}	
	}
	return File(f, de->d_name);
  }
  return File();
}

void File::rewindDirectory(void) {  
  if(_dir)
	rewinddir(_dir);
}


SDClass SD;
