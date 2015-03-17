#ifndef _ROS_pano_ros_StitchResult_h
#define _ROS_pano_ros_StitchResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pano_ros
{

  class StitchResult : public ros::Msg
  {
    public:
      int32_t result_flags;
      enum { PANO_SUCCESS = 1 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_result_flags;
      u_result_flags.real = this->result_flags;
      *(outbuffer + offset + 0) = (u_result_flags.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_result_flags.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_result_flags.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_result_flags.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->result_flags);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_result_flags;
      u_result_flags.base = 0;
      u_result_flags.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_result_flags.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_result_flags.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_result_flags.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->result_flags = u_result_flags.real;
      offset += sizeof(this->result_flags);
     return offset;
    }

    const char * getType(){ return "pano_ros/StitchResult"; };
    const char * getMD5(){ return "c34b583b7e4652b9ca26654583ffb4ce"; };

  };

}
#endif