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

#include <SD.h>
#include <unistd.h>

#if defined (SD_REMOTE)

#include "ipcnet.h"
#include "vdpmem.h"

static int rc;
static unsigned char rxbuf[4096];
static unsigned char txbuf[4096];
extern int __fd; // get from SD.cpp
extern VDPM_HANDLE_t vdppSlave;

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

// make a fake File to use
File::File(char* s, int n) {
    strncpy(_name, s, n);
    _name[n] = '\0';
}


bool File::isDirectory(void) {
    int r;
    unsigned char _r[1];
    
    if (_name[0] == '\0') return false;
    
    send_command_para(6, (uint8_t*)_name, sizeof(_name)); // 6 : "File.isDirectory" command
    
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 0) // open "OK"
            return true;
        else
            return false;
    }
    return false;
}

size_t File::write(uint8_t val) {
    int i, r;
    unsigned char tmp[1024], _r[4];
    
    if (_name[0] == '\0') return 0;
    
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; _name[i] != '\0'; i++) tmp[i] = _name[i];
    tmp[i++] = '\0';
    tmp[i++] = val;
    tmp[i++] = _wtflush ? 1 : 0;
    
    send_command_para(7, (uint8_t*)tmp, i); // 7 : "File.write" command
    
    r = recv_command_result(_r, 4);
    if (r == 0) {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }
    
    return 0;
}

size_t File::write(const uint8_t *buf, size_t size) {
    int i, j, r;
    unsigned char tmp[1024], _r[4];
    
    if (_name[0] == '\0') return 0;
    
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; _name[i] != '\0'; i++) tmp[i] = _name[i];
    tmp[i++] = '\0';
    for (j=0; j<4; i++, j++) tmp[i] = (size >> (j*8)) & 0xFF;
    
    // vdpm write
    vdpm_MemWrite(vdppSlave, 0, (uint8_t*)buf, size);

    // talk to master to read them and call write()
    send_command_para(20, (uint8_t*)tmp, i);

    r = recv_command_result(_r, 4);
    if (r == 0) {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }
    
    return 0;
}

int File::peek() {
    int r;
    unsigned char _r[4];
    
    if (_name[0] == '\0') return 0;
    
    send_command_para(8, (uint8_t*)_name, sizeof(_name)); // 8 : "File.peek" command
    
    r = recv_command_result(_r, 4);
    if (r == 0) {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }
    
    return 0;
}

int File::read() {
    int r;
    unsigned char _r[4];
    
    if (_name[0] == '\0') return 0;
    
    send_command_para(9, (uint8_t*)_name, sizeof(_name)); // 9 : "File.read" command
    
    r = recv_command_result(_r, 4);
    if (r == 0) {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }
    
    return 0;
}

int File::read(void *buf, uint16_t nbyte) {
    int i, r;
    unsigned char tmp[1024], _r[4];
    
    if (_name[0] == '\0') return 0;
    
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; _name[i] != '\0'; i++) tmp[i] = _name[i];
    tmp[i++] = '\0';
    tmp[i++] = nbyte & 0xFF;
    tmp[i++] = (nbyte >> 8) & 0xFF;
    
    // talk to master start to read data and write into RAM
    send_command_para(21, (uint8_t*)tmp, i);
    r = recv_command_result(_r, 4);
    if (r == 0) {
        r = fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
        vdpm_MemRead(vdppSlave, 0, (uint8_t*)buf, r);
        return r;
    }
    
    return 0;
}

int File::available() {
    int r;
    unsigned char _r[4];
    
    if (_name[0] == '\0') return 0;
    
    send_command_para(10, (uint8_t*)_name, sizeof(_name)); // 10 : "File.available" command
    
    r = recv_command_result(_r, 4);
    if (r == 0) {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }
    
    return 0;
}

void File::flush() {
    int r;
    unsigned char _r[1];
    
    if (_name[0] == '\0') return;
    
    send_command_para(11, (uint8_t*)_name, sizeof(_name)); // 11 : "File.available" command
    
    r = recv_command_result(_r, 1);
    if (r == 0)
        printf("flush OK\n");
    else
        printf("flush fail\n");
}

