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

#if defined (SD_REMOTE)

#include "ipcnet.h"
#include "vdpmem.h"

VDPM_HANDLE_t vdppSlave;

static int rc;
static unsigned char rxbuf[4096];
static unsigned char txbuf[4096];
static unsigned long val, rx_timeout, tx_timeout;
static bool init_OK = false;
int __fd; // share with File.cpp

static void send_command_para(uint8_t c, uint8_t* p, int p_size) {
    int i = 0, cur = 0, j;
    
    // command
    txbuf[i++] = c;
    
    // other parameters
    if (p != NULL) {
        for (j=0; j<p_size; j++)
            txbuf[i++] = *(p++);
    }

    do {
        rc = ipcnet_send(__fd, &txbuf[cur], i - cur);
        if (rc > 0) cur += rc;
    } while (cur != i);
    
}

// -1 : continue
//  0 : complite
static int recv_command_result(uint8_t* mybuf, int num) {
    int i, cur = 0;
    
    do {
        rc = ipcnet_recv(__fd, &rxbuf[cur], num - cur);
        if (rc < 0 && ipcnet_errno(__fd) == IPCNET_ECONNABORTED)
            return -1;
        cur += rc;
    } while (cur != num);
    
    for (i=0; i<num; i++)
        mybuf[i] = rxbuf[i];
    
    return 0;
}

static int fourBytes2Int(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3) {
    int r = 0;
    r = a3;
    r = (r << 8) + a2;
    r = (r << 8) + a1;
    r = (r << 8) + a0;
    return r;
}

static void sd_init(void) {
    int r;
    unsigned char _r[4];
    uint32_t tmp;
    
    val = 1;

    __fd = ipcnet_socket();
    ipcnet_ioctl(__fd, IPCNET_FIONBIO, (unsigned long)&val);

    do {
        rc = ipcnet_connect(__fd, 90); // 90 : SD port
        if (rc == 0)
            break;
        printf("begining...\r");
    } while (1);
    printf("\n begin() OK!\n");
    
    if (vdpm_Initialize(1 * 1024) < 0)
        printf("ERROR: vdpm_Initialize() failed.\n");
    
    send_command_para(0xAA, NULL, 0);
    r = recv_command_result(_r, 4);
    
    if (r == 0) {
        tmp = fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
        vdppSlave = vdpm_GetMemHandle(tmp);
        printf("vdpm_GetMemHandle: %08X\n", vdppSlave);
    } else
        printf("vdpm_GetMemHandle() not be run.\n");
}

bool SDClass::begin(uint8_t csPin) {
    if (!init_OK) { sd_init(); init_OK = true; }
    return true;
}

File SDClass::open(const char *filepath, uint8_t mode) {
    unsigned char tmp[1024], _r[14], r;
    int i;
    
    if (!init_OK) { sd_init(); init_OK = true; }
    
    // Notic : the command is combined by filepath and mode,
    //              so Master side must know the lenght of filepath is (len - 1)
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; filepath[i] != '\0'; i++) tmp[i] = *(filepath + i);
    tmp[i++] = '\0';
    tmp[i++] = mode; // this causes the lenght "i" is [len(filepath) + 1]
    
    send_command_para(1, tmp, i); // 1 : "SD.open" command
    
    r = recv_command_result(_r, 1024);
    
    if (r == 0) {
        if (_r[0] == 0) // open "OK"
        {
            for (i = 1; _r[i] != '\0'; i++) tmp[i-1] = _r[i];
            tmp[i-1] = '\0';
            // printf("Open file OK, file name: %s\n", tmp);
            return File((char*)tmp, i); //  ==> fake File
        }
        else
        {
            printf("SD.open() fail\n");
            return File();
        }
    }
    
    return File();
}

File SDClass::open(char *filepath, uint8_t mode) {
    return open((const char *)filepath, mode);
}

bool SDClass::exists(char *filepath) {
    unsigned char tmp[1024], _r[1], r;
    int i;
    
    if (!init_OK) { sd_init(); init_OK = true; }
    
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; filepath[i] != '\0'; i++) tmp[i] = *(filepath + i);
    tmp[i++] = '\0';
    
    send_command_para(2, tmp, i); // 2 : "SD.exists" command
    
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 0) // open "OK"
            return true;
        else
            return false;
    }
}

