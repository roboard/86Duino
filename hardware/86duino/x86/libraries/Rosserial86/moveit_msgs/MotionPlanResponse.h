#ifndef _ROS_moveit_msgs_MotionPlanResponse_h
#define _ROS_moveit_msgs_MotionPlanResponse_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/RobotState.h"
#include "moveit_msgs/RobotTrajectory.h"
#include "moveit_msgs/MoveItErrorCodes.h"

namespace moveit_msgs
{

  class MotionPlanResponse : public ros::Msg
  {
    public:
      moveit_msgs::RobotState trajectory_start;
      char * group_name;
      moveit_msgs::RobotTrajectory trajectory;
      float planning_time;
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
      offset += this->trajectory.serialize(outbuffer + offset);
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
      offset += this->trajectory.deserialize(inbuffer + offset);
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
      offset += this->error_code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "moveit_msgs/MotionPlanResponse"; };
    const char * getMD5(){ return "e493d20ab41424c48f671e152c70fc74"; };

  };

}
#endif