bool File::seek(uint32_t pos) {
    int i, r, j = 0;
    unsigned char _r[1], tmp[1024];
    
    if (_name[0] == '\0') return false;
    
    for (i=0; _name[i] != '\0'; i++) tmp[i] = _name[i];
    tmp[i++] = '\0';
    for (j=0; j<4; j++, i++) tmp[i] = (pos >> (j*8)) & 0xFF;
    
    send_command_para(19, tmp, i); // 6 : "File.isDirectory" command
    
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 0) // open "OK"
            return true;
        else
            return false;
    }
    return false;
}

uint32_t File::position() {
    int r;
    unsigned char _r[4];
    
    if (_name[0] == '\0') return 0;
    
    send_command_para(12, (uint8_t*)_name, sizeof(_name)); // 12 : "File.position" command
    
    r = recv_command_result(_r, 4);
    if (r == 0) {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }
    
    return 0;
}

uint32_t File::size() {
    int r;
    unsigned char _r[4];
    
    if (_name[0] == '\0') return 0;
    
    send_command_para(13, (uint8_t*)_name, sizeof(_name)); // 13 : "File.size" command
    
    r = recv_command_result(_r, 4);
    if (r == 0) {
        return fourBytes2Int(_r[0], _r[1], _r[2], _r[3]);
    }
    
    return 0;
}

void File::close() {
    int r;
    unsigned char _r[1];
    
    if (_name[0] == '\0') return;
    
    send_command_para(14, (uint8_t*)_name, sizeof(_name)); // 14 : "File.close" command
    
    r = recv_command_result(_r, 1);
    if (r == 0)
        printf("close OK\n");
    else
        printf("close fail\n");
}

File::operator bool() {
    int r;
    unsigned char _r[1];

    if (_name[0] == '\0') return false;

    send_command_para(15, (uint8_t*)_name, sizeof(_name)); // 15 :  command
    
    r = recv_command_result(_r, 1);
    if (r == 0 && _r[0] == 0)
        return true;
    
    return false;
}

bool File::isOpen() {
    int r;
	unsigned char _r[1];
    
    if (_name[0] == '\0') return false;
    
    send_command_para(16, (uint8_t*)_name, sizeof(_name)); // 16 : "File.isOpen" command
    
    r = recv_command_result(_r, 1);
    if (r == 0 && _r[0] == 0)
        return true;
    
    return false;
}

bool File::open(const char *parent, const char *name, int type, bool dir) {
    // empty
    return true;
}

// allows you to recurse into a directory
File File::openNextFile(uint8_t mode) {
    unsigned char tmp[1024], _r[14], r;
    int i;
    
    // Notic : the command is combined by filepath and mode,
    //              so Master side must know the lenght of filepath is (len - 1)
    for (i=0; i<1024; i++) tmp[i] = '\0';
    for (i=0; _name[i] != '\0'; i++) tmp[i] = _name[i];
    tmp[i++] = '\0';
    tmp[i++] = mode; // this causes the lenght "i" is [len(filepath) + 1]
    
    send_command_para(17, (uint8_t*)tmp, i); // 1 : "File.openNextFile" command
    
    r = recv_command_result(_r, 1024);
    if (r == 0) {
        if (_r[0] == 0) // open "OK"
        {
            for (i = 1; _r[i] != '\0'; i++) tmp[i-1] = _r[i];
            tmp[i-1] = '\0';
            // printf("openNextFile OK, file name: %s\n", tmp);
            return File((char*)tmp, i); //  ==> fake File
        }
        else
        {
            printf("openNextFile fail\n");
            return File();
        }
    }
    
    return File();
}

void File::rewindDirectory(void) { 
    int r;
    unsigned char _r[1];
    
    send_command_para(18, (uint8_t*)_name, sizeof(_name)); // 18 : "File.rewindDirectory" command
    
    r = recv_command_result(_r, 1);
    if (r == 0)
        printf("rewindDirectory OK\n");
    else
        printf("rewindDirectory fail\n");
}