bool SDClass::mkdir(char *filepath) {
    unsigned char tmp[1024], _r[1], r;
    int i;
    
    if (!init_OK) { sd_init(); init_OK = true; }
    
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; filepath[i] != '\0'; i++) tmp[i] = *(filepath + i);
    tmp[i++] = '\0';
    
    send_command_para(3, tmp, i); // 3 : "SD.mkdir" command
    
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 0) // open "OK"
            return true;
        else
            return false;
    }
}

bool SDClass::rmdir(char *filepath) {
    unsigned char tmp[1024], _r[1], r;
    int i;
    
    if (!init_OK) { sd_init(); init_OK = true; }
    
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; filepath[i] != '\0'; i++) tmp[i] = *(filepath + i);
    tmp[i++] = '\0';
    
    send_command_para(4, tmp, i); // 4 : "SD.rmdir" command
    
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 0) // open "OK"
            return true;
        else
            return false;
    }
}

bool SDClass::remove(char *filepath) {
    unsigned char tmp[1024], _r[1], r;
    int i;
    
    if (!init_OK) { sd_init(); init_OK = true; }
    
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; filepath[i] != '\0'; i++) tmp[i] = *(filepath + i);
    tmp[i++] = '\0';
    
    send_command_para(5, tmp, i); // 5 : "SD.remove" command
    
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 0) // open "OK"
            return true;
        else
            return false;
    }
}


#else

extern char* sdcardPath;
extern char* emmcDiskPath;
extern char* usbDiskPath;
bool SDClass::begin(uint8_t csPin) {
  /*

    Performs the initialisation required by the sdfatlib library.

    Return true if initialization succeeds, false otherwise.

   */
   return true;
}

SDClass::SDClass() {
    _bank = SDCARD;
}

void SDClass::initPath() {
    static bool _init = false;
    
    if (!_init) {
        FILE* fp;
        
        memset(path_sdcard, '\0', sizeof(path_sdcard)/sizeof(char));
        memset(path_usb, '\0', sizeof(path_sdcard)/sizeof(char));
        memset(path_emmc, '\0', sizeof(path_sdcard)/sizeof(char));
        
        strcpy(path_sdcard, sdcardPath);
        strcpy(path_usb, usbDiskPath);
        strcpy(path_emmc, emmcDiskPath);
        
        fp = fopen("a:\\_brdn.v86", "r");
        if (fp == NULL)
            fp = fopen("b:\\_brdn.v86", "r");

        if (fp != NULL) {
            char boardName[20];
            fgets(boardName, 20, fp);

            if (strstr(boardName, "QEC") != NULL)
                _bank = USBDISK;
            else
                _bank = SDCARD;
            
            fclose(fp);
        }

        _init = true;
    }
}

static bool mid_mkdir(char *filepath);
void SDClass::setBank(int bank) {
    char buf[64];
    
    initPath();
    if (bank > 0 && bank < UNKNOWN) _bank = bank;
    
    // Check if c:/sd directory is exist?
    if (_bank == EMMCDISK) {
        getDiskPath(_bank, buf);
        for (int i=0; buf[i] != '\0'; i++) if (buf[i] == '\\') buf[i] = '/';
        if (!exists(buf)) mid_mkdir(buf); // ex: c:/sd
    }
}

