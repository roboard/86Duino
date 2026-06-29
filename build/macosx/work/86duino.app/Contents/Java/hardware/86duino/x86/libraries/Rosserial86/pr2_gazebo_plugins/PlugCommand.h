#ifndef _ROS_pr2_gazebo_plugins_PlugCommand_h
#define _ROS_pr2_gazebo_plugins_PlugCommand_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_gazebo_plugins
{

  class PlugCommand : public ros::Msg
  {
    public:
      bool ac_present;
      float charge_rate;
      float discharge_rate;
      float charge;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_ac_present;
      u_ac_present.real = this->ac_present;
      *(outbuffer + offset + 0) = (u_ac_present.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->ac_present);
      int32_t * val_charge_rate = (int32_t *) &(this->charge_rate);
      int32_t exp_charge_rate = (((*val_charge_rate)>>23)&255);
      if(exp_charge_rate != 0)
        exp_charge_rate += 1023-127;
      int32_t sig_charge_rate = *val_charge_rate;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_charge_rate<<5) & 0xff;
      *(outbuffer + offset++) = (sig_charge_rate>>3) & 0xff;
      *(outbuffer + offset++) = (sig_charge_rate>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_charge_rate<<4) & 0xF0) | ((sig_charge_rate>>19)&0x0F);
      *(outbuffer + offset++) = (exp_charge_rate>>4) & 0x7F;
      if(this->charge_rate < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_discharge_rate = (int32_t *) &(this->discharge_rate);
      int32_t exp_discharge_rate = (((*val_discharge_rate)>>23)&255);
      if(exp_discharge_rate != 0)
        exp_discharge_rate += 1023-127;
      int32_t sig_discharge_rate = *val_discharge_rate;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_discharge_rate<<5) & 0xff;
      *(outbuffer + offset++) = (sig_discharge_rate>>3) & 0xff;
      *(outbuffer + offset++) = (sig_discharge_rate>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_discharge_rate<<4) & 0xF0) | ((sig_discharge_rate>>19)&0x0F);
      *(outbuffer + offset++) = (exp_discharge_rate>>4) & 0x7F;
      if(this->discharge_rate < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_charge = (int32_t *) &(this->charge);
      int32_t exp_charge = (((*val_charge)>>23)&255);
      if(exp_charge != 0)
        exp_charge += 1023-127;
      int32_t sig_charge = *val_charge;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_charge<<5) & 0xff;
      *(outbuffer + offset++) = (sig_charge>>3) & 0xff;
      *(outbuffer + offset++) = (sig_charge>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_charge<<4) & 0xF0) | ((sig_charge>>19)&0x0F);
      *(outbuffer + offset++) = (exp_charge>>4) & 0x7F;
      if(this->charge < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_ac_present;
      u_ac_present.base = 0;
      u_ac_present.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->ac_present = u_ac_present.real;
      offset += sizeof(this->ac_present);
      uint32_t * val_charge_rate = (uint32_t*) &(this->charge_rate);
      offset += 3;
      *val_charge_rate = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_charge_rate |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_charge_rate |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_charge_rate |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_charge_rate = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_charge_rate |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_charge_rate !=0)
        *val_charge_rate |= ((exp_charge_rate)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->charge_rate = -this->charge_rate;
      uint32_t * val_discharge_rate = (uint32_t*) &(this->discharge_rate);
      offset += 3;
      *val_discharge_rate = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_discharge_rate |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_discharge_rate |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_discharge_rate |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_discharge_rate = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_discharge_rate |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_discharge_rate !=0)
        *val_discharge_rate |= ((exp_discharge_rate)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->discharge_rate = -this->discharge_rate;
      uint32_t * val_charge = (uint32_t*) &(this->charge);
      offset += 3;
      *val_charge = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_charge |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_charge |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_charge |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_charge = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_charge |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_charge !=0)
        *val_charge |= ((exp_charge)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->charge = -this->charge;
     return offset;
    }

    const char * getType(){ return "pr2_gazebo_plugins/PlugCommand"; };
    const char * getMD5(){ return "852b7035ee3e7fa6390824cf7b7e6dd1"; };

  };

}
#endif