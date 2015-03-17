#ifndef _ROS_concert_msgs_Constants_h
#define _ROS_concert_msgs_Constants_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace concert_msgs
{

  class Constants : public ros::Msg
  {
    public:
      enum { CONCERT_CLIENT_STATUS_AVAILABLE = available };
      enum { CONCERT_CLIENT_STATUS_UNVAILABLE = unavailable };
      enum { CONCERT_CLIENT_STATUS_CONNECTED = connected };
      enum { APP_STATUS_STOPPED = stopped };
      enum { APP_STATUS_RUNNING = running };

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

    const char * getType(){ return "concert_msgs/Constants"; };
    const char * getMD5(){ return "67dff40b2fb0fac383b37c72216f4cfc"; };

  };

}
#endif