#ifndef _ROS_moveit_msgs_MoveGroupResult_h
#define _ROS_moveit_msgs_MoveGroupResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/MoveItErrorCodes.h"
#include "moveit_msgs/RobotState.h"
#include "moveit_msgs/RobotTrajectory.h"

namespace moveit_msgs
{

  class MoveGroupResult : public ros::Msg
  {
    public:
      moveit_msgs::MoveItErrorCodes error_code;
      moveit_msgs::RobotState trajectory_start;
      moveit_msgs::RobotTrajectory planned_trajectory;
      moveit_msgs::RobotTrajectory executed_trajectory;
      float planning_time;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->error_code.serialize(outbuffer + offset);
      offset += this->trajectory_start.serialize(outbuffer + offset);
      offset += this->planned_trajectory.serialize(outbuffer + offset);
      offset += this->executed_trajectory.serialize(outbuffer + offset);
      int32_t * val_planning_time = (int32_t *) &(this->planning_time);
      int32_t exp_planning_time = (((*val_planning_time)>>23)&255);
      if(exp_planning_time != 0)
        exp_planning_time += 1023-127;
      int32_t sig_planning_time = *val_planning_time;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_planning_time<<5) & 0xff;
      *(outbuffer + offset++) = (sig_planning_time>>3) & 0xff;
      *(outbuffer + offset++) = (sig_planning_time>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_planning_time<<4) & 0xF0) | ((sig_planning_time>>19)&0x0F);
      *(outbuffer + offset++) = (exp_planning_time>>4) & 0x7F;
      if(this->planning_time < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->error_code.deserialize(inbuffer + offset);
      offset += this->trajectory_start.deserialize(inbuffer + offset);
      offset += this->planned_trajectory.deserialize(inbuffer + offset);
      offset += this->executed_trajectory.deserialize(inbuffer + offset);
      uint32_t * val_planning_time = (uint32_t*) &(this->planning_time);
      offset += 3;
      *val_planning_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_planning_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_planning_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_planning_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_planning_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_planning_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_planning_time !=0)
        *val_planning_time |= ((exp_planning_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->planning_time = -this->planning_time;
     return offset;
    }

    const char * getType(){ return "moveit_msgs/MoveGroupResult"; };
    const char * getMD5(){ return "34098589d402fee7ae9c3fd413e5a6c6"; };

  };

}
#endif