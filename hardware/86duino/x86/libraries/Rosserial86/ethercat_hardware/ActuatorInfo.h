#ifndef _ROS_ethercat_hardware_ActuatorInfo_h
#define _ROS_ethercat_hardware_ActuatorInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ethercat_hardware
{

  class ActuatorInfo : public ros::Msg
  {
    public:
      uint32_t id;
      char * name;
      char * robot_name;
      char * motor_make;
      char * motor_model;
      float max_current;
      float speed_constant;
      float motor_resistance;
      float motor_torque_constant;
      float encoder_reduction;
      float pulses_per_revolution;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->id >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->id >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->id >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->id >> (8 * 3)) & 0xFF;
      offset += sizeof(this->id);
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_robot_name = strlen( (const char*) this->robot_name);
      memcpy(outbuffer + offset, &length_robot_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->robot_name, length_robot_name);
      offset += length_robot_name;
      uint32_t length_motor_make = strlen( (const char*) this->motor_make);
      memcpy(outbuffer + offset, &length_motor_make, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->motor_make, length_motor_make);
      offset += length_motor_make;
      uint32_t length_motor_model = strlen( (const char*) this->motor_model);
      memcpy(outbuffer + offset, &length_motor_model, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->motor_model, length_motor_model);
      offset += length_motor_model;
      int32_t * val_max_current = (int32_t *) &(this->max_current);
      int32_t exp_max_current = (((*val_max_current)>>23)&255);
      if(exp_max_current != 0)
        exp_max_current += 1023-127;
      int32_t sig_max_current = *val_max_current;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_current<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_current>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_current>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_current<<4) & 0xF0) | ((sig_max_current>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_current>>4) & 0x7F;
      if(this->max_current < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_speed_constant = (int32_t *) &(this->speed_constant);
      int32_t exp_speed_constant = (((*val_speed_constant)>>23)&255);
      if(exp_speed_constant != 0)
        exp_speed_constant += 1023-127;
      int32_t sig_speed_constant = *val_speed_constant;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_speed_constant<<5) & 0xff;
      *(outbuffer + offset++) = (sig_speed_constant>>3) & 0xff;
      *(outbuffer + offset++) = (sig_speed_constant>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_speed_constant<<4) & 0xF0) | ((sig_speed_constant>>19)&0x0F);
      *(outbuffer + offset++) = (exp_speed_constant>>4) & 0x7F;
      if(this->speed_constant < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_motor_resistance = (int32_t *) &(this->motor_resistance);
      int32_t exp_motor_resistance = (((*val_motor_resistance)>>23)&255);
      if(exp_motor_resistance != 0)
        exp_motor_resistance += 1023-127;
      int32_t sig_motor_resistance = *val_motor_resistance;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_motor_resistance<<5) & 0xff;
      *(outbuffer + offset++) = (sig_motor_resistance>>3) & 0xff;
      *(outbuffer + offset++) = (sig_motor_resistance>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_motor_resistance<<4) & 0xF0) | ((sig_motor_resistance>>19)&0x0F);
      *(outbuffer + offset++) = (exp_motor_resistance>>4) & 0x7F;
      if(this->motor_resistance < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_motor_torque_constant = (int32_t *) &(this->motor_torque_constant);
      int32_t exp_motor_torque_constant = (((*val_motor_torque_constant)>>23)&255);
      if(exp_motor_torque_constant != 0)
        exp_motor_torque_constant += 1023-127;
      int32_t sig_motor_torque_constant = *val_motor_torque_constant;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_motor_torque_constant<<5) & 0xff;
      *(outbuffer + offset++) = (sig_motor_torque_constant>>3) & 0xff;
      *(outbuffer + offset++) = (sig_motor_torque_constant>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_motor_torque_constant<<4) & 0xF0) | ((sig_motor_torque_constant>>19)&0x0F);
      *(outbuffer + offset++) = (exp_motor_torque_constant>>4) & 0x7F;
      if(this->motor_torque_constant < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_encoder_reduction = (int32_t *) &(this->encoder_reduction);
      int32_t exp_encoder_reduction = (((*val_encoder_reduction)>>23)&255);
      if(exp_encoder_reduction != 0)
        exp_encoder_reduction += 1023-127;
      int32_t sig_encoder_reduction = *val_encoder_reduction;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_encoder_reduction<<5) & 0xff;
      *(outbuffer + offset++) = (sig_encoder_reduction>>3) & 0xff;
      *(outbuffer + offset++) = (sig_encoder_reduction>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_encoder_reduction<<4) & 0xF0) | ((sig_encoder_reduction>>19)&0x0F);
      *(outbuffer + offset++) = (exp_encoder_reduction>>4) & 0x7F;
      if(this->encoder_reduction < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_pulses_per_revolution = (int32_t *) &(this->pulses_per_revolution);
      int32_t exp_pulses_per_revolution = (((*val_pulses_per_revolution)>>23)&255);
      if(exp_pulses_per_revolution != 0)
        exp_pulses_per_revolution += 1023-127;
      int32_t sig_pulses_per_revolution = *val_pulses_per_revolution;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_pulses_per_revolution<<5) & 0xff;
      *(outbuffer + offset++) = (sig_pulses_per_revolution>>3) & 0xff;
      *(outbuffer + offset++) = (sig_pulses_per_revolution>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_pulses_per_revolution<<4) & 0xF0) | ((sig_pulses_per_revolution>>19)&0x0F);
      *(outbuffer + offset++) = (exp_pulses_per_revolution>>4) & 0x7F;
      if(this->pulses_per_revolution < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->id =  ((uint32_t) (*(inbuffer + offset)));
      this->id |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->id |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->id |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->id);
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      uint32_t length_robot_name;
      memcpy(&length_robot_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_robot_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_robot_name-1]=0;
      this->robot_name = (char *)(inbuffer + offset-1);
      offset += length_robot_name;
      uint32_t length_motor_make;
      memcpy(&length_motor_make, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_motor_make; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_motor_make-1]=0;
      this->motor_make = (char *)(inbuffer + offset-1);
      offset += length_motor_make;
      uint32_t length_motor_model;
      memcpy(&length_motor_model, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_motor_model; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_motor_model-1]=0;
      this->motor_model = (char *)(inbuffer + offset-1);
      offset += length_motor_model;
      uint32_t * val_max_current = (uint32_t*) &(this->max_current);
      offset += 3;
      *val_max_current = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_current |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_current = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_current |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_current !=0)
        *val_max_current |= ((exp_max_current)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_current = -this->max_current;
      uint32_t * val_speed_constant = (uint32_t*) &(this->speed_constant);
      offset += 3;
      *val_speed_constant = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_speed_constant |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_speed_constant |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_speed_constant |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_speed_constant = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_speed_constant |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_speed_constant !=0)
        *val_speed_constant |= ((exp_speed_constant)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->speed_constant = -this->speed_constant;
      uint32_t * val_motor_resistance = (uint32_t*) &(this->motor_resistance);
      offset += 3;
      *val_motor_resistance = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_motor_resistance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_motor_resistance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_motor_resistance |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_motor_resistance = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_motor_resistance |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_motor_resistance !=0)
        *val_motor_resistance |= ((exp_motor_resistance)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->motor_resistance = -this->motor_resistance;
      uint32_t * val_motor_torque_constant = (uint32_t*) &(this->motor_torque_constant);
      offset += 3;
      *val_motor_torque_constant = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_motor_torque_constant |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_motor_torque_constant |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_motor_torque_constant |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_motor_torque_constant = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_motor_torque_constant |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_motor_torque_constant !=0)
        *val_motor_torque_constant |= ((exp_motor_torque_constant)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->motor_torque_constant = -this->motor_torque_constant;
      uint32_t * val_encoder_reduction = (uint32_t*) &(this->encoder_reduction);
      offset += 3;
      *val_encoder_reduction = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_encoder_reduction |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_encoder_reduction |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_encoder_reduction |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_encoder_reduction = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_encoder_reduction |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_encoder_reduction !=0)
        *val_encoder_reduction |= ((exp_encoder_reduction)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->encoder_reduction = -this->encoder_reduction;
      uint32_t * val_pulses_per_revolution = (uint32_t*) &(this->pulses_per_revolution);
      offset += 3;
      *val_pulses_per_revolution = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_pulses_per_revolution |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_pulses_per_revolution |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_pulses_per_revolution |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_pulses_per_revolution = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_pulses_per_revolution |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_pulses_per_revolution !=0)
        *val_pulses_per_revolution |= ((exp_pulses_per_revolution)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->pulses_per_revolution = -this->pulses_per_revolution;
     return offset;
    }

    const char * getType(){ return "ethercat_hardware/ActuatorInfo"; };
    const char * getMD5(){ return "40f44d8ec4380adc0b63713486eecb09"; };

  };

}
#endif