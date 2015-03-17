#ifndef _ROS_pr2_self_test_msgs_TestInfo_h
#define _ROS_pr2_self_test_msgs_TestInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_self_test_msgs
{

  class TestInfo : public ros::Msg
  {
    public:
      char * serial;
      char * test_name;
      int8_t test_status;
      char * bay_name;
      char * machine;
      int32_t board;
      int32_t breaker;
      char * power_status;
      int8_t estop;
      int32_t elapsed;
      char * status_msg;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_serial = strlen( (const char*) this->serial);
      memcpy(outbuffer + offset, &length_serial, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->serial, length_serial);
      offset += length_serial;
      uint32_t length_test_name = strlen( (const char*) this->test_name);
      memcpy(outbuffer + offset, &length_test_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->test_name, length_test_name);
      offset += length_test_name;
      union {
        int8_t real;
        uint8_t base;
      } u_test_status;
      u_test_status.real = this->test_status;
      *(outbuffer + offset + 0) = (u_test_status.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->test_status);
      uint32_t length_bay_name = strlen( (const char*) this->bay_name);
      memcpy(outbuffer + offset, &length_bay_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->bay_name, length_bay_name);
      offset += length_bay_name;
      uint32_t length_machine = strlen( (const char*) this->machine);
      memcpy(outbuffer + offset, &length_machine, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->machine, length_machine);
      offset += length_machine;
      union {
        int32_t real;
        uint32_t base;
      } u_board;
      u_board.real = this->board;
      *(outbuffer + offset + 0) = (u_board.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_board.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_board.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_board.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->board);
      union {
        int32_t real;
        uint32_t base;
      } u_breaker;
      u_breaker.real = this->breaker;
      *(outbuffer + offset + 0) = (u_breaker.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_breaker.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_breaker.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_breaker.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->breaker);
      uint32_t length_power_status = strlen( (const char*) this->power_status);
      memcpy(outbuffer + offset, &length_power_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->power_status, length_power_status);
      offset += length_power_status;
      union {
        int8_t real;
        uint8_t base;
      } u_estop;
      u_estop.real = this->estop;
      *(outbuffer + offset + 0) = (u_estop.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->estop);
      union {
        int32_t real;
        uint32_t base;
      } u_elapsed;
      u_elapsed.real = this->elapsed;
      *(outbuffer + offset + 0) = (u_elapsed.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_elapsed.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_elapsed.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_elapsed.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->elapsed);
      uint32_t length_status_msg = strlen( (const char*) this->status_msg);
      memcpy(outbuffer + offset, &length_status_msg, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->status_msg, length_status_msg);
      offset += length_status_msg;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_serial;
      memcpy(&length_serial, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_serial; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_serial-1]=0;
      this->serial = (char *)(inbuffer + offset-1);
      offset += length_serial;
      uint32_t length_test_name;
      memcpy(&length_test_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_test_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_test_name-1]=0;
      this->test_name = (char *)(inbuffer + offset-1);
      offset += length_test_name;
      union {
        int8_t real;
        uint8_t base;
      } u_test_status;
      u_test_status.base = 0;
      u_test_status.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->test_status = u_test_status.real;
      offset += sizeof(this->test_status);
      uint32_t length_bay_name;
      memcpy(&length_bay_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_bay_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_bay_name-1]=0;
      this->bay_name = (char *)(inbuffer + offset-1);
      offset += length_bay_name;
      uint32_t length_machine;
      memcpy(&length_machine, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_machine; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_machine-1]=0;
      this->machine = (char *)(inbuffer + offset-1);
      offset += length_machine;
      union {
        int32_t real;
        uint32_t base;
      } u_board;
      u_board.base = 0;
      u_board.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_board.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_board.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_board.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->board = u_board.real;
      offset += sizeof(this->board);
      union {
        int32_t real;
        uint32_t base;
      } u_breaker;
      u_breaker.base = 0;
      u_breaker.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_breaker.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_breaker.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_breaker.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->breaker = u_breaker.real;
      offset += sizeof(this->breaker);
      uint32_t length_power_status;
      memcpy(&length_power_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_power_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_power_status-1]=0;
      this->power_status = (char *)(inbuffer + offset-1);
      offset += length_power_status;
      union {
        int8_t real;
        uint8_t base;
      } u_estop;
      u_estop.base = 0;
      u_estop.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->estop = u_estop.real;
      offset += sizeof(this->estop);
      union {
        int32_t real;
        uint32_t base;
      } u_elapsed;
      u_elapsed.base = 0;
      u_elapsed.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_elapsed.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_elapsed.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_elapsed.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->elapsed = u_elapsed.real;
      offset += sizeof(this->elapsed);
      uint32_t length_status_msg;
      memcpy(&length_status_msg, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_status_msg; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_status_msg-1]=0;
      this->status_msg = (char *)(inbuffer + offset-1);
      offset += length_status_msg;
     return offset;
    }

    const char * getType(){ return "pr2_self_test_msgs/TestInfo"; };
    const char * getMD5(){ return "594219b720ecaee963aa0b601d38b8f2"; };

  };

}
#endif