#ifndef _ROS_rocon_app_manager_msgs_ErrorCodes_h
#define _ROS_rocon_app_manager_msgs_ErrorCodes_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace rocon_app_manager_msgs
{

  class ErrorCodes : public ros::Msg
  {
    public:
      enum { SUCCESS =  0 };
      enum { MULTI_RAPP_NOT_SUPPORTED =  10 };
      enum { RAPP_IS_NOT_RUNNING =  20 };

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

    const char * getType(){ return "rocon_app_manager_msgs/ErrorCodes"; };
    const char * getMD5(){ return "d9c7f8c09a64096ffbbc33db313b966c"; };

  };

}
#endif