#ifndef _ROS_joint_qualification_controllers_HysteresisData_h
#define _ROS_joint_qualification_controllers_HysteresisData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace joint_qualification_controllers
{

  class HysteresisData : public ros::Msg
  {
    public:
      char * joint_name;
      uint8_t time_up_length;
      float st_time_up;
      float * time_up;
      uint8_t effort_up_length;
      float st_effort_up;
      float * effort_up;
      uint8_t position_up_length;
      float st_position_up;
      float * position_up;
      uint8_t velocity_up_length;
      float st_velocity_up;
      float * velocity_up;
      uint8_t time_down_length;
      float st_time_down;
      float * time_down;
      uint8_t effort_down_length;
      float st_effort_down;
      float * effort_down;
      uint8_t position_down_length;
      float st_position_down;
      float * position_down;
      uint8_t velocity_down_length;
      float st_velocity_down;
      float * velocity_down;
      uint8_t arg_name_length;
      char* st_arg_name;
      char* * arg_name;
      uint8_t arg_value_length;
      float st_arg_value;
      float * arg_value;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_joint_name = strlen( (const char*) this->joint_name);
      memcpy(outbuffer + offset, &length_joint_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->joint_name, length_joint_name);
      offset += length_joint_name;
      *(outbuffer + offset++) = time_up_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < time_up_length; i++){
      union {
        float real;
        uint32_t base;
      } u_time_upi;
      u_time_upi.real = this->time_up[i];
      *(outbuffer + offset + 0) = (u_time_upi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_time_upi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_time_upi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_time_upi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->time_up[i]);
      }
      *(outbuffer + offset++) = effort_up_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < effort_up_length; i++){
      union {
        float real;
        uint32_t base;
      } u_effort_upi;
      u_effort_upi.real = this->effort_up[i];
      *(outbuffer + offset + 0) = (u_effort_upi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_effort_upi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_effort_upi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_effort_upi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->effort_up[i]);
      }
      *(outbuffer + offset++) = position_up_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < position_up_length; i++){
      union {
        float real;
        uint32_t base;
      } u_position_upi;
      u_position_upi.real = this->position_up[i];
      *(outbuffer + offset + 0) = (u_position_upi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_position_upi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_position_upi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_position_upi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->position_up[i]);
      }
      *(outbuffer + offset++) = velocity_up_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < velocity_up_length; i++){
      union {
        float real;
        uint32_t base;
      } u_velocity_upi;
      u_velocity_upi.real = this->velocity_up[i];
      *(outbuffer + offset + 0) = (u_velocity_upi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_velocity_upi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_velocity_upi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_velocity_upi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->velocity_up[i]);
      }
      *(outbuffer + offset++) = time_down_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < time_down_length; i++){
      union {
        float real;
        uint32_t base;
      } u_time_downi;
      u_time_downi.real = this->time_down[i];
      *(outbuffer + offset + 0) = (u_time_downi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_time_downi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_time_downi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_time_downi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->time_down[i]);
      }
      *(outbuffer + offset++) = effort_down_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < effort_down_length; i++){
      union {
        float real;
        uint32_t base;
      } u_effort_downi;
      u_effort_downi.real = this->effort_down[i];
      *(outbuffer + offset + 0) = (u_effort_downi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_effort_downi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_effort_downi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_effort_downi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->effort_down[i]);
      }
      *(outbuffer + offset++) = position_down_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < position_down_length; i++){
      union {
        float real;
        uint32_t base;
      } u_position_downi;
      u_position_downi.real = this->position_down[i];
      *(outbuffer + offset + 0) = (u_position_downi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_position_downi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_position_downi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_position_downi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->position_down[i]);
      }
      *(outbuffer + offset++) = velocity_down_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < velocity_down_length; i++){
      union {
        float real;
        uint32_t base;
      } u_velocity_downi;
      u_velocity_downi.real = this->velocity_down[i];
      *(outbuffer + offset + 0) = (u_velocity_downi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_velocity_downi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_velocity_downi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_velocity_downi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->velocity_down[i]);
      }
      *(outbuffer + offset++) = arg_name_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < arg_name_length; i++){
      uint32_t length_arg_namei = strlen( (const char*) this->arg_name[i]);
      memcpy(outbuffer + offset, &length_arg_namei, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->arg_name[i], length_arg_namei);
      offset += length_arg_namei;
      }
      *(outbuffer + offset++) = arg_value_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < arg_value_length; i++){
      union {
        float real;
        uint32_t base;
      } u_arg_valuei;
      u_arg_valuei.real = this->arg_value[i];
      *(outbuffer + offset + 0) = (u_arg_valuei.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_arg_valuei.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_arg_valuei.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_arg_valuei.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->arg_value[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_joint_name;
      memcpy(&length_joint_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_joint_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_joint_name-1]=0;
      this->joint_name = (char *)(inbuffer + offset-1);
      offset += length_joint_name;
      uint8_t time_up_lengthT = *(inbuffer + offset++);
      if(time_up_lengthT > time_up_length)
        this->time_up = (float*)realloc(this->time_up, time_up_lengthT * sizeof(float));
      offset += 3;
      time_up_length = time_up_lengthT;
      for( uint8_t i = 0; i < time_up_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_time_up;
      u_st_time_up.base = 0;
      u_st_time_up.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_time_up.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_time_up.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_time_up.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_time_up = u_st_time_up.real;
      offset += sizeof(this->st_time_up);
        memcpy( &(this->time_up[i]), &(this->st_time_up), sizeof(float));
      }
      uint8_t effort_up_lengthT = *(inbuffer + offset++);
      if(effort_up_lengthT > effort_up_length)
        this->effort_up = (float*)realloc(this->effort_up, effort_up_lengthT * sizeof(float));
      offset += 3;
      effort_up_length = effort_up_lengthT;
      for( uint8_t i = 0; i < effort_up_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_effort_up;
      u_st_effort_up.base = 0;
      u_st_effort_up.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_effort_up.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_effort_up.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_effort_up.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_effort_up = u_st_effort_up.real;
      offset += sizeof(this->st_effort_up);
        memcpy( &(this->effort_up[i]), &(this->st_effort_up), sizeof(float));
      }
      uint8_t position_up_lengthT = *(inbuffer + offset++);
      if(position_up_lengthT > position_up_length)
        this->position_up = (float*)realloc(this->position_up, position_up_lengthT * sizeof(float));
      offset += 3;
      position_up_length = position_up_lengthT;
      for( uint8_t i = 0; i < position_up_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_position_up;
      u_st_position_up.base = 0;
      u_st_position_up.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_position_up.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_position_up.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_position_up.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_position_up = u_st_position_up.real;
      offset += sizeof(this->st_position_up);
        memcpy( &(this->position_up[i]), &(this->st_position_up), sizeof(float));
      }
      uint8_t velocity_up_lengthT = *(inbuffer + offset++);
      if(velocity_up_lengthT > velocity_up_length)
        this->velocity_up = (float*)realloc(this->velocity_up, velocity_up_lengthT * sizeof(float));
      offset += 3;
      velocity_up_length = velocity_up_lengthT;
      for( uint8_t i = 0; i < velocity_up_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_velocity_up;
      u_st_velocity_up.base = 0;
      u_st_velocity_up.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_velocity_up.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_velocity_up.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_velocity_up.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_velocity_up = u_st_velocity_up.real;
      offset += sizeof(this->st_velocity_up);
        memcpy( &(this->velocity_up[i]), &(this->st_velocity_up), sizeof(float));
      }
      uint8_t time_down_lengthT = *(inbuffer + offset++);
      if(time_down_lengthT > time_down_length)
        this->time_down = (float*)realloc(this->time_down, time_down_lengthT * sizeof(float));
      offset += 3;
      time_down_length = time_down_lengthT;
      for( uint8_t i = 0; i < time_down_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_time_down;
      u_st_time_down.base = 0;
      u_st_time_down.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_time_down.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_time_down.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_time_down.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_time_down = u_st_time_down.real;
      offset += sizeof(this->st_time_down);
        memcpy( &(this->time_down[i]), &(this->st_time_down), sizeof(float));
      }
      uint8_t effort_down_lengthT = *(inbuffer + offset++);
      if(effort_down_lengthT > effort_down_length)
        this->effort_down = (float*)realloc(this->effort_down, effort_down_lengthT * sizeof(float));
      offset += 3;
      effort_down_length = effort_down_lengthT;
      for( uint8_t i = 0; i < effort_down_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_effort_down;
      u_st_effort_down.base = 0;
      u_st_effort_down.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_effort_down.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_effort_down.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_effort_down.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_effort_down = u_st_effort_down.real;
      offset += sizeof(this->st_effort_down);
        memcpy( &(this->effort_down[i]), &(this->st_effort_down), sizeof(float));
      }
      uint8_t position_down_lengthT = *(inbuffer + offset++);
      if(position_down_lengthT > position_down_length)
        this->position_down = (float*)realloc(this->position_down, position_down_lengthT * sizeof(float));
      offset += 3;
      position_down_length = position_down_lengthT;
      for( uint8_t i = 0; i < position_down_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_position_down;
      u_st_position_down.base = 0;
      u_st_position_down.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_position_down.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_position_down.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_position_down.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_position_down = u_st_position_down.real;
      offset += sizeof(this->st_position_down);
        memcpy( &(this->position_down[i]), &(this->st_position_down), sizeof(float));
      }
      uint8_t velocity_down_lengthT = *(inbuffer + offset++);
      if(velocity_down_lengthT > velocity_down_length)
        this->velocity_down = (float*)realloc(this->velocity_down, velocity_down_lengthT * sizeof(float));
      offset += 3;
      velocity_down_length = velocity_down_lengthT;
      for( uint8_t i = 0; i < velocity_down_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_velocity_down;
      u_st_velocity_down.base = 0;
      u_st_velocity_down.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_velocity_down.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_velocity_down.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_velocity_down.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_velocity_down = u_st_velocity_down.real;
      offset += sizeof(this->st_velocity_down);
        memcpy( &(this->velocity_down[i]), &(this->st_velocity_down), sizeof(float));
      }
      uint8_t arg_name_lengthT = *(inbuffer + offset++);
      if(arg_name_lengthT > arg_name_length)
        this->arg_name = (char**)realloc(this->arg_name, arg_name_lengthT * sizeof(char*));
      offset += 3;
      arg_name_length = arg_name_lengthT;
      for( uint8_t i = 0; i < arg_name_length; i++){
      uint32_t length_st_arg_name;
      memcpy(&length_st_arg_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_arg_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_arg_name-1]=0;
      this->st_arg_name = (char *)(inbuffer + offset-1);
      offset += length_st_arg_name;
        memcpy( &(this->arg_name[i]), &(this->st_arg_name), sizeof(char*));
      }
      uint8_t arg_value_lengthT = *(inbuffer + offset++);
      if(arg_value_lengthT > arg_value_length)
        this->arg_value = (float*)realloc(this->arg_value, arg_value_lengthT * sizeof(float));
      offset += 3;
      arg_value_length = arg_value_lengthT;
      for( uint8_t i = 0; i < arg_value_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_arg_value;
      u_st_arg_value.base = 0;
      u_st_arg_value.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_arg_value.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_arg_value.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_arg_value.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_arg_value = u_st_arg_value.real;
      offset += sizeof(this->st_arg_value);
        memcpy( &(this->arg_value[i]), &(this->st_arg_value), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "joint_qualification_controllers/HysteresisData"; };
    const char * getMD5(){ return "55018edece2f193bc97f016db04a871b"; };

  };

}
#endif