/*

 SD - a slightly more friendly wrapper for sdfatlib

 This library aims to expose a subset of SD card functionality
 in the form of a higher level "wrapper" object.

 License: GNU General Public License V3
          (Because sdfatlib is licensed with this.)

 (C) Copyright 2010 SparkFun Electronics

 */

#include <SD.h>

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
}

File::File(File f, const char *n) {
  // oh man you are kidding me, new() doesnt exist? Ok we do it by hand!
  strncpy(_parent, f._parent, 1023);
  _parent[1023] = '\0';
  strncpy(_name, n, 12);
  _name[12] = '\0';
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
}

File::File(void) {
  _parent[0] = '\0';
  _file = NULL;
  _dir = NULL;
  _name[0] = '\0';
  _type = 0;
}

File::~File(void) {
}

// returns a pointer to the file name
char *File::name(void) {
  return _name;
}

// a directory is a special type of file
bool File::isDirectory(void) {
  return _dir != NULL;
}


size_t File::write(uint8_t val) {
  return write(&val, 1);
}

size_t File::write(const uint8_t *buf, size_t size) 
{
  int t;
  if (!_file || (_type == O_READ)) {
    return 0;
  }
  t = fwrite(buf, 1, size, _file);
  flush();
  return t;
}

int File::peek() {
  if (!_file || _type == O_WRITE) 
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
  if (_file || _type == O_WRITE) 
  {
    int c;
    fread(&c, 1, 1, _file);
	return c;
  }
  return -1;
}

// buffered read for more efficient, high speed reading
int File::read(void *buf, uint16_t nbyte) {
  if (_file || _type == O_WRITE) 
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
    fflush(_file);
}

bool File::seek(uint32_t pos) {
  if (! _file) return false;
  return !(fseek(_file, pos, SEEK_CUR));
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
	sprintf(temp_name, "%s%s", parent, name);
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
		strncpy(_name, name, 12);
		_name[12] = '\0';
		_type = type;
		return true;
	}
	if(type == O_READ)
	{
		_file = fopen(temp_name, "rb");
	}
	else if(type == (O_READ | O_WRITE | O_CREAT))
	{
		_file = fopen(temp_name, "ab");
		if(_file == NULL)
		{
			Serial.println("fail");
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
		fclose(_file);
		return true;
	}
	else if(type == (O_CREAT | O_EXCL))
		return false;
	else
		return false;
	if(!_file)
	{	
		return false;
	}
	// flags_ = oflag & (O_ACCMODE | O_SYNC | O_APPEND);
	_type = type;
	strncpy(_parent, parent, 1023);
	_parent[1023] = '\0';
	strncpy(_name, name, 12);
	_name[12] = '\0';
	return true;
}

bool File::isOpen()
{
	return (_dir || _file);
}