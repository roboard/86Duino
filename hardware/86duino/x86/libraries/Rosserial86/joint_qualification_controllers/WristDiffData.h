#ifndef _ROS_joint_qualification_controllers_WristDiffData_h
#define _ROS_joint_qualification_controllers_WristDiffData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "joint_qualification_controllers/WristRollTurn.h"

namespace joint_qualification_controllers
{

  class WristDiffData : public ros::Msg
  {
    public:
      char * flex_joint;
      char * roll_joint;
      uint8_t flex_pid_length;
      float st_flex_pid;
      float * flex_pid;
      uint8_t roll_pid_length;
      float st_roll_pid;
      float * roll_pid;
      uint8_t arg_name_length;
      char* st_arg_name;
      char* * arg_name;
      uint8_t arg_value_length;
      float st_arg_value;
      float * arg_value;
      joint_qualification_controllers::WristRollTurn left_turn;
      joint_qualification_controllers::WristRollTurn right_turn;
      bool timeout;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_flex_joint = strlen( (const char*) this->flex_joint);
      memcpy(outbuffer + offset, &length_flex_joint, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->flex_joint, length_flex_joint);
      offset += length_flex_joint;
      uint32_t length_roll_joint = strlen( (const char*) this->roll_joint);
      memcpy(outbuffer + offset, &length_roll_joint, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->roll_joint, length_roll_joint);
      offset += length_roll_joint;
      *(outbuffer + offset++) = flex_pid_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < flex_pid_length; i++){
      union {
        float real;
        uint32_t base;
      } u_flex_pidi;
      u_flex_pidi.real = this->flex_pid[i];
      *(outbuffer + offset + 0) = (u_flex_pidi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_flex_pidi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_flex_pidi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_flex_pidi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->flex_pid[i]);
      }
      *(outbuffer + offset++) = roll_pid_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < roll_pid_length; i++){
      union {
        float real;
        uint32_t base;
      } u_roll_pidi;
      u_roll_pidi.real = this->roll_pid[i];
      *(outbuffer + offset + 0) = (u_roll_pidi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_roll_pidi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_roll_pidi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_roll_pidi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->roll_pid[i]);
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
      offset += this->left_turn.serialize(outbuffer + offset);
      offset += this->right_turn.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_timeout;
      u_timeout.real = this->timeout;
      *(outbuffer + offset + 0) = (u_timeout.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->timeout);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_flex_joint;
      memcpy(&length_flex_joint, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_flex_joint; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_flex_joint-1]=0;
      this->flex_joint = (char *)(inbuffer + offset-1);
      offset += length_flex_joint;
      uint32_t length_roll_joint;
      memcpy(&length_roll_joint, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_roll_joint; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_roll_joint-1]=0;
      this->roll_joint = (char *)(inbuffer + offset-1);
      offset += length_roll_joint;
      uint8_t flex_pid_lengthT = *(inbuffer + offset++);
      if(flex_pid_lengthT > flex_pid_length)
        this->flex_pid = (float*)realloc(this->flex_pid, flex_pid_lengthT * sizeof(float));
      offset += 3;
      flex_pid_length = flex_pid_lengthT;
      for( uint8_t i = 0; i < flex_pid_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_flex_pid;
      u_st_flex_pid.base = 0;
      u_st_flex_pid.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_flex_pid.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_flex_pid.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_flex_pid.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_flex_pid = u_st_flex_pid.real;
      offset += sizeof(this->st_flex_pid);
        memcpy( &(this->flex_pid[i]), &(this->st_flex_pid), sizeof(float));
      }
      uint8_t roll_pid_lengthT = *(inbuffer + offset++);
      if(roll_pid_lengthT > roll_pid_length)
        this->roll_pid = (float*)realloc(this->roll_pid, roll_pid_lengthT * sizeof(float));
      offset += 3;
      roll_pid_length = roll_pid_lengthT;
      for( uint8_t i = 0; i < roll_pid_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_roll_pid;
      u_st_roll_pid.base = 0;
      u_st_roll_pid.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_roll_pid.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_roll_pid.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_roll_pid.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_roll_pid = u_st_roll_pid.real;
      offset += sizeof(this->st_roll_pid);
        memcpy( &(this->roll_pid[i]), &(this->st_roll_pid), sizeof(float));
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
      offset += this->left_turn.deserialize(inbuffer + offset);
      offset += this->right_turn.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_timeout;
      u_timeout.base = 0;
      u_timeout.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->timeout = u_timeout.real;
      offset += sizeof(this->timeout);
     return offset;
    }

    const char * getType(){ return "joint_qualification_controllers/WristDiffData"; };
    const char * getMD5(){ return "b5450bf7a09b17c68a893b9c02e710f1"; };

  };

}
#endif