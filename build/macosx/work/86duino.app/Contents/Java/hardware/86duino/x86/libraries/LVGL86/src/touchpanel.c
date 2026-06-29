/*---TOUCHKIT.CPP---*/
// test eGalax/EETI touch screen
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pc.h>

typedef int bool;
#define true  (1==1)
#define false (1==0)

#define DEBUG (0)

// write to 64 port
uint8_t kbd_status()
{
  return inportb(0x64);
}
bool kbd_ibf()
{
  uint8_t status = kbd_status();
  if((status & 2))
    return true;
  return false;
}
bool kbd_obf()
{
  uint8_t status = kbd_status();
  // if (DEBUG) printf("%d\n", status & 1);
  if((status & 1))
    return true;
  return false;
}
uint8_t kbd_read()
{
  return inportb(0x60);
}

// write keyboard commad/data, with timeout check
int write_kbd_check_timeout(int port, uint8_t d)
{
  uint32_t clk = clock();
  while((clock() - clk) <= CLOCKS_PER_SEC/2 )
  {
    if(!kbd_ibf())
    {
      outportb(port, d);
      return 0;
    }
  }
  return 1;  // timeout
}
int write_kbd_cmd_check_timeout(uint8_t d)
{  
  return write_kbd_check_timeout(0x64, d);
}
int write_kbd_data_check_timeout(uint8_t d)
{  
  return write_kbd_check_timeout(0x60, d);
}

// read keyboard data, with timeout check
int read_kbd_data_check_timeout(int *kb_ms, uint8_t *d)
{
  uint32_t clk = clock();
  while((clock() - clk) <= CLOCKS_PER_SEC/2 )
  {
    if(kbd_obf())
    {
      *kb_ms = (kbd_status() & 0x20) >> 5;
      *d = kbd_read();     // read data uint8_t
      return 0;
    }
  }
  return 1;  // timeout
}
// read/discard data from keyboard/mouse
void discard_kbd_data()
{
  int st;
  uint8_t d;
  while(read_kbd_data_check_timeout(&st, &d) == 0)
  {
    //cif (DEBUG) printf("discard %d %x\r\n", st, d);
  }
}

int write_touch_screen_cmd(uint8_t cmd, uint8_t *resp_buf)
{
  uint8_t cmdbuf[3];
  int i;
  uint8_t d;
  int st;
  uint8_t resp_len;
  // prepare command packet : 0A 01 cmd
  cmdbuf[0] = 0xa;
  cmdbuf[1] = 0x1;
  cmdbuf[2] = cmd;
  // write command packet
  if (DEBUG) printf("CMD :  ");
  for(i = 0; i < 3; i++)
  {
    if (DEBUG) printf("%02X ", cmdbuf[i]);
    if(write_kbd_cmd_check_timeout((uint8_t)0xd4))
    {
      if (DEBUG) printf("\nCMD D4 : Timeout\n");
      return 1;
    }
    if(write_kbd_data_check_timeout(cmdbuf[i]))
    {
      if (DEBUG) printf("\nDATA : Timeout\n");
      return 1;
    }
    if(read_kbd_data_check_timeout(&st, &d))
    {
      if (DEBUG) printf("\nREAD ACK : Timeout\n");
      return 1;
    }
    if(st != 1 || d!= 0xfa)
    {
      if (DEBUG) printf("\nREAD ACK : Data error\n");
      return 1;
    }
  }
  if (DEBUG) printf("\n");
  // read respond packet
  if (DEBUG) printf("READ : ");
  i = 0;
  while(1)
  {
    if(read_kbd_data_check_timeout(&st, &d))
    {
      if (DEBUG) printf("\nREAD : Timeout\n");
      return 1;
    }
    if (DEBUG) printf("%02X ", d);
    if(!st)
    {
      if (DEBUG) printf("\nREAD : NOT MOUSE\n");
      return 1;
    }
    if(i == 0)
    {
      if(d != 0xa)
      {
        if (DEBUG) printf("RESP 0xA Error\n");
        return 1;
      }
    }
    else if(i == 1)
    {
      resp_len = d;
    }
    else if(i == 2)
    {
      if(d != cmd)
      {
        if (DEBUG) printf("RESP cmd Error\n");
        return 1;
      }
    }
    *resp_buf = d;
    resp_buf++;
    if(i >= 2 && i - 2 >= resp_len - 1)
    {
      break;
    }
    i++;
  } 
  if (DEBUG) printf("\n");
  return 0;
}