int SDClass::getBank() {
    initPath();
    return _bank;
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
	filepath_m[strlen(filepath)] = '\0';
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

    #if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE)
    if (_bank == SDCARD)
        filepath_m[0] = 'c';
	else if (_bank == USBDISK)
        filepath_m[0] = 'c'; // or 'd' but when?
    else if (_bank == EMMCDISK) {
        Serial.println("This board have not EMMC disk");
        return File();
    } else {
        Serial.println("Unknown disk");
        return File();
    }
    #else
    if (_bank == SDCARD && sdcardPath != NULL && strlen(sdcardPath) > 0) {
        if (strcmp(sdcardPath, "NONE") != 0)
            filepath_m[0] = sdcardPath[0];
        else { // NONE = no SD CARD slot
            Serial.println("This board have not SD CARD slot");
            return File();
        }
	} else if (_bank == USBDISK && usbDiskPath != NULL && strlen(usbDiskPath) > 0) {
        if (strcmp(usbDiskPath, "NONE") != 0)
            filepath_m[0] = usbDiskPath[0];
        else { // NONE = no USB slot
            Serial.println("This board have not USB slot");
            return File();
        }
    } else if (_bank == EMMCDISK && emmcDiskPath != NULL && strlen(emmcDiskPath) > 0) {
        if (strcmp(emmcDiskPath, "NONE") != 0) {
            
            if(filepath[2] != '/')
            {
                free(filepath_m);
                filepath_m = (char *)malloc(sizeof(char) * strlen(filepath) + 6 + 1);
                sprintf(filepath_m + 2, "/sd/");
                strncpy(filepath_m + 6, filepath, strlen(filepath));
                filepath_m[strlen(filepath) + 6] = '\0';
            }
            else
            {
                free(filepath_m);
                filepath_m = (char *)malloc(sizeof(char) * strlen(filepath) + 5 + 1);
                sprintf(filepath_m + 2, "/sd");
                strncpy(filepath_m + 5, filepath, strlen(filepath));
                filepath_m[strlen(filepath) + 5] = '\0';
            }
            
            filepath_m[0] = emmcDiskPath[0];
        } else { // NONE = no EMMC disk
            Serial.println("This board have not EMMC disk");
            return File();
        }
    } else {
        Serial.println("Unknown disk");
        return File();
    }
	#endif
    
    filepath_m[1] = ':';
  }

  File file; 

  if((strlen(filepath_m) == 3) && (filepath_m[0] != '\0') && (filepath_m[1] == ':') && (filepath_m[2] == '/')) //root (special case)
  {
	if(!file.open(filepath_m, "", mode, true)) // record the _parent path
	{
		return File();
	}
	return File(file, ""); // set _name is empty
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
  char *last_slash = token;
  char buffer[1024] = {0};
  int path_idx = 0;
  while(token != NULL)
  {
	last_slash = token;
    token = strchr(token + 1, '/');
  }
  if(last_slash != NULL)
  {
    path_idx = last_slash - filepath_m + 1;
    strncpy(buffer, filepath_m, path_idx);
  }
  buffer[path_idx+1] = '\0';
    
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
  // strncpy(buffer, filepath_m, length);
  sprintf(buffer, "%s", &filepath_m[path_idx]);
  free(filepath_m);

  if(file)
  {
	return File(file, buffer);
  }
  else
	return File();
}

File SDClass::open(char *filepath, uint8_t mode) {
    return open((const char *)filepath, mode);
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

bool SDClass::mkdir(char *dirpath) {
  /*
  
    Makes a single directory or a heirarchy of directories.

    A rough equivalent to `mkdir -p`.
  
   */
    char *orig_path, *mid_path, *temp;
    char buf[128];
    
    orig_path = (char *)calloc(1, 512);
	mid_path = (char *)calloc(1, 512);
    
    getDiskPath(getBank(), buf);
    sprintf(mid_path, "%s", buf);
    sprintf(orig_path, "%s", dirpath);
    
	temp = strtok(orig_path, "/");
	while(temp != NULL)
	{
		strcat(mid_path, temp);
        // printf("mid_path: %s\n", mid_path);
		mid_mkdir(mid_path);
		strcat(mid_path, "/");
		temp = strtok(NULL, "/");
	}
    
    free(orig_path);
    free(mid_path);
    
	return exists(dirpath);
}

// use standard C function
static bool mid_rmdir(char *filepath)
{
	return rmdir(filepath);
}

bool SDClass::rmdir(char *filepath) {
    char realPath[1024];
    
    File file = open(filepath, O_READ);
    bool result = ((file.name())[0] != '\0');
    if (result) {
        sprintf(realPath, "%s%s", file.parent(), file.name());
        filepath = &realPath[0];
    }
    file.close();
    
    return mid_rmdir(filepath);
}

bool SDClass::remove(char *filepath) {
    
    File file = open(filepath, O_READ);
    
    return file.remove();
}

char* SDClass::getDiskPath(int mybank, char* buf) {
    initPath();
    
    switch (mybank) {
    case SDCARD:
        if (buf == NULL) {
            sprintf(_buf, "%s", path_sdcard);
            buf = _buf;
        } else
            sprintf(buf, "%s", sdcardPath);
    break;
    case USBDISK:
        if (buf == NULL) {
            sprintf(_buf, "%s", path_usb);
            buf = _buf;
        } else
            sprintf(buf, "%s", usbDiskPath);
    break;
    case EMMCDISK:
        if (buf == NULL) {
            sprintf(_buf, "%ssd\\", path_emmc);
            buf = _buf;
        } else
            sprintf(buf, "%ssd\\", emmcDiskPath);
    break;
    }
    
    if (strstr(buf, "NONE") != NULL) return NULL;
    
    return buf;
}

#endif

SDClass SD;
