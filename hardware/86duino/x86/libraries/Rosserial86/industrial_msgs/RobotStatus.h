#ifndef _ROS_industrial_msgs_RobotStatus_h
#define _ROS_industrial_msgs_RobotStatus_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "industrial_msgs/RobotMode.h"
#include "industrial_msgs/TriState.h"

namespace industrial_msgs
{

  class RobotStatus : public ros::Msg
  {
    public:
      std_msgs::Header header;
      industrial_msgs::RobotMode mode;
      industrial_msgs::TriState e_stopped;
      industrial_msgs::TriState drives_powered;
      industrial_msgs::TriState motion_possible;
      industrial_msgs::TriState in_motion;
      industrial_msgs::TriState in_error;
      int32_t error_code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->mode.serialize(outbuffer + offset);
      offset += this->e_stopped.serialize(outbuffer + offset);
      offset += this->drives_powered.serialize(outbuffer + offset);
      offset += this->motion_possible.serialize(outbuffer + offset);
      offset += this->in_motion.serialize(outbuffer + offset);
      offset += this->in_error.serialize(outbuffer + offset);
      union {
        int32_t real;
        uint32_t base;
      } u_error_code;
      u_error_code.real = this->error_code;
      *(outbuffer + offset + 0) = (u_error_code.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_error_code.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_error_code.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_error_code.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->error_code);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->mode.deserialize(inbuffer + offset);
      offset += this->e_stopped.deserialize(inbuffer + offset);
      offset += this->drives_powered.deserialize(inbuffer + offset);
      offset += this->motion_possible.deserialize(inbuffer + offset);
      offset += this->in_motion.deserialize(inbuffer + offset);
      offset += this->in_error.deserialize(inbuffer + offset);
      union {
        int32_t real;
        uint32_t base;
      } u_error_code;
      u_error_code.base = 0;
      u_error_code.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_error_code.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_error_code.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_error_code.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->error_code = u_error_code.real;
      offset += sizeof(this->error_code);
     return offset;
    }

    const char * getType(){ return "industrial_msgs/RobotStatus"; };
    const char * getMD5(){ return "b733cb45a38101840b75c4c0d6093d19"; };

  };

}
#endif