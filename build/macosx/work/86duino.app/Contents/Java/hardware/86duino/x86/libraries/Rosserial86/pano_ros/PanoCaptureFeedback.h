#ifndef _ROS_pano_ros_PanoCaptureFeedback_h
#define _ROS_pano_ros_PanoCaptureFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pano_ros
{

  class PanoCaptureFeedback : public ros::Msg
  {
    public:
      float n_captures;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_n_captures;
      u_n_captures.real = this->n_captures;
      *(outbuffer + offset + 0) = (u_n_captures.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_n_captures.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_n_captures.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_n_captures.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->n_captures);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_n_captures;
      u_n_captures.base = 0;
      u_n_captures.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_n_captures.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_n_captures.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_n_captures.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->n_captures = u_n_captures.real;
      offset += sizeof(this->n_captures);
     return offset;
    }

    const char * getType(){ return "pano_ros/PanoCaptureFeedback"; };
    const char * getMD5(){ return "22ff7abf8b5e4a280047b5a08afb8cf1"; };

  };

}
#endif