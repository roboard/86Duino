#ifndef _ROS_pr2_common_action_msgs_ArmMoveIKGoal_h
#define _ROS_pr2_common_action_msgs_ArmMoveIKGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"
#include "sensor_msgs/JointState.h"
#include "ros/duration.h"

namespace pr2_common_action_msgs
{

  class ArmMoveIKGoal : public ros::Msg
  {
    public:
      geometry_msgs::PoseStamped pose;
      geometry_msgs::PoseStamped tool_frame;
      sensor_msgs::JointState ik_seed;
      ros::Duration ik_timeout;
      ros::Duration move_duration;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->pose.serialize(outbuffer + offset);
      offset += this->tool_frame.serialize(outbuffer + offset);
      offset += this->ik_seed.serialize(outbuffer + offset);
      *(outbuffer + offset + 0) = (this->ik_timeout.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->ik_timeout.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->ik_timeout.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->ik_timeout.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->ik_timeout.sec);
      *(outbuffer + offset + 0) = (this->ik_timeout.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->ik_timeout.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->ik_timeout.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->ik_timeout.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->ik_timeout.nsec);
      *(outbuffer + offset + 0) = (this->move_duration.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->move_duration.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->move_duration.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->move_duration.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->move_duration.sec);
      *(outbuffer + offset + 0) = (this->move_duration.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->move_duration.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->move_duration.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->move_duration.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->move_duration.nsec);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->pose.deserialize(inbuffer + offset);
      offset += this->tool_frame.deserialize(inbuffer + offset);
      offset += this->ik_seed.deserialize(inbuffer + offset);
      this->ik_timeout.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->ik_timeout.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->ik_timeout.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->ik_timeout.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->ik_timeout.sec);
      this->ik_timeout.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->ik_timeout.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->ik_timeout.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->ik_timeout.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->ik_timeout.nsec);
      this->move_duration.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->move_duration.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->move_duration.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->move_duration.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->move_duration.sec);
      this->move_duration.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->move_duration.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->move_duration.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->move_duration.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->move_duration.nsec);
     return offset;
    }

    const char * getType(){ return "pr2_common_action_msgs/ArmMoveIKGoal"; };
    const char * getMD5(){ return "659cdac4f142756518faf4644a34bdda"; };

  };

}
#endif