#ifndef _ROS_SERVICE_rgbdslam_ros_ui_h
#define _ROS_SERVICE_rgbdslam_ros_ui_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace rgbdslam
{

static const char RGBDSLAM_ROS_UI[] = "rgbdslam/rgbdslam_ros_ui";

  class rgbdslam_ros_uiRequest : public ros::Msg
  {
    public:
      char * command;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_command = strlen( (const char*) this->command);
      memcpy(outbuffer + offset, &length_command, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->command, length_command);
      offset += length_command;
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
     return offset;
    }

    const char * getType(){ return RGBDSLAM_ROS_UI; };
    const char * getMD5(){ return "cba5e21e920a3a2b7b375cb65b64cdea"; };

  };

  class rgbdslam_ros_uiResponse : public ros::Msg
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

    const char * getType(){ return RGBDSLAM_ROS_UI; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class rgbdslam_ros_ui {
    public:
    typedef rgbdslam_ros_uiRequest Request;
    typedef rgbdslam_ros_uiResponse Response;
  };

}
#endif
