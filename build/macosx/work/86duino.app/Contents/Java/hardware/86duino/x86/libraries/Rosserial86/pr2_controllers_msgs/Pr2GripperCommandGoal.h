#ifndef _ROS_pr2_controllers_msgs_Pr2GripperCommandGoal_h
#define _ROS_pr2_controllers_msgs_Pr2GripperCommandGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_controllers_msgs/Pr2GripperCommand.h"

namespace pr2_controllers_msgs
{

  class Pr2GripperCommandGoal : public ros::Msg
  {
    public:
      pr2_controllers_msgs::Pr2GripperCommand command;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->command.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->command.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_controllers_msgs/Pr2GripperCommandGoal"; };
    const char * getMD5(){ return "86fd82f4ddc48a4cb6856cfa69217e43"; };

  };

}
#endif