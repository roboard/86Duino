#ifndef _ROS_pr2_plugs_msgs_VisionPlugDetectionGoal_h
#define _ROS_pr2_plugs_msgs_VisionPlugDetectionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"

namespace pr2_plugs_msgs
{

  class VisionPlugDetectionGoal : public ros::Msg
  {
    public:
      char * camera_name;
      geometry_msgs::PoseStamped prior;
      bool origin_on_right;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_camera_name = strlen( (const char*) this->camera_name);
      memcpy(outbuffer + offset, &length_camera_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->camera_name, length_camera_name);
      offset += length_camera_name;
      offset += this->prior.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_origin_on_right;
      u_origin_on_right.real = this->origin_on_right;
      *(outbuffer + offset + 0) = (u_origin_on_right.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->origin_on_right);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_camera_name;
      memcpy(&length_camera_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_camera_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_camera_name-1]=0;
      this->camera_name = (char *)(inbuffer + offset-1);
      offset += length_camera_name;
      offset += this->prior.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_origin_on_right;
      u_origin_on_right.base = 0;
      u_origin_on_right.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->origin_on_right = u_origin_on_right.real;
      offset += sizeof(this->origin_on_right);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/VisionPlugDetectionGoal"; };
    const char * getMD5(){ return "8fbdac7b0b7a87b2acacc5336c7245cd"; };

  };

}
#endif