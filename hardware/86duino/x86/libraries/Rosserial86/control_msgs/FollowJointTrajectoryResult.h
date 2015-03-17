#ifndef _ROS_control_msgs_FollowJointTrajectoryResult_h
#define _ROS_control_msgs_FollowJointTrajectoryResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace control_msgs
{

  class FollowJointTrajectoryResult : public ros::Msg
  {
    public:
      int32_t error_code;
      enum { SUCCESSFUL =  0 };
      enum { INVALID_GOAL =  -1 };
      enum { INVALID_JOINTS =  -2 };
      enum { OLD_HEADER_TIMESTAMP =  -3 };
      enum { PATH_TOLERANCE_VIOLATED =  -4 };
      enum { GOAL_TOLERANCE_VIOLATED =  -5 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
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

    const char * getType(){ return "control_msgs/FollowJointTrajectoryResult"; };
    const char * getMD5(){ return "6243274b5d629dc838814109754410d5"; };

  };

}
#endif