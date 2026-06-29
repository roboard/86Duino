#ifndef _ROS_manipulation_msgs_ManipulationResult_h
#define _ROS_manipulation_msgs_ManipulationResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace manipulation_msgs
{

  class ManipulationResult : public ros::Msg
  {
    public:
      int32_t value;
      enum { SUCCESS =  1 };
      enum { UNFEASIBLE =  -1 };
      enum { FAILED =  -2 };
      enum { ERROR =  -3 };
      enum { ARM_MOVEMENT_PREVENTED =  -4 };
      enum { LIFT_FAILED =  -5 };
      enum { RETREAT_FAILED =  -6 };
      enum { CANCELLED =  -7 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_value;
      u_value.real = this->value;
      *(outbuffer + offset + 0) = (u_value.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_value.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_value.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_value.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->value);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_value;
      u_value.base = 0;
      u_value.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_value.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_value.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_value.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->value = u_value.real;
      offset += sizeof(this->value);
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/ManipulationResult"; };
    const char * getMD5(){ return "85f8d010e045fcb335637fc8fc59184b"; };

  };

}
#endif