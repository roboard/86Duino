#ifndef _ROS_pr2_mechanism_msgs_ActuatorStatistics_h
#define _ROS_pr2_mechanism_msgs_ActuatorStatistics_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/time.h"

namespace pr2_mechanism_msgs
{

  class ActuatorStatistics : public ros::Msg
  {
    public:
      char * name;
      int32_t device_id;
      ros::Time timestamp;
      int32_t encoder_count;
      float encoder_offset;
      float position;
      float encoder_velocity;
      float velocity;
      bool calibration_reading;
      bool calibration_rising_edge_valid;
      bool calibration_falling_edge_valid;
      float last_calibration_rising_edge;
      float last_calibration_falling_edge;
      bool is_enabled;
      bool halted;
      float last_commanded_current;
      float last_commanded_effort;
      float last_executed_current;
      float last_executed_effort;
      float last_measured_current;
      float last_measured_effort;
      float motor_voltage;
      int32_t num_encoder_errors;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      union {
        int32_t real;
        uint32_t base;
      } u_device_id;
      u_device_id.real = this->device_id;
      *(outbuffer + offset + 0) = (u_device_id.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_device_id.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_device_id.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_device_id.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->device_id);
      *(outbuffer + offset + 0) = (this->timestamp.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->timestamp.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->timestamp.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->timestamp.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->timestamp.sec);
      *(outbuffer + offset + 0) = (this->timestamp.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->timestamp.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->timestamp.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->timestamp.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->timestamp.nsec);
      union {
        int32_t real;
        uint32_t base;
      } u_encoder_count;
      u_encoder_count.real = this->encoder_count;
      *(outbuffer + offset + 0) = (u_encoder_count.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_encoder_count.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_encoder_count.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_encoder_count.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->encoder_count);
      int32_t * val_encoder_offset = (int32_t *) &(this->encoder_offset);
      int32_t exp_encoder_offset = (((*val_encoder_offset)>>23)&255);
      if(exp_encoder_offset != 0)
        exp_encoder_offset += 1023-127;
      int32_t sig_encoder_offset = *val_encoder_offset;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_encoder_offset<<5) & 0xff;
      *(outbuffer + offset++) = (sig_encoder_offset>>3) & 0xff;
      *(outbuffer + offset++) = (sig_encoder_offset>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_encoder_offset<<4) & 0xF0) | ((sig_encoder_offset>>19)&0x0F);
      *(outbuffer + offset++) = (exp_encoder_offset>>4) & 0x7F;
      if(this->encoder_offset < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_position = (int32_t *) &(this->position);
      int32_t exp_position = (((*val_position)>>23)&255);
      if(exp_position != 0)
        exp_position += 1023-127;
      int32_t sig_position = *val_position;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_position<<5) & 0xff;
      *(outbuffer + offset++) = (sig_position>>3) & 0xff;
      *(outbuffer + offset++) = (sig_position>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_position<<4) & 0xF0) | ((sig_position>>19)&0x0F);
      *(outbuffer + offset++) = (exp_position>>4) & 0x7F;
      if(this->position < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_encoder_velocity = (int32_t *) &(this->encoder_velocity);
      int32_t exp_encoder_velocity = (((*val_encoder_velocity)>>23)&255);
      if(exp_encoder_velocity != 0)
        exp_encoder_velocity += 1023-127;
      int32_t sig_encoder_velocity = *val_encoder_velocity;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_encoder_velocity<<5) & 0xff;
      *(outbuffer + offset++) = (sig_encoder_velocity>>3) & 0xff;
      *(outbuffer + offset++) = (sig_encoder_velocity>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_encoder_velocity<<4) & 0xF0) | ((sig_encoder_velocity>>19)&0x0F);
      *(outbuffer + offset++) = (exp_encoder_velocity>>4) & 0x7F;
      if(this->encoder_velocity < 0) *(outbuffer + offset -1) |= 0x80;
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
      union {
        bool real;
        uint8_t base;
      } u_calibration_reading;
      u_calibration_reading.real = this->calibration_reading;
      *(outbuffer + offset + 0) = (u_calibration_reading.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->calibration_reading);
      union {
        bool real;
        uint8_t base;
      } u_calibration_rising_edge_valid;
      u_calibration_rising_edge_valid.real = this->calibration_rising_edge_valid;
      *(outbuffer + offset + 0) = (u_calibration_rising_edge_valid.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->calibration_rising_edge_valid);
      union {
        bool real;
        uint8_t base;
      } u_calibration_falling_edge_valid;
      u_calibration_falling_edge_valid.real = this->calibration_falling_edge_valid;
      *(outbuffer + offset + 0) = (u_calibration_falling_edge_valid.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->calibration_falling_edge_valid);
      int32_t * val_last_calibration_rising_edge = (int32_t *) &(this->last_calibration_rising_edge);
      int32_t exp_last_calibration_rising_edge = (((*val_last_calibration_rising_edge)>>23)&255);
      if(exp_last_calibration_rising_edge != 0)
        exp_last_calibration_rising_edge += 1023-127;
      int32_t sig_last_calibration_rising_edge = *val_last_calibration_rising_edge;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_last_calibration_rising_edge<<5) & 0xff;
      *(outbuffer + offset++) = (sig_last_calibration_rising_edge>>3) & 0xff;
      *(outbuffer + offset++) = (sig_last_calibration_rising_edge>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_last_calibration_rising_edge<<4) & 0xF0) | ((sig_last_calibration_rising_edge>>19)&0x0F);
      *(outbuffer + offset++) = (exp_last_calibration_rising_edge>>4) & 0x7F;
      if(this->last_calibration_rising_edge < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_last_calibration_falling_edge = (int32_t *) &(this->last_calibration_falling_edge);
      int32_t exp_last_calibration_falling_edge = (((*val_last_calibration_falling_edge)>>23)&255);
      if(exp_last_calibration_falling_edge != 0)
        exp_last_calibration_falling_edge += 1023-127;
      int32_t sig_last_calibration_falling_edge = *val_last_calibration_falling_edge;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_last_calibration_falling_edge<<5) & 0xff;
      *(outbuffer + offset++) = (sig_last_calibration_falling_edge>>3) & 0xff;
      *(outbuffer + offset++) = (sig_last_calibration_falling_edge>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_last_calibration_falling_edge<<4) & 0xF0) | ((sig_last_calibration_falling_edge>>19)&0x0F);
      *(outbuffer + offset++) = (exp_last_calibration_falling_edge>>4) & 0x7F;
      if(this->last_calibration_falling_edge < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_is_enabled;
      u_is_enabled.real = this->is_enabled;
      *(outbuffer + offset + 0) = (u_is_enabled.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->is_enabled);
      union {
        bool real;
        uint8_t base;
      } u_halted;
      u_halted.real = this->halted;
      *(outbuffer + offset + 0) = (u_halted.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->halted);
      int32_t * val_last_commanded_current = (int32_t *) &(this->last_commanded_current);
      int32_t exp_last_commanded_current = (((*val_last_commanded_current)>>23)&255);
      if(exp_last_commanded_current != 0)
        exp_last_commanded_current += 1023-127;
      int32_t sig_last_commanded_current = *val_last_commanded_current;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_last_commanded_current<<5) & 0xff;
      *(outbuffer + offset++) = (sig_last_commanded_current>>3) & 0xff;
      *(outbuffer + offset++) = (sig_last_commanded_current>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_last_commanded_current<<4) & 0xF0) | ((sig_last_commanded_current>>19)&0x0F);
      *(outbuffer + offset++) = (exp_last_commanded_current>>4) & 0x7F;
      if(this->last_commanded_current < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_last_commanded_effort = (int32_t *) &(this->last_commanded_effort);
      int32_t exp_last_commanded_effort = (((*val_last_commanded_effort)>>23)&255);
      if(exp_last_commanded_effort != 0)
        exp_last_commanded_effort += 1023-127;
      int32_t sig_last_commanded_effort = *val_last_commanded_effort;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_last_commanded_effort<<5) & 0xff;
      *(outbuffer + offset++) = (sig_last_commanded_effort>>3) & 0xff;
      *(outbuffer + offset++) = (sig_last_commanded_effort>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_last_commanded_effort<<4) & 0xF0) | ((sig_last_commanded_effort>>19)&0x0F);
      *(outbuffer + offset++) = (exp_last_commanded_effort>>4) & 0x7F;
      if(this->last_commanded_effort < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_last_executed_current = (int32_t *) &(this->last_executed_current);
      int32_t exp_last_executed_current = (((*val_last_executed_current)>>23)&255);
      if(exp_last_executed_current != 0)
        exp_last_executed_current += 1023-127;
      int32_t sig_last_executed_current = *val_last_executed_current;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_last_executed_current<<5) & 0xff;
      *(outbuffer + offset++) = (sig_last_executed_current>>3) & 0xff;
      *(outbuffer + offset++) = (sig_last_executed_current>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_last_executed_current<<4) & 0xF0) | ((sig_last_executed_current>>19)&0x0F);
      *(outbuffer + offset++) = (exp_last_executed_current>>4) & 0x7F;
      if(this->last_executed_current < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_last_executed_effort = (int32_t *) &(this->last_executed_effort);
      int32_t exp_last_executed_effort = (((*val_last_executed_effort)>>23)&255);
      if(exp_last_executed_effort != 0)
        exp_last_executed_effort += 1023-127;
      int32_t sig_last_executed_effort = *val_last_executed_effort;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_last_executed_effort<<5) & 0xff;
      *(outbuffer + offset++) = (sig_last_executed_effort>>3) & 0xff;
      *(outbuffer + offset++) = (sig_last_executed_effort>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_last_executed_effort<<4) & 0xF0) | ((sig_last_executed_effort>>19)&0x0F);
      *(outbuffer + offset++) = (exp_last_executed_effort>>4) & 0x7F;
      if(this->last_executed_effort < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_last_measured_current = (int32_t *) &(this->last_measured_current);
      int32_t exp_last_measured_current = (((*val_last_measured_current)>>23)&255);
      if(exp_last_measured_current != 0)
        exp_last_measured_current += 1023-127;
      int32_t sig_last_measured_current = *val_last_measured_current;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_last_measured_current<<5) & 0xff;
      *(outbuffer + offset++) = (sig_last_measured_current>>3) & 0xff;
      *(outbuffer + offset++) = (sig_last_measured_current>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_last_measured_current<<4) & 0xF0) | ((sig_last_measured_current>>19)&0x0F);
      *(outbuffer + offset++) = (exp_last_measured_current>>4) & 0x7F;
      if(this->last_measured_current < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_last_measured_effort = (int32_t *) &(this->last_measured_effort);
      int32_t exp_last_measured_effort = (((*val_last_measured_effort)>>23)&255);
      if(exp_last_measured_effort != 0)
        exp_last_measured_effort += 1023-127;
      int32_t sig_last_measured_effort = *val_last_measured_effort;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_last_measured_effort<<5) & 0xff;
      *(outbuffer + offset++) = (sig_last_measured_effort>>3) & 0xff;
      *(outbuffer + offset++) = (sig_last_measured_effort>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_last_measured_effort<<4) & 0xF0) | ((sig_last_measured_effort>>19)&0x0F);
      *(outbuffer + offset++) = (exp_last_measured_effort>>4) & 0x7F;
      if(this->last_measured_effort < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_motor_voltage = (int32_t *) &(this->motor_voltage);
      int32_t exp_motor_voltage = (((*val_motor_voltage)>>23)&255);
      if(exp_motor_voltage != 0)
        exp_motor_voltage += 1023-127;
      int32_t sig_motor_voltage = *val_motor_voltage;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_motor_voltage<<5) & 0xff;
      *(outbuffer + offset++) = (sig_motor_voltage>>3) & 0xff;
      *(outbuffer + offset++) = (sig_motor_voltage>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_motor_voltage<<4) & 0xF0) | ((sig_motor_voltage>>19)&0x0F);
      *(outbuffer + offset++) = (exp_motor_voltage>>4) & 0x7F;
      if(this->motor_voltage < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        int32_t real;
        uint32_t base;
      } u_num_encoder_errors;
      u_num_encoder_errors.real = this->num_encoder_errors;
      *(outbuffer + offset + 0) = (u_num_encoder_errors.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_num_encoder_errors.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_num_encoder_errors.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_num_encoder_errors.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->num_encoder_errors);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      union {
        int32_t real;
        uint32_t base;
      } u_device_id;
      u_device_id.base = 0;
      u_device_id.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_device_id.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_device_id.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_device_id.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->device_id = u_device_id.real;
      offset += sizeof(this->device_id);
      this->timestamp.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->timestamp.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->timestamp.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->timestamp.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->timestamp.sec);
      this->timestamp.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->timestamp.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->timestamp.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->timestamp.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->timestamp.nsec);
      union {
        int32_t real;
        uint32_t base;
      } u_encoder_count;
      u_encoder_count.base = 0;
      u_encoder_count.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_encoder_count.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_encoder_count.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_encoder_count.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->encoder_count = u_encoder_count.real;
      offset += sizeof(this->encoder_count);
      uint32_t * val_encoder_offset = (uint32_t*) &(this->encoder_offset);
      offset += 3;
      *val_encoder_offset = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_encoder_offset |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_encoder_offset |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_encoder_offset |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_encoder_offset = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_encoder_offset |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_encoder_offset !=0)
        *val_encoder_offset |= ((exp_encoder_offset)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->encoder_offset = -this->encoder_offset;
      uint32_t * val_position = (uint32_t*) &(this->position);
      offset += 3;
      *val_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_position !=0)
        *val_position |= ((exp_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->position = -this->position;
      uint32_t * val_encoder_velocity = (uint32_t*) &(this->encoder_velocity);
      offset += 3;
      *val_encoder_velocity = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_encoder_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_encoder_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_encoder_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_encoder_velocity = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_encoder_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_encoder_velocity !=0)
        *val_encoder_velocity |= ((exp_encoder_velocity)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->encoder_velocity = -this->encoder_velocity;
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
      union {
        bool real;
        uint8_t base;
      } u_calibration_reading;
      u_calibration_reading.base = 0;
      u_calibration_reading.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->calibration_reading = u_calibration_reading.real;
      offset += sizeof(this->calibration_reading);
      union {
        bool real;
        uint8_t base;
      } u_calibration_rising_edge_valid;
      u_calibration_rising_edge_valid.base = 0;
      u_calibration_rising_edge_valid.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->calibration_rising_edge_valid = u_calibration_rising_edge_valid.real;
      offset += sizeof(this->calibration_rising_edge_valid);
      union {
        bool real;
        uint8_t base;
      } u_calibration_falling_edge_valid;
      u_calibration_falling_edge_valid.base = 0;
      u_calibration_falling_edge_valid.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->calibration_falling_edge_valid = u_calibration_falling_edge_valid.real;
      offset += sizeof(this->calibration_falling_edge_valid);
      uint32_t * val_last_calibration_rising_edge = (uint32_t*) &(this->last_calibration_rising_edge);
      offset += 3;
      *val_last_calibration_rising_edge = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_last_calibration_rising_edge |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_last_calibration_rising_edge |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_last_calibration_rising_edge |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_last_calibration_rising_edge = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_last_calibration_rising_edge |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_last_calibration_rising_edge !=0)
        *val_last_calibration_rising_edge |= ((exp_last_calibration_rising_edge)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->last_calibration_rising_edge = -this->last_calibration_rising_edge;
      uint32_t * val_last_calibration_falling_edge = (uint32_t*) &(this->last_calibration_falling_edge);
      offset += 3;
      *val_last_calibration_falling_edge = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_last_calibration_falling_edge |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_last_calibration_falling_edge |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_last_calibration_falling_edge |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_last_calibration_falling_edge = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_last_calibration_falling_edge |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_last_calibration_falling_edge !=0)
        *val_last_calibration_falling_edge |= ((exp_last_calibration_falling_edge)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->last_calibration_falling_edge = -this->last_calibration_falling_edge;
      union {
        bool real;
        uint8_t base;
      } u_is_enabled;
      u_is_enabled.base = 0;
      u_is_enabled.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->is_enabled = u_is_enabled.real;
      offset += sizeof(this->is_enabled);
      union {
        bool real;
        uint8_t base;
      } u_halted;
      u_halted.base = 0;
      u_halted.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->halted = u_halted.real;
      offset += sizeof(this->halted);
      uint32_t * val_last_commanded_current = (uint32_t*) &(this->last_commanded_current);
      offset += 3;
      *val_last_commanded_current = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_last_commanded_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_last_commanded_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_last_commanded_current |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_last_commanded_current = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_last_commanded_current |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_last_commanded_current !=0)
        *val_last_commanded_current |= ((exp_last_commanded_current)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->last_commanded_current = -this->last_commanded_current;
      uint32_t * val_last_commanded_effort = (uint32_t*) &(this->last_commanded_effort);
      offset += 3;
      *val_last_commanded_effort = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_last_commanded_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_last_commanded_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_last_commanded_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_last_commanded_effort = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_last_commanded_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_last_commanded_effort !=0)
        *val_last_commanded_effort |= ((exp_last_commanded_effort)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->last_commanded_effort = -this->last_commanded_effort;
      uint32_t * val_last_executed_current = (uint32_t*) &(this->last_executed_current);
      offset += 3;
      *val_last_executed_current = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_last_executed_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_last_executed_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_last_executed_current |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_last_executed_current = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_last_executed_current |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_last_executed_current !=0)
        *val_last_executed_current |= ((exp_last_executed_current)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->last_executed_current = -this->last_executed_current;
      uint32_t * val_last_executed_effort = (uint32_t*) &(this->last_executed_effort);
      offset += 3;
      *val_last_executed_effort = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_last_executed_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_last_executed_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_last_executed_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_last_executed_effort = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_last_executed_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_last_executed_effort !=0)
        *val_last_executed_effort |= ((exp_last_executed_effort)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->last_executed_effort = -this->last_executed_effort;
      uint32_t * val_last_measured_current = (uint32_t*) &(this->last_measured_current);
      offset += 3;
      *val_last_measured_current = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_last_measured_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_last_measured_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_last_measured_current |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_last_measured_current = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_last_measured_current |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_last_measured_current !=0)
        *val_last_measured_current |= ((exp_last_measured_current)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->last_measured_current = -this->last_measured_current;
      uint32_t * val_last_measured_effort = (uint32_t*) &(this->last_measured_effort);
      offset += 3;
      *val_last_measured_effort = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_last_measured_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_last_measured_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_last_measured_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_last_measured_effort = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_last_measured_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_last_measured_effort !=0)
        *val_last_measured_effort |= ((exp_last_measured_effort)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->last_measured_effort = -this->last_measured_effort;
      uint32_t * val_motor_voltage = (uint32_t*) &(this->motor_voltage);
      offset += 3;
      *val_motor_voltage = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_motor_voltage |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_motor_voltage |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_motor_voltage |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_motor_voltage = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_motor_voltage |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_motor_voltage !=0)
        *val_motor_voltage |= ((exp_motor_voltage)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->motor_voltage = -this->motor_voltage;
      union {
        int32_t real;
        uint32_t base;
      } u_num_encoder_errors;
      u_num_encoder_errors.base = 0;
      u_num_encoder_errors.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_num_encoder_errors.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_num_encoder_errors.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_num_encoder_errors.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->num_encoder_errors = u_num_encoder_errors.real;
      offset += sizeof(this->num_encoder_errors);
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_msgs/ActuatorStatistics"; };
    const char * getMD5(){ return "c37184273b29627de29382f1d3670175"; };

  };

}
#endif