#include "SPIFlash.h"

//SPI flash control InterFace
class EEPROMBlock
{
public:
  EEPROMBlock(unsigned short in_addr);
  ~EEPROMBlock();
  
  //operations
  void write(unsigned short int in_addr, unsigned char in_data);
  unsigned char read(unsigned short int in_addr);
  
private:
  
  SPIFlash  * _data;
  SPIFlash  * _data_redundancy;
  SPIFlash  * _counter;
  
  unsigned char * _data_buffer;
  
  unsigned short _data_position;
  unsigned short _counter_position;
  
  unsigned short _addr;
  
  unsigned char _redundancy; //check using redundancy or not
  
  void AllocBuffers();
  void CalculateRealData();
  void CheckCounterResetState();
  void SetRedundancyFlag();
  void WriteCounter();
  void WriteCounterRedundancy(unsigned char bit);
};

