#ifndef _ROS_pano_ros_StitchFeedback_h
#define _ROS_pano_ros_StitchFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pano_ros
{

  class StitchFeedback : public ros::Msg
  {
    public:
      float percent_complete;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_percent_complete;
      u_percent_complete.real = this->percent_complete;
      *(outbuffer + offset + 0) = (u_percent_complete.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_percent_complete.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_percent_complete.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_percent_complete.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->percent_complete);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_percent_complete;
      u_percent_complete.base = 0;
      u_percent_complete.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_percent_complete.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_percent_complete.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_percent_complete.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->percent_complete = u_percent_complete.real;
      offset += sizeof(this->percent_complete);
     return offset;
    }

    const char * getType(){ return "pano_ros/StitchFeedback"; };
    const char * getMD5(){ return "d342375c60a5a58d3bc32664070a1368"; };

  };

}
#endif