#ifndef _ROS_ethercat_hardware_MotorTemperature_h
#define _ROS_ethercat_hardware_MotorTemperature_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/time.h"

namespace ethercat_hardware
{

  class MotorTemperature : public ros::Msg
  {
    public:
      ros::Time stamp;
      float winding_temperature;
      float housing_temperature;
      float ambient_temperature;
      float heating_power;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->stamp.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->stamp.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->stamp.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->stamp.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->stamp.sec);
      *(outbuffer + offset + 0) = (this->stamp.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->stamp.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->stamp.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->stamp.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->stamp.nsec);
      int32_t * val_winding_temperature = (int32_t *) &(this->winding_temperature);
      int32_t exp_winding_temperature = (((*val_winding_temperature)>>23)&255);
      if(exp_winding_temperature != 0)
        exp_winding_temperature += 1023-127;
      int32_t sig_winding_temperature = *val_winding_temperature;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_winding_temperature<<5) & 0xff;
      *(outbuffer + offset++) = (sig_winding_temperature>>3) & 0xff;
      *(outbuffer + offset++) = (sig_winding_temperature>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_winding_temperature<<4) & 0xF0) | ((sig_winding_temperature>>19)&0x0F);
      *(outbuffer + offset++) = (exp_winding_temperature>>4) & 0x7F;
      if(this->winding_temperature < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_housing_temperature = (int32_t *) &(this->housing_temperature);
      int32_t exp_housing_temperature = (((*val_housing_temperature)>>23)&255);
      if(exp_housing_temperature != 0)
        exp_housing_temperature += 1023-127;
      int32_t sig_housing_temperature = *val_housing_temperature;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_housing_temperature<<5) & 0xff;
      *(outbuffer + offset++) = (sig_housing_temperature>>3) & 0xff;
      *(outbuffer + offset++) = (sig_housing_temperature>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_housing_temperature<<4) & 0xF0) | ((sig_housing_temperature>>19)&0x0F);
      *(outbuffer + offset++) = (exp_housing_temperature>>4) & 0x7F;
      if(this->housing_temperature < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_ambient_temperature = (int32_t *) &(this->ambient_temperature);
      int32_t exp_ambient_temperature = (((*val_ambient_temperature)>>23)&255);
      if(exp_ambient_temperature != 0)
        exp_ambient_temperature += 1023-127;
      int32_t sig_ambient_temperature = *val_ambient_temperature;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_ambient_temperature<<5) & 0xff;
      *(outbuffer + offset++) = (sig_ambient_temperature>>3) & 0xff;
      *(outbuffer + offset++) = (sig_ambient_temperature>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_ambient_temperature<<4) & 0xF0) | ((sig_ambient_temperature>>19)&0x0F);
      *(outbuffer + offset++) = (exp_ambient_temperature>>4) & 0x7F;
      if(this->ambient_temperature < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_heating_power = (int32_t *) &(this->heating_power);
      int32_t exp_heating_power = (((*val_heating_power)>>23)&255);
      if(exp_heating_power != 0)
        exp_heating_power += 1023-127;
      int32_t sig_heating_power = *val_heating_power;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_heating_power<<5) & 0xff;
      *(outbuffer + offset++) = (sig_heating_power>>3) & 0xff;
      *(outbuffer + offset++) = (sig_heating_power>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_heating_power<<4) & 0xF0) | ((sig_heating_power>>19)&0x0F);
      *(outbuffer + offset++) = (exp_heating_power>>4) & 0x7F;
      if(this->heating_power < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->stamp.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->stamp.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->stamp.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->stamp.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->stamp.sec);
      this->stamp.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->stamp.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->stamp.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->stamp.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->stamp.nsec);
      uint32_t * val_winding_temperature = (uint32_t*) &(this->winding_temperature);
      offset += 3;
      *val_winding_temperature = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_winding_temperature |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_winding_temperature |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_winding_temperature |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_winding_temperature = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_winding_temperature |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_winding_temperature !=0)
        *val_winding_temperature |= ((exp_winding_temperature)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->winding_temperature = -this->winding_temperature;
      uint32_t * val_housing_temperature = (uint32_t*) &(this->housing_temperature);
      offset += 3;
      *val_housing_temperature = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_housing_temperature |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_housing_temperature |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_housing_temperature |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_housing_temperature = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_housing_temperature |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_housing_temperature !=0)
        *val_housing_temperature |= ((exp_housing_temperature)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->housing_temperature = -this->housing_temperature;
      uint32_t * val_ambient_temperature = (uint32_t*) &(this->ambient_temperature);
      offset += 3;
      *val_ambient_temperature = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_ambient_temperature |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_ambient_temperature |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_ambient_temperature |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_ambient_temperature = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_ambient_temperature |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_ambient_temperature !=0)
        *val_ambient_temperature |= ((exp_ambient_temperature)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->ambient_temperature = -this->ambient_temperature;
      uint32_t * val_heating_power = (uint32_t*) &(this->heating_power);
      offset += 3;
      *val_heating_power = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_heating_power |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_heating_power |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_heating_power |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_heating_power = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_heating_power |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_heating_power !=0)
        *val_heating_power |= ((exp_heating_power)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->heating_power = -this->heating_power;
     return offset;
    }

    const char * getType(){ return "ethercat_hardware/MotorTemperature"; };
    const char * getMD5(){ return "d8c7239cd096d6f25b75bff6b63f2162"; };

  };

}
#endif