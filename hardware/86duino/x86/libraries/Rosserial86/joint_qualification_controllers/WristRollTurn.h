#ifndef _ROS_joint_qualification_controllers_WristRollTurn_h
#define _ROS_joint_qualification_controllers_WristRollTurn_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace joint_qualification_controllers
{

  class WristRollTurn : public ros::Msg
  {
    public:
      uint8_t time_length;
      float st_time;
      float * time;
      uint8_t flex_position_length;
      float st_flex_position;
      float * flex_position;
      uint8_t flex_effort_length;
      float st_flex_effort;
      float * flex_effort;
      uint8_t flex_cmd_length;
      float st_flex_cmd;
      float * flex_cmd;
      uint8_t roll_position_length;
      float st_roll_position;
      float * roll_position;
      uint8_t roll_effort_length;
      float st_roll_effort;
      float * roll_effort;
      uint8_t roll_cmd_length;
      float st_roll_cmd;
      float * roll_cmd;
      uint8_t roll_velocity_length;
      float st_roll_velocity;
      float * roll_velocity;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = time_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < time_length; i++){
      union {
        float real;
        uint32_t base;
      } u_timei;
      u_timei.real = this->time[i];
      *(outbuffer + offset + 0) = (u_timei.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_timei.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_timei.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_timei.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->time[i]);
      }
      *(outbuffer + offset++) = flex_position_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < flex_position_length; i++){
      union {
        float real;
        uint32_t base;
      } u_flex_positioni;
      u_flex_positioni.real = this->flex_position[i];
      *(outbuffer + offset + 0) = (u_flex_positioni.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_flex_positioni.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_flex_positioni.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_flex_positioni.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->flex_position[i]);
      }
      *(outbuffer + offset++) = flex_effort_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < flex_effort_length; i++){
      union {
        float real;
        uint32_t base;
      } u_flex_efforti;
      u_flex_efforti.real = this->flex_effort[i];
      *(outbuffer + offset + 0) = (u_flex_efforti.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_flex_efforti.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_flex_efforti.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_flex_efforti.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->flex_effort[i]);
      }
      *(outbuffer + offset++) = flex_cmd_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < flex_cmd_length; i++){
      union {
        float real;
        uint32_t base;
      } u_flex_cmdi;
      u_flex_cmdi.real = this->flex_cmd[i];
      *(outbuffer + offset + 0) = (u_flex_cmdi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_flex_cmdi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_flex_cmdi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_flex_cmdi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->flex_cmd[i]);
      }
      *(outbuffer + offset++) = roll_position_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < roll_position_length; i++){
      union {
        float real;
        uint32_t base;
      } u_roll_positioni;
      u_roll_positioni.real = this->roll_position[i];
      *(outbuffer + offset + 0) = (u_roll_positioni.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_roll_positioni.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_roll_positioni.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_roll_positioni.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->roll_position[i]);
      }
      *(outbuffer + offset++) = roll_effort_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < roll_effort_length; i++){
      union {
        float real;
        uint32_t base;
      } u_roll_efforti;
      u_roll_efforti.real = this->roll_effort[i];
      *(outbuffer + offset + 0) = (u_roll_efforti.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_roll_efforti.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_roll_efforti.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_roll_efforti.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->roll_effort[i]);
      }
      *(outbuffer + offset++) = roll_cmd_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < roll_cmd_length; i++){
      union {
        float real;
        uint32_t base;
      } u_roll_cmdi;
      u_roll_cmdi.real = this->roll_cmd[i];
      *(outbuffer + offset + 0) = (u_roll_cmdi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_roll_cmdi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_roll_cmdi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_roll_cmdi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->roll_cmd[i]);
      }
      *(outbuffer + offset++) = roll_velocity_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < roll_velocity_length; i++){
      union {
        float real;
        uint32_t base;
      } u_roll_velocityi;
      u_roll_velocityi.real = this->roll_velocity[i];
      *(outbuffer + offset + 0) = (u_roll_velocityi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_roll_velocityi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_roll_velocityi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_roll_velocityi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->roll_velocity[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t time_lengthT = *(inbuffer + offset++);
      if(time_lengthT > time_length)
        this->time = (float*)realloc(this->time, time_lengthT * sizeof(float));
      offset += 3;
      time_length = time_lengthT;
      for( uint8_t i = 0; i < time_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_time;
      u_st_time.base = 0;
      u_st_time.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_time.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_time.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_time.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_time = u_st_time.real;
      offset += sizeof(this->st_time);
        memcpy( &(this->time[i]), &(this->st_time), sizeof(float));
      }
      uint8_t flex_position_lengthT = *(inbuffer + offset++);
      if(flex_position_lengthT > flex_position_length)
        this->flex_position = (float*)realloc(this->flex_position, flex_position_lengthT * sizeof(float));
      offset += 3;
      flex_position_length = flex_position_lengthT;
      for( uint8_t i = 0; i < flex_position_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_flex_position;
      u_st_flex_position.base = 0;
      u_st_flex_position.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_flex_position.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_flex_position.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_flex_position.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_flex_position = u_st_flex_position.real;
      offset += sizeof(this->st_flex_position);
        memcpy( &(this->flex_position[i]), &(this->st_flex_position), sizeof(float));
      }
      uint8_t flex_effort_lengthT = *(inbuffer + offset++);
      if(flex_effort_lengthT > flex_effort_length)
        this->flex_effort = (float*)realloc(this->flex_effort, flex_effort_lengthT * sizeof(float));
      offset += 3;
      flex_effort_length = flex_effort_lengthT;
      for( uint8_t i = 0; i < flex_effort_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_flex_effort;
      u_st_flex_effort.base = 0;
      u_st_flex_effort.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_flex_effort.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_flex_effort.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_flex_effort.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_flex_effort = u_st_flex_effort.real;
      offset += sizeof(this->st_flex_effort);
        memcpy( &(this->flex_effort[i]), &(this->st_flex_effort), sizeof(float));
      }
      uint8_t flex_cmd_lengthT = *(inbuffer + offset++);
      if(flex_cmd_lengthT > flex_cmd_length)
        this->flex_cmd = (float*)realloc(this->flex_cmd, flex_cmd_lengthT * sizeof(float));
      offset += 3;
      flex_cmd_length = flex_cmd_lengthT;
      for( uint8_t i = 0; i < flex_cmd_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_flex_cmd;
      u_st_flex_cmd.base = 0;
      u_st_flex_cmd.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_flex_cmd.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_flex_cmd.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_flex_cmd.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_flex_cmd = u_st_flex_cmd.real;
      offset += sizeof(this->st_flex_cmd);
        memcpy( &(this->flex_cmd[i]), &(this->st_flex_cmd), sizeof(float));
      }
      uint8_t roll_position_lengthT = *(inbuffer + offset++);
      if(roll_position_lengthT > roll_position_length)
        this->roll_position = (float*)realloc(this->roll_position, roll_position_lengthT * sizeof(float));
      offset += 3;
      roll_position_length = roll_position_lengthT;
      for( uint8_t i = 0; i < roll_position_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_roll_position;
      u_st_roll_position.base = 0;
      u_st_roll_position.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_roll_position.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_roll_position.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_roll_position.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_roll_position = u_st_roll_position.real;
      offset += sizeof(this->st_roll_position);
        memcpy( &(this->roll_position[i]), &(this->st_roll_position), sizeof(float));
      }
      uint8_t roll_effort_lengthT = *(inbuffer + offset++);
      if(roll_effort_lengthT > roll_effort_length)
        this->roll_effort = (float*)realloc(this->roll_effort, roll_effort_lengthT * sizeof(float));
      offset += 3;
      roll_effort_length = roll_effort_lengthT;
      for( uint8_t i = 0; i < roll_effort_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_roll_effort;
      u_st_roll_effort.base = 0;
      u_st_roll_effort.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_roll_effort.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_roll_effort.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_roll_effort.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_roll_effort = u_st_roll_effort.real;
      offset += sizeof(this->st_roll_effort);
        memcpy( &(this->roll_effort[i]), &(this->st_roll_effort), sizeof(float));
      }
      uint8_t roll_cmd_lengthT = *(inbuffer + offset++);
      if(roll_cmd_lengthT > roll_cmd_length)
        this->roll_cmd = (float*)realloc(this->roll_cmd, roll_cmd_lengthT * sizeof(float));
      offset += 3;
      roll_cmd_length = roll_cmd_lengthT;
      for( uint8_t i = 0; i < roll_cmd_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_roll_cmd;
      u_st_roll_cmd.base = 0;
      u_st_roll_cmd.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_roll_cmd.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_roll_cmd.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_roll_cmd.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_roll_cmd = u_st_roll_cmd.real;
      offset += sizeof(this->st_roll_cmd);
        memcpy( &(this->roll_cmd[i]), &(this->st_roll_cmd), sizeof(float));
      }
      uint8_t roll_velocity_lengthT = *(inbuffer + offset++);
      if(roll_velocity_lengthT > roll_velocity_length)
        this->roll_velocity = (float*)realloc(this->roll_velocity, roll_velocity_lengthT * sizeof(float));
      offset += 3;
      roll_velocity_length = roll_velocity_lengthT;
      for( uint8_t i = 0; i < roll_velocity_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_roll_velocity;
      u_st_roll_velocity.base = 0;
      u_st_roll_velocity.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_roll_velocity.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_roll_velocity.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_roll_velocity.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_roll_velocity = u_st_roll_velocity.real;
      offset += sizeof(this->st_roll_velocity);
        memcpy( &(this->roll_velocity[i]), &(this->st_roll_velocity), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "joint_qualification_controllers/WristRollTurn"; };
    const char * getMD5(){ return "27db8c0ca950f1334a70a45b5bc8427f"; };

  };

}
#endif