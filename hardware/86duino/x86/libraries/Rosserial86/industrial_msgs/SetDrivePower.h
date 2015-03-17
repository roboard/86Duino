#ifndef _ROS_SERVICE_SetDrivePower_h
#define _ROS_SERVICE_SetDrivePower_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "industrial_msgs/ServiceReturnCode.h"

namespace industrial_msgs
{

static const char SETDRIVEPOWER[] = "industrial_msgs/SetDrivePower";

  class SetDrivePowerRequest : public ros::Msg
  {
    public:
      bool drive_power;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_drive_power;
      u_drive_power.real = this->drive_power;
      *(outbuffer + offset + 0) = (u_drive_power.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->drive_power);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_drive_power;
      u_drive_power.base = 0;
      u_drive_power.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->drive_power = u_drive_power.real;
      offset += sizeof(this->drive_power);
     return offset;
    }

    const char * getType(){ return SETDRIVEPOWER; };
    const char * getMD5(){ return "ad0065fa1febb42851b8c0a0493a1234"; };

  };

  class SetDrivePowerResponse : public ros::Msg
  {
    public:
      industrial_msgs::ServiceReturnCode code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return SETDRIVEPOWER; };
    const char * getMD5(){ return "50b1f38f75f5677e5692f3b3e7e1ea48"; };

  };

  class SetDrivePower {
    public:
    typedef SetDrivePowerRequest Request;
    typedef SetDrivePowerResponse Response;
  };

}
#endif