int File::eof(void) {
    int r;
    unsigned char _r[1];
    
    send_command_para(19, (uint8_t*)_name, sizeof(_name)); // 19 : "File.eof" command
    
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 1) {
            printf("get EOF\n");
            return 1;
        } else
            return 0;
    } else
        printf("get EOF fail\n");
    
    return 0;
}

#else

// a directory is a special type of file
bool File::isDirectory(void) {
  return _dir != NULL;
}

size_t File::write(uint8_t val) {
  int r;
  
  if (!_file || (_type == O_READ))
    return 0;
  
  r = fwrite(&val, 1, 1, _file);
  
  if (_wtflush) {
    fflush(_file);
    fsync(fileno(_file));
  }
  
  return r;
}

size_t File::write(const uint8_t *buf, size_t size) 
{
  static int counter = 0;
  
  int t = 0, i = 0;
  if (!_file || (_type == O_READ)) {
    return 0;
  }
  
  for(i = 0; i < size; ++i)
  {
	t += fwrite((buf + i), 1, 1, _file);
  }
  // flush();
  fflush(_file);
  fsync(fileno(_file));
  return t;
}

int File::peek() {
  if (!_file || !(_type & O_READ)) 
    return 0;

  fpos_t pos;
  int c, t;
  fgetpos(_file, &pos);
  t = fread(&c, 1, 1, _file);
  if (t == 1) 
	fsetpos(_file, &pos);
  return c;
}

int File::read() {
  if (_file && (_type & O_READ)) 
  {
    char c;
    if (fread(&c, 1, 1, _file) == 1) {
        return c;
    } else
        return -1;
  }
  return -1;
}

// buffered read for more efficient, high speed reading
int File::read(void *buf, uint16_t nbyte) {
  if (_file && (_type & O_READ)) 
  {
    return fread(buf, 1, nbyte, _file);
  }
  return 0;
}

int File::available() {
  if (! _file) return 0;
  
  uint32_t n = size() - position();
  return n > 0X7FFF ? 0X7FFF : n;
}

void File::flush() {

  if (_file)
  {
    fflush(_file);
	fsync(fileno(_file));
  }
}

bool File::seek(uint32_t pos) {
  if (! _file) return false;
  return !(fseek(_file, pos, SEEK_SET));
}

unsigned long File::seekLineNumber(uint32_t index, uint32_t maxlength){
	if (! _file) return 0;
	char *ptr, *buf;
	unsigned long read_len;
	unsigned long line_num = 0;
	buf = (char*)malloc(sizeof(char)* maxlength);
	if(buf == NULL)
		return 0;
	
	while(index > 0){
		read_len = index < maxlength ? index + 1 : maxlength;
		ptr = fgets(buf, read_len, _file);
		if(ptr == NULL)
			break;
		line_num++;
		index -= strlen(buf);
		// printf("buf = %s, remain index = %lu\n", buf, index);
	}
	if(ptr != NULL && buf[strlen(buf) - 1] == '\n')
		line_num++;
	
	free(buf);
	return line_num;
}

uint32_t File::position() {
  if (! _file) return -1;
  return ftell(_file);
}

uint32_t File::size() {
  if (_file == NULL) 
	return 0;
  
  long pos = ftell(_file);
  long len = 0;
  fseek(_file, 0L, SEEK_END);
  len = ftell(_file);
  fseek(_file, pos, SEEK_SET);
  return len;
}

void File::close() {
  if (_file) {
    fclose(_file);
    _file = NULL;
  }
  else if (_dir)
  {
	closedir(_dir);
	_dir = NULL;
  }
}

File::operator bool() {
  return (_file != NULL) || (_dir != NULL);
}



