#ifndef _ROS_pr2_mechanism_msgs_JointStatistics_h
#define _ROS_pr2_mechanism_msgs_JointStatistics_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/time.h"

namespace pr2_mechanism_msgs
{

  class JointStatistics : public ros::Msg
  {
    public:
      char * name;
      ros::Time timestamp;
      float position;
      float velocity;
      float measured_effort;
      float commanded_effort;
      bool is_calibrated;
      bool violated_limits;
      float odometer;
      float min_position;
      float max_position;
      float max_abs_velocity;
      float max_abs_effort;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
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
      int32_t * val_measured_effort = (int32_t *) &(this->measured_effort);
      int32_t exp_measured_effort = (((*val_measured_effort)>>23)&255);
      if(exp_measured_effort != 0)
        exp_measured_effort += 1023-127;
      int32_t sig_measured_effort = *val_measured_effort;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_measured_effort<<5) & 0xff;
      *(outbuffer + offset++) = (sig_measured_effort>>3) & 0xff;
      *(outbuffer + offset++) = (sig_measured_effort>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_measured_effort<<4) & 0xF0) | ((sig_measured_effort>>19)&0x0F);
      *(outbuffer + offset++) = (exp_measured_effort>>4) & 0x7F;
      if(this->measured_effort < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_commanded_effort = (int32_t *) &(this->commanded_effort);
      int32_t exp_commanded_effort = (((*val_commanded_effort)>>23)&255);
      if(exp_commanded_effort != 0)
        exp_commanded_effort += 1023-127;
      int32_t sig_commanded_effort = *val_commanded_effort;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_commanded_effort<<5) & 0xff;
      *(outbuffer + offset++) = (sig_commanded_effort>>3) & 0xff;
      *(outbuffer + offset++) = (sig_commanded_effort>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_commanded_effort<<4) & 0xF0) | ((sig_commanded_effort>>19)&0x0F);
      *(outbuffer + offset++) = (exp_commanded_effort>>4) & 0x7F;
      if(this->commanded_effort < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_is_calibrated;
      u_is_calibrated.real = this->is_calibrated;
      *(outbuffer + offset + 0) = (u_is_calibrated.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->is_calibrated);
      union {
        bool real;
        uint8_t base;
      } u_violated_limits;
      u_violated_limits.real = this->violated_limits;
      *(outbuffer + offset + 0) = (u_violated_limits.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->violated_limits);
      int32_t * val_odometer = (int32_t *) &(this->odometer);
      int32_t exp_odometer = (((*val_odometer)>>23)&255);
      if(exp_odometer != 0)
        exp_odometer += 1023-127;
      int32_t sig_odometer = *val_odometer;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_odometer<<5) & 0xff;
      *(outbuffer + offset++) = (sig_odometer>>3) & 0xff;
      *(outbuffer + offset++) = (sig_odometer>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_odometer<<4) & 0xF0) | ((sig_odometer>>19)&0x0F);
      *(outbuffer + offset++) = (exp_odometer>>4) & 0x7F;
      if(this->odometer < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_min_position = (int32_t *) &(this->min_position);
      int32_t exp_min_position = (((*val_min_position)>>23)&255);
      if(exp_min_position != 0)
        exp_min_position += 1023-127;
      int32_t sig_min_position = *val_min_position;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_min_position<<5) & 0xff;
      *(outbuffer + offset++) = (sig_min_position>>3) & 0xff;
      *(outbuffer + offset++) = (sig_min_position>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_min_position<<4) & 0xF0) | ((sig_min_position>>19)&0x0F);
      *(outbuffer + offset++) = (exp_min_position>>4) & 0x7F;
      if(this->min_position < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_max_position = (int32_t *) &(this->max_position);
      int32_t exp_max_position = (((*val_max_position)>>23)&255);
      if(exp_max_position != 0)
        exp_max_position += 1023-127;
      int32_t sig_max_position = *val_max_position;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_position<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_position>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_position>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_position<<4) & 0xF0) | ((sig_max_position>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_position>>4) & 0x7F;
      if(this->max_position < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_max_abs_velocity = (int32_t *) &(this->max_abs_velocity);
      int32_t exp_max_abs_velocity = (((*val_max_abs_velocity)>>23)&255);
      if(exp_max_abs_velocity != 0)
        exp_max_abs_velocity += 1023-127;
      int32_t sig_max_abs_velocity = *val_max_abs_velocity;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_abs_velocity<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_abs_velocity>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_abs_velocity>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_abs_velocity<<4) & 0xF0) | ((sig_max_abs_velocity>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_abs_velocity>>4) & 0x7F;
      if(this->max_abs_velocity < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_max_abs_effort = (int32_t *) &(this->max_abs_effort);
      int32_t exp_max_abs_effort = (((*val_max_abs_effort)>>23)&255);
      if(exp_max_abs_effort != 0)
        exp_max_abs_effort += 1023-127;
      int32_t sig_max_abs_effort = *val_max_abs_effort;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_abs_effort<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_abs_effort>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_abs_effort>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_abs_effort<<4) & 0xF0) | ((sig_max_abs_effort>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_abs_effort>>4) & 0x7F;
      if(this->max_abs_effort < 0) *(outbuffer + offset -1) |= 0x80;
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
      uint32_t * val_measured_effort = (uint32_t*) &(this->measured_effort);
      offset += 3;
      *val_measured_effort = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_measured_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_measured_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_measured_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_measured_effort = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_measured_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_measured_effort !=0)
        *val_measured_effort |= ((exp_measured_effort)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->measured_effort = -this->measured_effort;
      uint32_t * val_commanded_effort = (uint32_t*) &(this->commanded_effort);
      offset += 3;
      *val_commanded_effort = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_commanded_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_commanded_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_commanded_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_commanded_effort = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_commanded_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_commanded_effort !=0)
        *val_commanded_effort |= ((exp_commanded_effort)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->commanded_effort = -this->commanded_effort;
      union {
        bool real;
        uint8_t base;
      } u_is_calibrated;
      u_is_calibrated.base = 0;
      u_is_calibrated.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->is_calibrated = u_is_calibrated.real;
      offset += sizeof(this->is_calibrated);
      union {
        bool real;
        uint8_t base;
      } u_violated_limits;
      u_violated_limits.base = 0;
      u_violated_limits.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->violated_limits = u_violated_limits.real;
      offset += sizeof(this->violated_limits);
      uint32_t * val_odometer = (uint32_t*) &(this->odometer);
      offset += 3;
      *val_odometer = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_odometer |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_odometer |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_odometer |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_odometer = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_odometer |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_odometer !=0)
        *val_odometer |= ((exp_odometer)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->odometer = -this->odometer;
      uint32_t * val_min_position = (uint32_t*) &(this->min_position);
      offset += 3;
      *val_min_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_min_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_min_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_min_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_min_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_min_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_min_position !=0)
        *val_min_position |= ((exp_min_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->min_position = -this->min_position;
      uint32_t * val_max_position = (uint32_t*) &(this->max_position);
      offset += 3;
      *val_max_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_position !=0)
        *val_max_position |= ((exp_max_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_position = -this->max_position;
      uint32_t * val_max_abs_velocity = (uint32_t*) &(this->max_abs_velocity);
      offset += 3;
      *val_max_abs_velocity = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_abs_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_abs_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_abs_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_abs_velocity = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_abs_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_abs_velocity !=0)
        *val_max_abs_velocity |= ((exp_max_abs_velocity)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_abs_velocity = -this->max_abs_velocity;
      uint32_t * val_max_abs_effort = (uint32_t*) &(this->max_abs_effort);
      offset += 3;
      *val_max_abs_effort = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_abs_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_abs_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_abs_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_abs_effort = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_abs_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_abs_effort !=0)
        *val_max_abs_effort |= ((exp_max_abs_effort)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_abs_effort = -this->max_abs_effort;
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_msgs/JointStatistics"; };
    const char * getMD5(){ return "90fdc8acbce5bc783d8b4aec49af6590"; };

  };

}
#endif