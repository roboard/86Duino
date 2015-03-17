#ifndef _ROS_moveit_msgs_MotionPlanDetailedResponse_h
#define _ROS_moveit_msgs_MotionPlanDetailedResponse_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/RobotState.h"
#include "moveit_msgs/RobotTrajectory.h"
#include "moveit_msgs/MoveItErrorCodes.h"

namespace moveit_msgs
{

  class MotionPlanDetailedResponse : public ros::Msg
  {
    public:
      moveit_msgs::RobotState trajectory_start;
      char * group_name;
      uint8_t trajectory_length;
      moveit_msgs::RobotTrajectory st_trajectory;
      moveit_msgs::RobotTrajectory * trajectory;
      uint8_t description_length;
      char* st_description;
      char* * description;
      uint8_t processing_time_length;
      float st_processing_time;
      float * processing_time;
      moveit_msgs::MoveItErrorCodes error_code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->trajectory_start.serialize(outbuffer + offset);
      uint32_t length_group_name = strlen( (const char*) this->group_name);
      memcpy(outbuffer + offset, &length_group_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->group_name, length_group_name);
      offset += length_group_name;
      *(outbuffer + offset++) = trajectory_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < trajectory_length; i++){
      offset += this->trajectory[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = description_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < description_length; i++){
      uint32_t length_descriptioni = strlen( (const char*) this->description[i]);
      memcpy(outbuffer + offset, &length_descriptioni, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->description[i], length_descriptioni);
      offset += length_descriptioni;
      }
      *(outbuffer + offset++) = processing_time_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < processing_time_length; i++){
      int32_t * val_processing_timei = (int32_t *) &(this->processing_time[i]);
      int32_t exp_processing_timei = (((*val_processing_timei)>>23)&255);
      if(exp_processing_timei != 0)
        exp_processing_timei += 1023-127;
      int32_t sig_processing_timei = *val_processing_timei;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_processing_timei<<5) & 0xff;
      *(outbuffer + offset++) = (sig_processing_timei>>3) & 0xff;
      *(outbuffer + offset++) = (sig_processing_timei>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_processing_timei<<4) & 0xF0) | ((sig_processing_timei>>19)&0x0F);
      *(outbuffer + offset++) = (exp_processing_timei>>4) & 0x7F;
      if(this->processing_time[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      offset += this->error_code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->trajectory_start.deserialize(inbuffer + offset);
      uint32_t length_group_name;
      memcpy(&length_group_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_group_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_group_name-1]=0;
      this->group_name = (char *)(inbuffer + offset-1);
      offset += length_group_name;
      uint8_t trajectory_lengthT = *(inbuffer + offset++);
      if(trajectory_lengthT > trajectory_length)
        this->trajectory = (moveit_msgs::RobotTrajectory*)realloc(this->trajectory, trajectory_lengthT * sizeof(moveit_msgs::RobotTrajectory));
      offset += 3;
      trajectory_length = trajectory_lengthT;
      for( uint8_t i = 0; i < trajectory_length; i++){
      offset += this->st_trajectory.deserialize(inbuffer + offset);
        memcpy( &(this->trajectory[i]), &(this->st_trajectory), sizeof(moveit_msgs::RobotTrajectory));
      }
      uint8_t description_lengthT = *(inbuffer + offset++);
      if(description_lengthT > description_length)
        this->description = (char**)realloc(this->description, description_lengthT * sizeof(char*));
      offset += 3;
      description_length = description_lengthT;
      for( uint8_t i = 0; i < description_length; i++){
      uint32_t length_st_description;
      memcpy(&length_st_description, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_description; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_description-1]=0;
      this->st_description = (char *)(inbuffer + offset-1);
      offset += length_st_description;
        memcpy( &(this->description[i]), &(this->st_description), sizeof(char*));
      }
      uint8_t processing_time_lengthT = *(inbuffer + offset++);
      if(processing_time_lengthT > processing_time_length)
        this->processing_time = (float*)realloc(this->processing_time, processing_time_lengthT * sizeof(float));
      offset += 3;
      processing_time_length = processing_time_lengthT;
      for( uint8_t i = 0; i < processing_time_length; i++){
      uint32_t * val_st_processing_time = (uint32_t*) &(this->st_processing_time);
      offset += 3;
      *val_st_processing_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_processing_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_processing_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_processing_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_processing_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_processing_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_processing_time !=0)
        *val_st_processing_time |= ((exp_st_processing_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_processing_time = -this->st_processing_time;
        memcpy( &(this->processing_time[i]), &(this->st_processing_time), sizeof(float));
      }
      offset += this->error_code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "moveit_msgs/MotionPlanDetailedResponse"; };
    const char * getMD5(){ return "7b84c374bb2e37bdc0eba664f7636a8f"; };

  };

}
#endif