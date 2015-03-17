#ifndef _ROS_pr2_msgs_PowerBoardState_h
#define _ROS_pr2_msgs_PowerBoardState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace pr2_msgs
{

  class PowerBoardState : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * name;
      uint32_t serial_num;
      float input_voltage;
      int8_t master_state;
      int8_t circuit_state[3];
      float circuit_voltage[3];
      bool run_stop;
      bool wireless_stop;
      enum { STATE_NOPOWER = 0 };
      enum { STATE_STANDBY = 1 };
      enum { STATE_PUMPING = 2 };
      enum { STATE_ON = 3 };
      enum { STATE_ENABLED = 3   };
      enum { STATE_DISABLED = 4 };
      enum { MASTER_NOPOWER = 0 };
      enum { MASTER_STANDBY = 1 };
      enum { MASTER_ON = 2 };
      enum { MASTER_OFF = 3 };
      enum { MASTER_SHUTDOWN = 4 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      *(outbuffer + offset + 0) = (this->serial_num >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->serial_num >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->serial_num >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->serial_num >> (8 * 3)) & 0xFF;
      offset += sizeof(this->serial_num);
      int32_t * val_input_voltage = (int32_t *) &(this->input_voltage);
      int32_t exp_input_voltage = (((*val_input_voltage)>>23)&255);
      if(exp_input_voltage != 0)
        exp_input_voltage += 1023-127;
      int32_t sig_input_voltage = *val_input_voltage;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_input_voltage<<5) & 0xff;
      *(outbuffer + offset++) = (sig_input_voltage>>3) & 0xff;
      *(outbuffer + offset++) = (sig_input_voltage>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_input_voltage<<4) & 0xF0) | ((sig_input_voltage>>19)&0x0F);
      *(outbuffer + offset++) = (exp_input_voltage>>4) & 0x7F;
      if(this->input_voltage < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        int8_t real;
        uint8_t base;
      } u_master_state;
      u_master_state.real = this->master_state;
      *(outbuffer + offset + 0) = (u_master_state.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->master_state);
      unsigned char * circuit_state_val = (unsigned char *) this->circuit_state;
      for( uint8_t i = 0; i < 3; i++){
      union {
        int8_t real;
        uint8_t base;
      } u_circuit_statei;
      u_circuit_statei.real = this->circuit_state[i];
      *(outbuffer + offset + 0) = (u_circuit_statei.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->circuit_state[i]);
      }
      unsigned char * circuit_voltage_val = (unsigned char *) this->circuit_voltage;
      for( uint8_t i = 0; i < 3; i++){
      int32_t * val_circuit_voltagei = (int32_t *) &(this->circuit_voltage[i]);
      int32_t exp_circuit_voltagei = (((*val_circuit_voltagei)>>23)&255);
      if(exp_circuit_voltagei != 0)
        exp_circuit_voltagei += 1023-127;
      int32_t sig_circuit_voltagei = *val_circuit_voltagei;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_circuit_voltagei<<5) & 0xff;
      *(outbuffer + offset++) = (sig_circuit_voltagei>>3) & 0xff;
      *(outbuffer + offset++) = (sig_circuit_voltagei>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_circuit_voltagei<<4) & 0xF0) | ((sig_circuit_voltagei>>19)&0x0F);
      *(outbuffer + offset++) = (exp_circuit_voltagei>>4) & 0x7F;
      if(this->circuit_voltage[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      union {
        bool real;
        uint8_t base;
      } u_run_stop;
      u_run_stop.real = this->run_stop;
      *(outbuffer + offset + 0) = (u_run_stop.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->run_stop);
      union {
        bool real;
        uint8_t base;
      } u_wireless_stop;
      u_wireless_stop.real = this->wireless_stop;
      *(outbuffer + offset + 0) = (u_wireless_stop.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->wireless_stop);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      this->serial_num =  ((uint32_t) (*(inbuffer + offset)));
      this->serial_num |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->serial_num |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->serial_num |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->serial_num);
      uint32_t * val_input_voltage = (uint32_t*) &(this->input_voltage);
      offset += 3;
      *val_input_voltage = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_input_voltage |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_input_voltage |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_input_voltage |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_input_voltage = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_input_voltage |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_input_voltage !=0)
        *val_input_voltage |= ((exp_input_voltage)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->input_voltage = -this->input_voltage;
      union {
        int8_t real;
        uint8_t base;
      } u_master_state;
      u_master_state.base = 0;
      u_master_state.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->master_state = u_master_state.real;
      offset += sizeof(this->master_state);
      uint8_t * circuit_state_val = (uint8_t*) this->circuit_state;
      for( uint8_t i = 0; i < 3; i++){
      union {
        int8_t real;
        uint8_t base;
      } u_circuit_statei;
      u_circuit_statei.base = 0;
      u_circuit_statei.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->circuit_state[i] = u_circuit_statei.real;
      offset += sizeof(this->circuit_state[i]);
      }
      uint8_t * circuit_voltage_val = (uint8_t*) this->circuit_voltage;
      for( uint8_t i = 0; i < 3; i++){
      uint32_t * val_circuit_voltagei = (uint32_t*) &(this->circuit_voltage[i]);
      offset += 3;
      *val_circuit_voltagei = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_circuit_voltagei |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_circuit_voltagei |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_circuit_voltagei |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_circuit_voltagei = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_circuit_voltagei |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_circuit_voltagei !=0)
        *val_circuit_voltagei |= ((exp_circuit_voltagei)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->circuit_voltage[i] = -this->circuit_voltage[i];
      }
      union {
        bool real;
        uint8_t base;
      } u_run_stop;
      u_run_stop.base = 0;
      u_run_stop.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->run_stop = u_run_stop.real;
      offset += sizeof(this->run_stop);
      union {
        bool real;
        uint8_t base;
      } u_wireless_stop;
      u_wireless_stop.base = 0;
      u_wireless_stop.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->wireless_stop = u_wireless_stop.real;
      offset += sizeof(this->wireless_stop);
     return offset;
    }

    const char * getType(){ return "pr2_msgs/PowerBoardState"; };
    const char * getMD5(){ return "08899b671e6a1a449e7ce0000da8ae7b"; };

  };

}
#endif