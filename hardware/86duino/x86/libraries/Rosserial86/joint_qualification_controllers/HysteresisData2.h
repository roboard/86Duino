#ifndef _ROS_joint_qualification_controllers_HysteresisData2_h
#define _ROS_joint_qualification_controllers_HysteresisData2_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "joint_qualification_controllers/HysteresisRun.h"

namespace joint_qualification_controllers
{

  class HysteresisData2 : public ros::Msg
  {
    public:
      char * joint_name;
      uint8_t runs_length;
      joint_qualification_controllers::HysteresisRun st_runs;
      joint_qualification_controllers::HysteresisRun * runs;
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
      *(outbuffer + offset++) = runs_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < runs_length; i++){
      offset += this->runs[i].serialize(outbuffer + offset);
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
      uint8_t runs_lengthT = *(inbuffer + offset++);
      if(runs_lengthT > runs_length)
        this->runs = (joint_qualification_controllers::HysteresisRun*)realloc(this->runs, runs_lengthT * sizeof(joint_qualification_controllers::HysteresisRun));
      offset += 3;
      runs_length = runs_lengthT;
      for( uint8_t i = 0; i < runs_length; i++){
      offset += this->st_runs.deserialize(inbuffer + offset);
        memcpy( &(this->runs[i]), &(this->st_runs), sizeof(joint_qualification_controllers::HysteresisRun));
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

    const char * getType(){ return "joint_qualification_controllers/HysteresisData2"; };
    const char * getMD5(){ return "e58eacdd7f3ac3c02f9d494da1e59355"; };

  };

}
#endif