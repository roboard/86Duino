#ifndef _ROS_pr2_msgs_LaserTrajCmd_h
#define _ROS_pr2_msgs_LaserTrajCmd_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "ros/duration.h"

namespace pr2_msgs
{

  class LaserTrajCmd : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * profile;
      uint8_t position_length;
      float st_position;
      float * position;
      uint8_t time_from_start_length;
      ros::Duration st_time_from_start;
      ros::Duration * time_from_start;
      float max_velocity;
      float max_acceleration;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_profile = strlen( (const char*) this->profile);
      memcpy(outbuffer + offset, &length_profile, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->profile, length_profile);
      offset += length_profile;
      *(outbuffer + offset++) = position_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < position_length; i++){
      int32_t * val_positioni = (int32_t *) &(this->position[i]);
      int32_t exp_positioni = (((*val_positioni)>>23)&255);
      if(exp_positioni != 0)
        exp_positioni += 1023-127;
      int32_t sig_positioni = *val_positioni;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_positioni<<5) & 0xff;
      *(outbuffer + offset++) = (sig_positioni>>3) & 0xff;
      *(outbuffer + offset++) = (sig_positioni>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_positioni<<4) & 0xF0) | ((sig_positioni>>19)&0x0F);
      *(outbuffer + offset++) = (exp_positioni>>4) & 0x7F;
      if(this->position[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = time_from_start_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < time_from_start_length; i++){
      *(outbuffer + offset + 0) = (this->time_from_start[i].sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->time_from_start[i].sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->time_from_start[i].sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->time_from_start[i].sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->time_from_start[i].sec);
      *(outbuffer + offset + 0) = (this->time_from_start[i].nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->time_from_start[i].nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->time_from_start[i].nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->time_from_start[i].nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->time_from_start[i].nsec);
      }
      int32_t * val_max_velocity = (int32_t *) &(this->max_velocity);
      int32_t exp_max_velocity = (((*val_max_velocity)>>23)&255);
      if(exp_max_velocity != 0)
        exp_max_velocity += 1023-127;
      int32_t sig_max_velocity = *val_max_velocity;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_velocity<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_velocity>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_velocity>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_velocity<<4) & 0xF0) | ((sig_max_velocity>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_velocity>>4) & 0x7F;
      if(this->max_velocity < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_max_acceleration = (int32_t *) &(this->max_acceleration);
      int32_t exp_max_acceleration = (((*val_max_acceleration)>>23)&255);
      if(exp_max_acceleration != 0)
        exp_max_acceleration += 1023-127;
      int32_t sig_max_acceleration = *val_max_acceleration;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_acceleration<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_acceleration>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_acceleration>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_acceleration<<4) & 0xF0) | ((sig_max_acceleration>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_acceleration>>4) & 0x7F;
      if(this->max_acceleration < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_profile;
      memcpy(&length_profile, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_profile; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_profile-1]=0;
      this->profile = (char *)(inbuffer + offset-1);
      offset += length_profile;
      uint8_t position_lengthT = *(inbuffer + offset++);
      if(position_lengthT > position_length)
        this->position = (float*)realloc(this->position, position_lengthT * sizeof(float));
      offset += 3;
      position_length = position_lengthT;
      for( uint8_t i = 0; i < position_length; i++){
      uint32_t * val_st_position = (uint32_t*) &(this->st_position);
      offset += 3;
      *val_st_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_position !=0)
        *val_st_position |= ((exp_st_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_position = -this->st_position;
        memcpy( &(this->position[i]), &(this->st_position), sizeof(float));
      }
      uint8_t time_from_start_lengthT = *(inbuffer + offset++);
      if(time_from_start_lengthT > time_from_start_length)
        this->time_from_start = (ros::Duration*)realloc(this->time_from_start, time_from_start_lengthT * sizeof(ros::Duration));
      offset += 3;
      time_from_start_length = time_from_start_lengthT;
      for( uint8_t i = 0; i < time_from_start_length; i++){
      this->st_time_from_start.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->st_time_from_start.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->st_time_from_start.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->st_time_from_start.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->st_time_from_start.sec);
      this->st_time_from_start.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->st_time_from_start.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->st_time_from_start.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->st_time_from_start.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->st_time_from_start.nsec);
        memcpy( &(this->time_from_start[i]), &(this->st_time_from_start), sizeof(ros::Duration));
      }
      uint32_t * val_max_velocity = (uint32_t*) &(this->max_velocity);
      offset += 3;
      *val_max_velocity = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_velocity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_velocity = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_velocity |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_velocity !=0)
        *val_max_velocity |= ((exp_max_velocity)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_velocity = -this->max_velocity;
      uint32_t * val_max_acceleration = (uint32_t*) &(this->max_acceleration);
      offset += 3;
      *val_max_acceleration = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_acceleration |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_acceleration |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_acceleration |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_acceleration = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_acceleration |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_acceleration !=0)
        *val_max_acceleration |= ((exp_max_acceleration)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_acceleration = -this->max_acceleration;
     return offset;
    }

    const char * getType(){ return "pr2_msgs/LaserTrajCmd"; };
    const char * getMD5(){ return "68a1665e9079049dce55a0384cb2e9b5"; };

  };

}
#endif