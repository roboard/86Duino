#ifndef _ROS_pr2_common_action_msgs_TuckArmsGoal_h
#define _ROS_pr2_common_action_msgs_TuckArmsGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_common_action_msgs
{

  class TuckArmsGoal : public ros::Msg
  {
    public:
      bool tuck_left;
      bool tuck_right;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_tuck_left;
      u_tuck_left.real = this->tuck_left;
      *(outbuffer + offset + 0) = (u_tuck_left.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->tuck_left);
      union {
        bool real;
        uint8_t base;
      } u_tuck_right;
      u_tuck_right.real = this->tuck_right;
      *(outbuffer + offset + 0) = (u_tuck_right.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->tuck_right);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_tuck_left;
      u_tuck_left.base = 0;
      u_tuck_left.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->tuck_left = u_tuck_left.real;
      offset += sizeof(this->tuck_left);
      union {
        bool real;
        uint8_t base;
      } u_tuck_right;
      u_tuck_right.base = 0;
      u_tuck_right.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->tuck_right = u_tuck_right.real;
      offset += sizeof(this->tuck_right);
     return offset;
    }

    const char * getType(){ return "pr2_common_action_msgs/TuckArmsGoal"; };
    const char * getMD5(){ return "a07b11078a50f9881dc3004ca1174834"; };

  };

}
#endif