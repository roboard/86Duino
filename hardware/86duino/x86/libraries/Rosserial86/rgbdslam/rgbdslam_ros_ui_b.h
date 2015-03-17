#ifndef _ROS_SERVICE_rgbdslam_ros_ui_b_h
#define _ROS_SERVICE_rgbdslam_ros_ui_b_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace rgbdslam
{

static const char RGBDSLAM_ROS_UI_B[] = "rgbdslam/rgbdslam_ros_ui_b";

  class rgbdslam_ros_ui_bRequest : public ros::Msg
  {
    public:
      char * command;
      bool value;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_command = strlen( (const char*) this->command);
      memcpy(outbuffer + offset, &length_command, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->command, length_command);
      offset += length_command;
      union {
        bool real;
        uint8_t base;
      } u_value;
      u_value.real = this->value;
      *(outbuffer + offset + 0) = (u_value.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->value);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_command;
      memcpy(&length_command, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_command; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_command-1]=0;
      this->command = (char *)(inbuffer + offset-1);
      offset += length_command;
      union {
        bool real;
        uint8_t base;
      } u_value;
      u_value.base = 0;
      u_value.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->value = u_value.real;
      offset += sizeof(this->value);
     return offset;
    }

    const char * getType(){ return RGBDSLAM_ROS_UI_B; };
    const char * getMD5(){ return "95aa0151a35e3de365041ffa089ce8c7"; };

  };

  class rgbdslam_ros_ui_bResponse : public ros::Msg
  {
    public:

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return RGBDSLAM_ROS_UI_B; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class rgbdslam_ros_ui_b {
    public:
    typedef rgbdslam_ros_ui_bRequest Request;
    typedef rgbdslam_ros_ui_bResponse Response;
  };

}
#endif
