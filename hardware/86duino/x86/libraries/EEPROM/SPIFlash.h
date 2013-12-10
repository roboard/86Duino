#include <stdio.h>

class SPIFlash
{
public:
  SPIFlash(unsigned int in_offset);
  ~SPIFlash();
  
  //operations
  void write(unsigned short int in_addr, unsigned char in_value);
  unsigned char read(unsigned short int in_addr);
  void reset();
  
private:
  int _sector_offset;
};
