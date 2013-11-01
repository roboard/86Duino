/*
  SD - SD Card library for 86Duino
  Copyright (c) 2013 Vic Chen <vic@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
  char *filepath_m;
  
  if(filepath == NULL)
	return File();
  else if(filepath[0] != '\0' && filepath[1] == ':')//check a: b:
  {
    if(filepath[0] == 'a' || filepath[0] == 'A' || filepath[0] == 'b' || filepath[0] == 'B')
	  return File();
	filepath_m = (char *)malloc(sizeof(char) * strlen(filepath) + 1);
	strncpy(filepath_m, filepath, strlen(filepath));
	filepath_m[strlen(filepath) + 1] = '\0';
  }
  else //add c:
  {
	if(filepath[0] != '/')
	{
		filepath_m = (char *)malloc(sizeof(char) * strlen(filepath) + 3 + 1);
		filepath_m[2] = '/';
		strncpy(filepath_m + 3, filepath, strlen(filepath));
		filepath_m[strlen(filepath) + 3] = '\0';
	}
	else
	{
		filepath_m = (char *)malloc(sizeof(char) * strlen(filepath) + 2 + 1);
		strncpy(filepath_m + 2, filepath, strlen(filepath));
		filepath_m[strlen(filepath) + 2] = '\0';
	}
		
	filepath_m[0] = 'c';
	filepath_m[1] = ':';
	
  }

  File file; 
  
  if((strlen(filepath_m) == 3) && (filepath_m[0] != '\0') && (filepath_m[1] == ':') && (filepath_m[2] == '/')) //root (special case)
  {
	if(!file.open("", filepath_m, mode, true))
	{
		return File();
	}
	return File(file, filepath_m);
  }
  int length = strlen(filepath_m);

  
  //delete '/'
  bool is_dir = false;
  if(filepath_m[length - 1] == '/') //directory
  {
	is_dir = true;
	for(int i = length-1;; --i)
	{
		if(filepath_m[i] == '/')
		{
			filepath_m[i] = '\0';
			length--;
		}
		else
			break;
	}
  }
  
  //get parent path
  char *token = strchr(filepath_m, '/');
  char buffer[1024] = {0};
  int buffer_idx = 0;
  int path_idx = 0;
  while(token != NULL)
  {
		
	strncpy(buffer + buffer_idx, &filepath_m[path_idx], token-filepath_m - path_idx);
	buffer_idx = buffer_idx + token - filepath_m - path_idx;
	buffer[buffer_idx++] = '/';//add '/' if directory
	buffer[buffer_idx] = '\0';
	path_idx = token-filepath_m;
	path_idx++;
	token = strchr(token + 1, '/');
  }
  if(is_dir)
  {
    file.open(buffer, &filepath_m[path_idx], mode, true);
  }
  else
  {
	bool result = file.open(buffer, &filepath_m[path_idx], mode, false);

	if(result==false)
	{ 
	  file.open(buffer, &filepath_m[path_idx], mode, true);
	} 
  }
  strncpy(buffer, filepath_m, length);
  free(filepath_m);

  if(file)
  {
	return File(file, buffer);
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

static bool mid_mkdir(char *filepath)
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

static bool mid_rmdir(char *filepath)
{
	return rmdir(filepath);
}

bool SDClass::rmdir(char *filepath) {
  return mid_rmdir(filepath);
}

static bool mid_remove(char *filepath)
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
