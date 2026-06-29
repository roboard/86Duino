#ifndef _ROS_pr2_plugs_msgs_FetchPlugActionResult_h
#define _ROS_pr2_plugs_msgs_FetchPlugActionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "pr2_plugs_msgs/FetchPlugResult.h"

namespace pr2_plugs_msgs
{

  class FetchPlugActionResult : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      pr2_plugs_msgs::FetchPlugResult result;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->result.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->result.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/FetchPlugActionResult"; };
    const char * getMD5(){ return "42a274f9cc656bdbcbd47fb5ce5c04e2"; };

  };

}
#endif