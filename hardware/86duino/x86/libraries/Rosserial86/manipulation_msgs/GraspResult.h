#ifndef _ROS_manipulation_msgs_GraspResult_h
#define _ROS_manipulation_msgs_GraspResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace manipulation_msgs
{

  class GraspResult : public ros::Msg
  {
    public:
      int32_t result_code;
      bool continuation_possible;
      enum { SUCCESS =  1 };
      enum { GRASP_OUT_OF_REACH =  2 };
      enum { GRASP_IN_COLLISION =  3 };
      enum { GRASP_UNFEASIBLE =  4 };
      enum { PREGRASP_OUT_OF_REACH =  5 };
      enum { PREGRASP_IN_COLLISION =  6 };
      enum { PREGRASP_UNFEASIBLE =  7 };
      enum { LIFT_OUT_OF_REACH =  8 };
      enum { LIFT_IN_COLLISION =  9 };
      enum { LIFT_UNFEASIBLE =  10 };
      enum { MOVE_ARM_FAILED =  11 };
      enum { GRASP_FAILED =  12 };
      enum { LIFT_FAILED =  13 };
      enum { RETREAT_FAILED =  14 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_result_code;
      u_result_code.real = this->result_code;
      *(outbuffer + offset + 0) = (u_result_code.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_result_code.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_result_code.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_result_code.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->result_code);
      union {
        bool real;
        uint8_t base;
      } u_continuation_possible;
      u_continuation_possible.real = this->continuation_possible;
      *(outbuffer + offset + 0) = (u_continuation_possible.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->continuation_possible);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_result_code;
      u_result_code.base = 0;
      u_result_code.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_result_code.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_result_code.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_result_code.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->result_code = u_result_code.real;
      offset += sizeof(this->result_code);
      union {
        bool real;
        uint8_t base;
      } u_continuation_possible;
      u_continuation_possible.base = 0;
      u_continuation_possible.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->continuation_possible = u_continuation_possible.real;
      offset += sizeof(this->continuation_possible);
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/GraspResult"; };
    const char * getMD5(){ return "c8a909da895cdddc0630aafd59848191"; };

  };

}
#endif