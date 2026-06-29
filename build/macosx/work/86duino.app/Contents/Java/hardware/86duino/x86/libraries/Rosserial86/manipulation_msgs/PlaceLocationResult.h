#ifndef _ROS_manipulation_msgs_PlaceLocationResult_h
#define _ROS_manipulation_msgs_PlaceLocationResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace manipulation_msgs
{

  class PlaceLocationResult : public ros::Msg
  {
    public:
      int32_t result_code;
      bool continuation_possible;
      enum { SUCCESS =  1 };
      enum { PLACE_OUT_OF_REACH =  2 };
      enum { PLACE_IN_COLLISION =  3 };
      enum { PLACE_UNFEASIBLE =  4 };
      enum { PREPLACE_OUT_OF_REACH =  5 };
      enum { PREPLACE_IN_COLLISION =  6 };
      enum { PREPLACE_UNFEASIBLE =  7 };
      enum { RETREAT_OUT_OF_REACH =  8 };
      enum { RETREAT_IN_COLLISION =  9 };
      enum { RETREAT_UNFEASIBLE =  10 };
      enum { MOVE_ARM_FAILED =  11 };
      enum { PLACE_FAILED =  12 };
      enum { RETREAT_FAILED =  13 };

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

    const char * getType(){ return "manipulation_msgs/PlaceLocationResult"; };
    const char * getMD5(){ return "8dd9edc3a2a98cab298ca81031224cda"; };

  };

}
#endif