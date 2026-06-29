#ifndef _ROS_tf2_web_republisher_TFSubscriptionActionFeedback_h
#define _ROS_tf2_web_republisher_TFSubscriptionActionFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "tf2_web_republisher/TFSubscriptionFeedback.h"

namespace tf2_web_republisher
{

  class TFSubscriptionActionFeedback : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      tf2_web_republisher::TFSubscriptionFeedback feedback;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->feedback.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->feedback.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "tf2_web_republisher/TFSubscriptionActionFeedback"; };
    const char * getMD5(){ return "de686654be3ef0f8970616dd702bb360"; };

  };

}
#endif