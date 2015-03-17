#ifndef _ROS_pr2_plugs_msgs_RechargeResult_h
#define _ROS_pr2_plugs_msgs_RechargeResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/RechargeState.h"

namespace pr2_plugs_msgs
{

  class RechargeResult : public ros::Msg
  {
    public:
      pr2_plugs_msgs::RechargeState state;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->state.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->state.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/RechargeResult"; };
    const char * getMD5(){ return "65acf8e8352dfdb03ae68f2a332c97c7"; };

  };

}
#endif