int write_touch_screen_cmd_43(uint8_t idx, uint8_t *data)
{
  uint8_t cmdbuf[4];
  int i;
  uint8_t d;
  int st;
  //uint8_t resp_len;
  // prepare command packet : 0A 01 43 idx
  cmdbuf[0] = 0xa;
  cmdbuf[1] = 0x2;
  cmdbuf[2] = 0x43;
  cmdbuf[3] = idx;
  // write command packet
  //if (DEBUG) printf("CMD :  ");
  for(i = 0; i < 4; i++)
  {
    //if (DEBUG) printf("%02X ", cmdbuf[i]);
    if(write_kbd_cmd_check_timeout((uint8_t)0xd4))
    {
      //if (DEBUG) printf("\nCMD D4 : Timeout\n");
      return 1;
    }
    if(write_kbd_data_check_timeout(cmdbuf[i]))
    {
      //if (DEBUG) printf("\nDATA : Timeout\n");
      return 1;
    }
    if(read_kbd_data_check_timeout(&st, &d))
    {
      //if (DEBUG) printf("\nREAD ACK : Timeout\n");
      return 1;
    }
    if(st != 1 || d!= 0xfa)
    {
      //if (DEBUG) printf("\nREAD ACK : Data error\n");
      return 1;
    }
  }
  //if (DEBUG) printf("\n");
  // read respond packet
  //if (DEBUG) printf("READ : ");
  i = 0;
  while(1)
  {
    if(read_kbd_data_check_timeout(&st, &d))
    {
      //if (DEBUG) printf("\nREAD : Timeout\n");
      return 1;
    }
    //if (DEBUG) printf("%02X ", d);
    if(!st)
    {
      //if (DEBUG) printf("\nREAD : NOT MOUSE\n");
      return 1;
    }
    if(i == 0)
    {
      if(d != 0xa)
      {
        //if (DEBUG) printf("RESP 0xA Error\n");
        return 1;
      }
    }
    else if(i == 1)
    {
      //resp_len = d;
      if(d != 4)
        return 1;
    }
    else if(i == 2)
    {
      if(d != 0x43)
      {
        //if (DEBUG) printf("RESP cmd Error\n");
        return 1;
      }
    }
    else if(i == 3)
    {
      if(d != idx)
      {
        return 1;
      }
    }
    if(i > 3)
    {
      *data = d;
      data++;
    }
    if(i >= 2 && i - 2 >= 4 - 1)
    {
      break;
    }
    i++;
  } 
  //if (DEBUG) printf("\n");
  return 0;
}

bool touchpanel_init(void) {
  uint8_t resp_buf[255];
  uint8_t eeprom_data[128];
  uint8_t idx;

  if (DEBUG) printf("discard keyboard data...");
  discard_kbd_data();
  if (DEBUG) printf("\n");
  
  if(write_touch_screen_cmd('A', resp_buf))
  {
    if (DEBUG) printf("Error\n");
    return false;
  }
  if(write_touch_screen_cmd('D', resp_buf))
  {
    if (DEBUG) printf("Error\n");
    return false;
  }
  if (DEBUG) printf("ASCII : %.*s\n", (int)resp_buf[1] - 1, resp_buf + 3);
  if(write_touch_screen_cmd('E', resp_buf))
  {
    if (DEBUG) printf("Error\n");
    return false;
  }
  if (DEBUG) printf("ASCII : %.*s\n", (int)resp_buf[1] - 1, resp_buf + 3);
  // read EEPROM data
  if (DEBUG) printf("Read EEPROM data (43H):\n");
  for(idx = 0; idx < 0x40; idx++)
  {
    if(write_touch_screen_cmd_43(idx, eeprom_data + idx * 2))
    {
      if (DEBUG) printf("Error\n");
      return false;
    }
  }
  if (DEBUG) printf("Read EEPROM Ok\n");
  // FILE *fp;
  // fp = fopen("touchkit.bin", "wb");
  // fwrite(eeprom_data, 1, 128, fp);
  // fclose(fp);
  
  return true;
}

bool get_touchXY(int* dataX, int* dataY) {
  static uint8_t ts_data[5];
  static int prev_dx=0, prev_dy=0;
  int i = 0;
  static int status = false;
  uint8_t st, d, index = 0;
  int ad;

  while(1) {
    if(kbd_obf())
    {
      st = (kbd_status() & 0x20) >> 5;
      if(st == 0)
        return false;  // from keyboard, quit
      d = kbd_read();     // read data uint8_t
      
      // if (DEBUG) printf("%02X ", d);
      if(i == 0)
      {
        if((d & 0x80) != 0x80)
        {
          if (DEBUG) printf("Bad uint8_t 0\n");
        }
      }
      else
      {
        if((d & 0x80) != 0)
        {
          if (DEBUG) printf("Bad uint8_t 1-4\n");
        }
      }
      ts_data[i] = d;
      if(i == 4)
      {
        status = (ts_data[0] & 1);
        ad = (ts_data[0] >> 1) & 0x3;
        *dataY = ((unsigned int)ts_data[1] << 7) | (ts_data[2] & 0x7f);
        *dataX = ((unsigned int)ts_data[3] << 7) | (ts_data[4] & 0x7f);
        // if (DEBUG) printf("| AD : %d, A : %04d, B : %04d, S = %d\n", ad, *dataX, *dataY, status);
        prev_dy = *dataY;
        prev_dx = *dataX;
        i = 0;
        // if(kbd_obf()) return true;
        return status;
      }
      else
        i++;
    } else {
      if (i == 0) break;
    }
  }
  
  *dataY = prev_dy;
  *dataX = prev_dx;
  return status;
}

void resetTPAll(void) {
  unsigned char retry = 0;
  
  if(write_kbd_cmd_check_timeout((uint8_t)0xd4))
  {
    if (DEBUG) printf("\nCMD D4 : Timeout\n");
    return;
  }
  if(write_kbd_data_check_timeout(0xff))
  {
    if (DEBUG) printf("\nDATA : Timeout\n");
    return;
  }
  
  while(1) {
      if (!touchpanel_init()) retry++; else break;
      if (retry > 50) break;
  }
}