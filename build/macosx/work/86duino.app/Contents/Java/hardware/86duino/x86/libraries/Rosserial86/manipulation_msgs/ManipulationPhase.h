#ifndef _ROS_manipulation_msgs_ManipulationPhase_h
#define _ROS_manipulation_msgs_ManipulationPhase_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace manipulation_msgs
{

  class ManipulationPhase : public ros::Msg
  {
    public:
      int32_t phase;
      enum { CHECKING_FEASIBILITY =  0 };
      enum { MOVING_TO_PREGRASP =  1 };
      enum { MOVING_TO_GRASP =  2 };
      enum { CLOSING =  3 };
      enum { ADJUSTING_GRASP =  4 };
      enum { LIFTING =  5 };
      enum { MOVING_WITH_OBJECT =  6 };
      enum { MOVING_TO_PLACE =  7 };
      enum { PLACING =  8 };
      enum { OPENING =  9 };
      enum { RETREATING =  10 };
      enum { MOVING_WITHOUT_OBJECT =  11 };
      enum { SHAKING =  12 };
      enum { SUCCEEDED =  13 };
      enum { FAILED =  14 };
      enum { ABORTED =  15 };
      enum { HOLDING_OBJECT =  16 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_phase;
      u_phase.real = this->phase;
      *(outbuffer + offset + 0) = (u_phase.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_phase.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_phase.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_phase.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->phase);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_phase;
      u_phase.base = 0;
      u_phase.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_phase.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_phase.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_phase.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->phase = u_phase.real;
      offset += sizeof(this->phase);
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/ManipulationPhase"; };
    const char * getMD5(){ return "2c824c847a35d8fd9277d324a3723378"; };

  };

}
#endif