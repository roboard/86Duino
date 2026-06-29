#ifndef _ROS_joint_qualification_controllers_CounterbalanceTestData_h
#define _ROS_joint_qualification_controllers_CounterbalanceTestData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "joint_qualification_controllers/CBRunData.h"

namespace joint_qualification_controllers
{

  class CounterbalanceTestData : public ros::Msg
  {
    public:
      char * lift_joint;
      char * flex_joint;
      float lift_amplitude;
      float flex_amplitude;
      bool timeout_hit;
      bool flex_test;
      uint8_t arg_name_length;
      char* st_arg_name;
      char* * arg_name;
      uint8_t arg_value_length;
      float st_arg_value;
      float * arg_value;
      uint8_t lift_data_length;
      joint_qualification_controllers::CBRunData st_lift_data;
      joint_qualification_controllers::CBRunData * lift_data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_lift_joint = strlen( (const char*) this->lift_joint);
      memcpy(outbuffer + offset, &length_lift_joint, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->lift_joint, length_lift_joint);
      offset += length_lift_joint;
      uint32_t length_flex_joint = strlen( (const char*) this->flex_joint);
      memcpy(outbuffer + offset, &length_flex_joint, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->flex_joint, length_flex_joint);
      offset += length_flex_joint;
      union {
        float real;
        uint32_t base;
      } u_lift_amplitude;
      u_lift_amplitude.real = this->lift_amplitude;
      *(outbuffer + offset + 0) = (u_lift_amplitude.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_lift_amplitude.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_lift_amplitude.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_lift_amplitude.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->lift_amplitude);
      union {
        float real;
        uint32_t base;
      } u_flex_amplitude;
      u_flex_amplitude.real = this->flex_amplitude;
      *(outbuffer + offset + 0) = (u_flex_amplitude.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_flex_amplitude.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_flex_amplitude.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_flex_amplitude.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->flex_amplitude);
      union {
        bool real;
        uint8_t base;
      } u_timeout_hit;
      u_timeout_hit.real = this->timeout_hit;
      *(outbuffer + offset + 0) = (u_timeout_hit.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->timeout_hit);
      union {
        bool real;
        uint8_t base;
      } u_flex_test;
      u_flex_test.real = this->flex_test;
      *(outbuffer + offset + 0) = (u_flex_test.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->flex_test);
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
      *(outbuffer + offset++) = lift_data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < lift_data_length; i++){
      offset += this->lift_data[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_lift_joint;
      memcpy(&length_lift_joint, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_lift_joint; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_lift_joint-1]=0;
      this->lift_joint = (char *)(inbuffer + offset-1);
      offset += length_lift_joint;
      uint32_t length_flex_joint;
      memcpy(&length_flex_joint, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_flex_joint; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_flex_joint-1]=0;
      this->flex_joint = (char *)(inbuffer + offset-1);
      offset += length_flex_joint;
      union {
        float real;
        uint32_t base;
      } u_lift_amplitude;
      u_lift_amplitude.base = 0;
      u_lift_amplitude.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_lift_amplitude.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_lift_amplitude.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_lift_amplitude.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->lift_amplitude = u_lift_amplitude.real;
      offset += sizeof(this->lift_amplitude);
      union {
        float real;
        uint32_t base;
      } u_flex_amplitude;
      u_flex_amplitude.base = 0;
      u_flex_amplitude.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_flex_amplitude.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_flex_amplitude.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_flex_amplitude.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->flex_amplitude = u_flex_amplitude.real;
      offset += sizeof(this->flex_amplitude);
      union {
        bool real;
        uint8_t base;
      } u_timeout_hit;
      u_timeout_hit.base = 0;
      u_timeout_hit.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->timeout_hit = u_timeout_hit.real;
      offset += sizeof(this->timeout_hit);
      union {
        bool real;
        uint8_t base;
      } u_flex_test;
      u_flex_test.base = 0;
      u_flex_test.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->flex_test = u_flex_test.real;
      offset += sizeof(this->flex_test);
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
      uint8_t lift_data_lengthT = *(inbuffer + offset++);
      if(lift_data_lengthT > lift_data_length)
        this->lift_data = (joint_qualification_controllers::CBRunData*)realloc(this->lift_data, lift_data_lengthT * sizeof(joint_qualification_controllers::CBRunData));
      offset += 3;
      lift_data_length = lift_data_lengthT;
      for( uint8_t i = 0; i < lift_data_length; i++){
      offset += this->st_lift_data.deserialize(inbuffer + offset);
        memcpy( &(this->lift_data[i]), &(this->st_lift_data), sizeof(joint_qualification_controllers::CBRunData));
      }
     return offset;
    }

    const char * getType(){ return "joint_qualification_controllers/CounterbalanceTestData"; };
    const char * getMD5(){ return "ab80d40971aa9176fd2c9fb75aeb93cf"; };

  };

}
#endif