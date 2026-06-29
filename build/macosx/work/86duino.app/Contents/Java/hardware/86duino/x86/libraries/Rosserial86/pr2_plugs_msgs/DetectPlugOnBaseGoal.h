#ifndef _ROS_pr2_plugs_msgs_DetectPlugOnBaseGoal_h
#define _ROS_pr2_plugs_msgs_DetectPlugOnBaseGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_plugs_msgs
{

  class DetectPlugOnBaseGoal : public ros::Msg
  {
    public:
      bool record_image;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_record_image;
      u_record_image.real = this->record_image;
      *(outbuffer + offset + 0) = (u_record_image.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->record_image);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_record_image;
      u_record_image.base = 0;
      u_record_image.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->record_image = u_record_image.real;
      offset += sizeof(this->record_image);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/DetectPlugOnBaseGoal"; };
    const char * getMD5(){ return "66e258c362b1d484c81c518f48e0fd97"; };

  };

}
#endif