bool File::open(const char *parent, const char *name, int type, bool dir)
{
	
	char temp_name[1024];
    int _strn = strlen(name);
	sprintf(temp_name, "%s%s", parent, name);
    //printf("temp_name: %s\n", temp_name);
	if(dir)
	{
		if((type & O_WRITE))
		{
			return false;
		}
		if((_dir = opendir(temp_name)) == NULL)
		{
			return false;
		}
		strncpy(_parent, parent, 1023);
		_parent[1023] = '\0';
		strncpy(_name, name, _strn);
		_name[_strn] = '\0';
		_type = type;
		return true;
	}
	if(type == O_READ)
	{
		_file = fopen(temp_name, "rb");
	}
	else if(type == (O_READ | O_WRITE | O_CREAT))
	{
		_file = fopen(temp_name, "ab+");
		if(_file == NULL)
		{
			_file = fopen(temp_name, "w+b");
		}
	}
	else if(type == O_APPEND)
		_file = fopen(temp_name, "ab");
	else if(type == O_RDWR)
		_file = fopen(temp_name, "r+b");
	else if(type == (O_WRITE | O_APPEND))
		_file = fopen(temp_name, "ab");
	else if(type == (O_RDWR | O_CREAT))
		_file = fopen(temp_name, "w+b");
	else if(type == (O_APPEND | O_RDWR))
		_file = fopen(temp_name, "a+b");
	else if(type == (O_WRITE | O_TRUNC))
	{
		if(!(_file = fopen(temp_name, "w")))
			return false;
		//fclose(_file);
		//return true;
	}
	else if(type == (O_CREAT | O_EXCL))
		return false;
	else
		return false;
    
	if(!_file)	
		return false;

	// flags_ = oflag & (O_ACCMODE | O_SYNC | O_APPEND);
	_type = type;
	strncpy(_parent, parent, 1023);
	_parent[1023] = '\0';
	strncpy(_name, name, _strn);
	_name[_strn] = '\0';
	return true;
}

bool File::isOpen()
{
	return (_dir || _file);
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

int File::eof(void) {
    if (_file)  
        return feof(_file);
    return 1;
}
#endif
/*
File& File::operator=(File rhs)
{
  strncpy(_parent, rhs._parent, 1023);
  _parent[1023] = '\0';
  strncpy(_name, rhs._name, 12);
  _name[12] = '\0';
  _type = rhs._type;
  
  if(rhs._file != NULL)
  {
    _file = rhs._file;
	_dir = NULL;
  }
  else if(rhs._dir != NULL)
  {
    _dir = rhs._dir;
	_file = NULL;
  }
  
  _wtflush = true;
}
*/
File::File(File f, const char *n) {
  // oh man you are kidding me, new() doesnt exist? Ok we do it by hand!
  int _strn = strlen(n);
  strncpy(_parent, f._parent, 1023);
  _parent[1023] = '\0';
  strncpy(_name, n, _strn);
  _name[_strn] = '\0';
  _type = f._type;
  
  
  if(f._file != NULL)
  {
	_file = f._file;
	_dir = NULL;
  }
  else if(f._dir != NULL)
  {
	_dir = f._dir;
	_file = NULL;
  }
  
  _wtflush = true;
}

File::File(void) {
  _parent[0] = '\0';
  _file = NULL;
  _dir = NULL;
  _name[0] = '\0';
  _type = 0;
  _wtflush = true;
}

File::~File(void) {
}

// returns a pointer to the file name
char *File::name(void) {
  return _name;
}

char *File::parent(void) {
  return _parent;
}

void File::setWriteFileFlush(bool f) {
    _wtflush = f;
}

// use standard C function
static bool mid_remove(char *filepath)
{
	return (remove(filepath) == 0);
}

bool File::remove(void) {
    char realPath[1024];
    bool ret = false;
    bool result = ((name())[0] != '\0');
    if (result && (strlen(name()) > 0) && (strlen(parent()) > 0)) {
        sprintf(realPath, "%s%s", parent(), name());
    }
    
    if (strlen(realPath) > 0) {
        ret = mid_remove(realPath);
        close();
    }
    
    return ret;
}


