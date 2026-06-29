#ifndef _ROS_ethercat_hardware_MotorTraceSample_h
#define _ROS_ethercat_hardware_MotorTraceSample_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ethercat_hardware
{

  class MotorTraceSample : public ros::Msg
  {
    public:
      float timestamp;
      bool enabled;
      float supply_voltage;
      float measured_motor_voltage;
      float programmed_pwm;
      float executed_current;
      float measured_current;
      float velocity;
      float encoder_position;
      uint32_t encoder_error_count;
      float motor_voltage_error_limit;
      float filtered_motor_voltage_error;
      float filtered_abs_motor_voltage_error;
      float filtered_measured_voltage_error;
      float filtered_abs_measured_voltage_error;
      float filtered_current_error;
      float filtered_abs_current_error;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_timestamp = (int32_t *) &(this->timestamp);
      int32_t exp_timestamp = (((*val_timestamp)>>23)&255);
      if(exp_timestamp != 0)
        exp_timestamp += 1023-127;
      int32_t sig_timestamp = *val_timestamp;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_timestamp<<5) & 0xff;
      *(outbuffer + offset++) = (sig_timestamp>>3) & 0xff;
      *(outbuffer + offset++) = (sig_timestamp>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_timestamp<<4) & 0xF0) | ((sig_timestamp>>19)&0x0F);
      *(outbuffer + offset++) = (exp_timestamp>>4) & 0x7F;
      if(this->timestamp < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_enabled;
      u_enabled.real = this->enabled;
      *(outbuffer + offset + 0) = (u_enabled.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->enabled);
      int32_t * val_supply_voltage = (int32_t *) &(this->supply_voltage);
      int32_t exp_supply_voltage = (((*val_supply_voltage)>>23)&255);
      if(exp_supply_voltage != 0)
        exp_supply_voltage += 1023-127;
      int32_t sig_supply_voltage = *val_supply_voltage;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_supply_voltage<<5) & 0xff;
      *(outbuffer + offset++) = (sig_supply_voltage>>3) & 0xff;
      *(outbuffer + offset++) = (sig_supply_voltage>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_supply_voltage<<4) & 0xF0) | ((sig_supply_voltage>>19)&0x0F);
      *(outbuffer + offset++) = (exp_supply_voltage>>4) & 0x7F;
      if(this->supply_voltage < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_measured_motor_voltage = (int32_t *) &(this->measured_motor_voltage);
      int32_t exp_measured_motor_voltage = (((*val_measured_motor_voltage)>>23)&255);
      if(exp_measured_motor_voltage != 0)
        exp_measured_motor_voltage += 1023-127;
      int32_t sig_measured_motor_voltage = *val_measured_motor_voltage;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_measured_motor_voltage<<5) & 0xff;
      *(outbuffer + offset++) = (sig_measured_motor_voltage>>3) & 0xff;
      *(outbuffer + offset++) = (sig_measured_motor_voltage>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_measured_motor_voltage<<4) & 0xF0) | ((sig_measured_motor_voltage>>19)&0x0F);
      *(outbuffer + offset++) = (exp_measured_motor_voltage>>4) & 0x7F;
      if(this->measured_motor_voltage < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_programmed_pwm = (int32_t *) &(this->programmed_pwm);
      int32_t exp_programmed_pwm = (((*val_programmed_pwm)>>23)&255);
      if(exp_programmed_pwm != 0)
        exp_programmed_pwm += 1023-127;
      int32_t sig_programmed_pwm = *val_programmed_pwm;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_programmed_pwm<<5) & 0xff;
      *(outbuffer + offset++) = (sig_programmed_pwm>>3) & 0xff;
      *(outbuffer + offset++) = (sig_programmed_pwm>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_programmed_pwm<<4) & 0xF0) | ((sig_programmed_pwm>>19)&0x0F);
      *(outbuffer + offset++) = (exp_programmed_pwm>>4) & 0x7F;
      if(this->programmed_pwm < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_executed_current = (int32_t *) &(this->executed_current);
      int32_t exp_executed_current = (((*val_executed_current)>>23)&255);
      if(exp_executed_current != 0)
        exp_executed_current += 1023-127;
      int32_t sig_executed_current = *val_executed_current;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_executed_current<<5) & 0xff;
      *(outbuffer + offset++) = (sig_executed_current>>3) & 0xff;
      *(outbuffer + offset++) = (sig_executed_current>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_executed_current<<4) & 0xF0) | ((sig_executed_current>>19)&0x0F);
      *(outbuffer + offset++) = (exp_executed_current>>4) & 0x7F;
      if(this->executed_current < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_measured_current = (int32_t *) &(this->measured_current);
      int32_t exp_measured_current = (((*val_measured_current)>>23)&255);
      if(exp_measured_current != 0)
        exp_measured_current += 1023-127;
      int32_t sig_measured_current = *val_measured_current;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_measured_current<<5) & 0xff;
      *(outbuffer + offset++) = (sig_measured_current>>3) & 0xff;
      *(outbuffer + offset++) = (sig_measured_current>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_measured_current<<4) & 0xF0) | ((sig_measured_current>>19)&0x0F);
      *(outbuffer + offset++) = (exp_measured_current>>4) & 0x7F;
      if(this->measured_current < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_velocity = (int32_t *) &(this->velocity);
      int32_t exp_velocity = (((*val_velocity)>>23)&255);
      if(exp_velocity != 0)
        exp_velocity += 1023-127;
      int32_t sig_velocity = *val_velocity;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_velocity<<5) & 0xff;
      *(outbuffer + offset++) = (sig_velocity>>3) & 0xff;
      *(outbuffer + offset++) = (sig_velocity>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_velocity<<4) & 0xF0) | ((sig_velocity>>19)&0x0F);
      *(outbuffer + offset++) = (exp_velocity>>4) & 0x7F;
      if(this->velocity < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_encoder_position = (int32_t *) &(this->encoder_position);
      int32_t exp_encoder_position = (((*val_encoder_position)>>23)&255);
      if(exp_encoder_position != 0)
        exp_encoder_position += 1023-127;
      int32_t sig_encoder_position = *val_encoder_position;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_encoder_position<<5) & 0xff;
      *(outbuffer + offset++) = (sig_encoder_position>>3) & 0xff;
      *(outbuffer + offset++) = (sig_encoder_position>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_encoder_position<<4) & 0xF0) | ((sig_encoder_position>>19)&0x0F);
      *(outbuffer + offset++) = (exp_encoder_position>>4) & 0x7F;
      if(this->encoder_position < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset + 0) = (this->encoder_error_count >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->encoder_error_count >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->encoder_error_count >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->encoder_error_count >> (8 * 3)) & 0xFF;
      offset += sizeof(this->encoder_error_count);
      int32_t * val_motor_voltage_error_limit = (int32_t *) &(this->motor_voltage_error_limit);
      int32_t exp_motor_voltage_error_limit = (((*val_motor_voltage_error_limit)>>23)&255);
      if(exp_motor_voltage_error_limit != 0)
        exp_motor_voltage_error_limit += 1023-127;
      int32_t sig_motor_voltage_error_limit = *val_motor_voltage_error_limit;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_motor_voltage_error_limit<<5) & 0xff;
      *(outbuffer + offset++) = (sig_motor_voltage_error_limit>>3) & 0xff;
      *(outbuffer + offset++) = (sig_motor_voltage_error_limit>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_motor_voltage_error_limit<<4) & 0xF0) | ((sig_motor_voltage_error_limit>>19)&0x0F);
      *(outbuffer + offset++) = (exp_motor_voltage_error_limit>>4) & 0x7F;
      if(this->motor_voltage_error_limit < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_filtered_motor_voltage_error = (int32_t *) &(this->filtered_motor_voltage_error);
      int32_t exp_filtered_motor_voltage_error = (((*val_filtered_motor_voltage_error)>>23)&255);
      if(exp_filtered_motor_voltage_error != 0)
        exp_filtered_motor_voltage_error += 1023-127;
      int32_t sig_filtered_motor_voltage_error = *val_filtered_motor_voltage_error;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_filtered_motor_voltage_error<<5) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_motor_voltage_error>>3) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_motor_voltage_error>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_filtered_motor_voltage_error<<4) & 0xF0) | ((sig_filtered_motor_voltage_error>>19)&0x0F);
      *(outbuffer + offset++) = (exp_filtered_motor_voltage_error>>4) & 0x7F;
      if(this->filtered_motor_voltage_error < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_filtered_abs_motor_voltage_error = (int32_t *) &(this->filtered_abs_motor_voltage_error);
      int32_t exp_filtered_abs_motor_voltage_error = (((*val_filtered_abs_motor_voltage_error)>>23)&255);
      if(exp_filtered_abs_motor_voltage_error != 0)
        exp_filtered_abs_motor_voltage_error += 1023-127;
      int32_t sig_filtered_abs_motor_voltage_error = *val_filtered_abs_motor_voltage_error;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_filtered_abs_motor_voltage_error<<5) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_abs_motor_voltage_error>>3) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_abs_motor_voltage_error>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_filtered_abs_motor_voltage_error<<4) & 0xF0) | ((sig_filtered_abs_motor_voltage_error>>19)&0x0F);
      *(outbuffer + offset++) = (exp_filtered_abs_motor_voltage_error>>4) & 0x7F;
      if(this->filtered_abs_motor_voltage_error < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_filtered_measured_voltage_error = (int32_t *) &(this->filtered_measured_voltage_error);
      int32_t exp_filtered_measured_voltage_error = (((*val_filtered_measured_voltage_error)>>23)&255);
      if(exp_filtered_measured_voltage_error != 0)
        exp_filtered_measured_voltage_error += 1023-127;
      int32_t sig_filtered_measured_voltage_error = *val_filtered_measured_voltage_error;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_filtered_measured_voltage_error<<5) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_measured_voltage_error>>3) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_measured_voltage_error>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_filtered_measured_voltage_error<<4) & 0xF0) | ((sig_filtered_measured_voltage_error>>19)&0x0F);
      *(outbuffer + offset++) = (exp_filtered_measured_voltage_error>>4) & 0x7F;
      if(this->filtered_measured_voltage_error < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_filtered_abs_measured_voltage_error = (int32_t *) &(this->filtered_abs_measured_voltage_error);
      int32_t exp_filtered_abs_measured_voltage_error = (((*val_filtered_abs_measured_voltage_error)>>23)&255);
      if(exp_filtered_abs_measured_voltage_error != 0)
        exp_filtered_abs_measured_voltage_error += 1023-127;
      int32_t sig_filtered_abs_measured_voltage_error = *val_filtered_abs_measured_voltage_error;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_filtered_abs_measured_voltage_error<<5) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_abs_measured_voltage_error>>3) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_abs_measured_voltage_error>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_filtered_abs_measured_voltage_error<<4) & 0xF0) | ((sig_filtered_abs_measured_voltage_error>>19)&0x0F);
      *(outbuffer + offset++) = (exp_filtered_abs_measured_voltage_error>>4) & 0x7F;
      if(this->filtered_abs_measured_voltage_error < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_filtered_current_error = (int32_t *) &(this->filtered_current_error);
      int32_t exp_filtered_current_error = (((*val_filtered_current_error)>>23)&255);
      if(exp_filtered_current_error != 0)
        exp_filtered_current_error += 1023-127;
      int32_t sig_filtered_current_error = *val_filtered_current_error;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_filtered_current_error<<5) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_current_error>>3) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_current_error>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_filtered_current_error<<4) & 0xF0) | ((sig_filtered_current_error>>19)&0x0F);
      *(outbuffer + offset++) = (exp_filtered_current_error>>4) & 0x7F;
      if(this->filtered_current_error < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_filtered_abs_current_error = (int32_t *) &(this->filtered_abs_current_error);
      int32_t exp_filtered_abs_current_error = (((*val_filtered_abs_current_error)>>23)&255);
      if(exp_filtered_abs_current_error != 0)
        exp_filtered_abs_current_error += 1023-127;
      int32_t sig_filtered_abs_current_error = *val_filtered_abs_current_error;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_filtered_abs_current_error<<5) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_abs_current_error>>3) & 0xff;
      *(outbuffer + offset++) = (sig_filtered_abs_current_error>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_filtered_abs_current_error<<4) & 0xF0) | ((sig_filtered_abs_current_error>>19)&0x0F);
      *(outbuffer + offset++) = (exp_filtered_abs_current_error>>4) & 0x7F;
      if(this->filtered_abs_current_error < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_timestamp = (uint32_t*) &(this->timestamp);
      offset += 3;
      *val_timestamp = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_timestamp |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_timestamp |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_timestamp |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_timestamp = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_timestamp |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_timestamp !=0)
        *val_timestamp |= ((exp_timestamp)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->timestamp = -this->timestamp;
      union {
        bool real;
        uint8_t base;
      } u_enabled;
      u_enabled.base = 0;
      u_enabled.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->enabled = u_enabled.real;
      offset += sizeof(this->enabled);
      uint32_t * val_supply_voltage = (uint32_t*) &(this->supply_voltage);
      offset += 3;
      *val_supply_voltage = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_supply_voltage |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_supply_voltage |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_supply_voltage |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_supply_voltage = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_supply_voltage |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_supply_voltage !=0)
        *val_supply_voltage |= ((exp_supply_voltage)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->supply_voltage = -this->supply_voltage;
      uint32_t * val_measured_motor_voltage = (uint32_t*) &(this->measured_motor_voltage);
      offset += 3;
      *val_measured_motor_voltage = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_measured_motor_voltage |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_measured_motor_voltage |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_measured_motor_voltage |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_measured_motor_voltage = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_measured_motor_voltage |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_measured_motor_voltage !=0)
        *val_measured_motor_voltage |= ((exp_measured_motor_voltage)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->measured_motor_voltage = -this->measured_motor_voltage;
      uint32_t * val_programmed_pwm = (uint32_t*) &(this->programmed_pwm);
      offset += 3;
      *val_programmed_pwm = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_programmed_pwm |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_programmed_pwm |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_programmed_pwm |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_programmed_pwm = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_programmed_pwm |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_programmed_pwm !=0)
        *val_programmed_pwm |= ((exp_programmed_pwm)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->programmed_pwm = -this->programmed_pwm;
      uint32_t * val_executed_current = (uint32_t*) &(this->executed_current);
      offset += 3;
      *val_executed_current = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_executed_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_executed_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_executed_current |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_executed_current = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_executed_current |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_executed_current !=0)
        *val_executed_current |= ((exp_executed_current)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->executed_current = -this->executed_current;
      uint32_t * val_measured_current = (uint32_t*) &(this->measured_current);
      offset += 3;
      *val_measured_current = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_measured_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_measured_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_measured_current |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_measured_current = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_measured_current |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_measured_current !=0)
        *val_measured_current |= ((exp_measured_current)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->measured_current = -this->measured_current;
      uint32_t * val_velocity = (uint32_t*) &(this->velocity);
      offset += 3;
      *val_velocity = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_velocity = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_velocity !=0)
        *val_velocity |= ((exp_velocity)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->velocity = -this->velocity;
      uint32_t * val_encoder_position = (uint32_t*) &(this->encoder_position);
      offset += 3;
      *val_encoder_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_encoder_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_encoder_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_encoder_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_encoder_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_encoder_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_encoder_position !=0)
        *val_encoder_position |= ((exp_encoder_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->encoder_position = -this->encoder_position;
      this->encoder_error_count =  ((uint32_t) (*(inbuffer + offset)));
      this->encoder_error_count |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->encoder_error_count |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->encoder_error_count |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->encoder_error_count);
      uint32_t * val_motor_voltage_error_limit = (uint32_t*) &(this->motor_voltage_error_limit);
      offset += 3;
      *val_motor_voltage_error_limit = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_motor_voltage_error_limit |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_motor_voltage_error_limit |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_motor_voltage_error_limit |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_motor_voltage_error_limit = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_motor_voltage_error_limit |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_motor_voltage_error_limit !=0)
        *val_motor_voltage_error_limit |= ((exp_motor_voltage_error_limit)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->motor_voltage_error_limit = -this->motor_voltage_error_limit;
      uint32_t * val_filtered_motor_voltage_error = (uint32_t*) &(this->filtered_motor_voltage_error);
      offset += 3;
      *val_filtered_motor_voltage_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_filtered_motor_voltage_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_filtered_motor_voltage_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_filtered_motor_voltage_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_filtered_motor_voltage_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_filtered_motor_voltage_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_filtered_motor_voltage_error !=0)
        *val_filtered_motor_voltage_error |= ((exp_filtered_motor_voltage_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->filtered_motor_voltage_error = -this->filtered_motor_voltage_error;
      uint32_t * val_filtered_abs_motor_voltage_error = (uint32_t*) &(this->filtered_abs_motor_voltage_error);
      offset += 3;
      *val_filtered_abs_motor_voltage_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_filtered_abs_motor_voltage_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_filtered_abs_motor_voltage_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_filtered_abs_motor_voltage_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_filtered_abs_motor_voltage_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_filtered_abs_motor_voltage_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_filtered_abs_motor_voltage_error !=0)
        *val_filtered_abs_motor_voltage_error |= ((exp_filtered_abs_motor_voltage_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->filtered_abs_motor_voltage_error = -this->filtered_abs_motor_voltage_error;
      uint32_t * val_filtered_measured_voltage_error = (uint32_t*) &(this->filtered_measured_voltage_error);
      offset += 3;
      *val_filtered_measured_voltage_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_filtered_measured_voltage_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_filtered_measured_voltage_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_filtered_measured_voltage_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_filtered_measured_voltage_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_filtered_measured_voltage_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_filtered_measured_voltage_error !=0)
        *val_filtered_measured_voltage_error |= ((exp_filtered_measured_voltage_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->filtered_measured_voltage_error = -this->filtered_measured_voltage_error;
      uint32_t * val_filtered_abs_measured_voltage_error = (uint32_t*) &(this->filtered_abs_measured_voltage_error);
      offset += 3;
      *val_filtered_abs_measured_voltage_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_filtered_abs_measured_voltage_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_filtered_abs_measured_voltage_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_filtered_abs_measured_voltage_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_filtered_abs_measured_voltage_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_filtered_abs_measured_voltage_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_filtered_abs_measured_voltage_error !=0)
        *val_filtered_abs_measured_voltage_error |= ((exp_filtered_abs_measured_voltage_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->filtered_abs_measured_voltage_error = -this->filtered_abs_measured_voltage_error;
      uint32_t * val_filtered_current_error = (uint32_t*) &(this->filtered_current_error);
      offset += 3;
      *val_filtered_current_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_filtered_current_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_filtered_current_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_filtered_current_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_filtered_current_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_filtered_current_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_filtered_current_error !=0)
        *val_filtered_current_error |= ((exp_filtered_current_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->filtered_current_error = -this->filtered_current_error;
      uint32_t * val_filtered_abs_current_error = (uint32_t*) &(this->filtered_abs_current_error);
      offset += 3;
      *val_filtered_abs_current_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_filtered_abs_current_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_filtered_abs_current_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_filtered_abs_current_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_filtered_abs_current_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_filtered_abs_current_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_filtered_abs_current_error !=0)
        *val_filtered_abs_current_error |= ((exp_filtered_abs_current_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->filtered_abs_current_error = -this->filtered_abs_current_error;
     return offset;
    }

    const char * getType(){ return "ethercat_hardware/MotorTraceSample"; };
    const char * getMD5(){ return "3734a66334bc2033448f9c561d39c5e0"; };

  };

}
#endif