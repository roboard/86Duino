#ifndef _ROS_pr2_plugs_msgs_WigglePlugGoal_h
#define _ROS_pr2_plugs_msgs_WigglePlugGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Pose.h"
#include "ros/duration.h"

namespace pr2_plugs_msgs
{

  class WigglePlugGoal : public ros::Msg
  {
    public:
      geometry_msgs::Pose gripper_to_plug;
      geometry_msgs::Pose base_to_outlet;
      ros::Duration wiggle_period;
      int32_t insert;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->gripper_to_plug.serialize(outbuffer + offset);
      offset += this->base_to_outlet.serialize(outbuffer + offset);
      *(outbuffer + offset + 0) = (this->wiggle_period.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->wiggle_period.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->wiggle_period.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->wiggle_period.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->wiggle_period.sec);
      *(outbuffer + offset + 0) = (this->wiggle_period.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->wiggle_period.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->wiggle_period.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->wiggle_period.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->wiggle_period.nsec);
      union {
        int32_t real;
        uint32_t base;
      } u_insert;
      u_insert.real = this->insert;
      *(outbuffer + offset + 0) = (u_insert.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_insert.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_insert.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_insert.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->insert);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->gripper_to_plug.deserialize(inbuffer + offset);
      offset += this->base_to_outlet.deserialize(inbuffer + offset);
      this->wiggle_period.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->wiggle_period.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->wiggle_period.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->wiggle_period.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->wiggle_period.sec);
      this->wiggle_period.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->wiggle_period.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->wiggle_period.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->wiggle_period.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->wiggle_period.nsec);
      union {
        int32_t real;
        uint32_t base;
      } u_insert;
      u_insert.base = 0;
      u_insert.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_insert.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_insert.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_insert.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->insert = u_insert.real;
      offset += sizeof(this->insert);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/WigglePlugGoal"; };
    const char * getMD5(){ return "e97cf54fc3e38bc45e2988c6aac5112c"; };

  };

}
#endif