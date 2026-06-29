#ifndef _ROS_pr2_plugs_msgs_DetectPlugInGripperFeedback_h
#define _ROS_pr2_plugs_msgs_DetectPlugInGripperFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_plugs_msgs
{

  class DetectPlugInGripperFeedback : public ros::Msg
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

    const char * getType(){ return "pr2_plugs_msgs/DetectPlugInGripperFeedback"; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

}
#endif