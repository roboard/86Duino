#ifndef _ROS_tf2_web_republisher_TFSubscriptionActionGoal_h
#define _ROS_tf2_web_republisher_TFSubscriptionActionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalID.h"
#include "tf2_web_republisher/TFSubscriptionGoal.h"

namespace tf2_web_republisher
{

  class TFSubscriptionActionGoal : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalID goal_id;
      tf2_web_republisher::TFSubscriptionGoal goal;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->goal_id.serialize(outbuffer + offset);
      offset += this->goal.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->goal_id.deserialize(inbuffer + offset);
      offset += this->goal.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "tf2_web_republisher/TFSubscriptionActionGoal"; };
    const char * getMD5(){ return "ef8da891ba3ba9b13d97bca8154eaeb5"; };

  };

}
#endif