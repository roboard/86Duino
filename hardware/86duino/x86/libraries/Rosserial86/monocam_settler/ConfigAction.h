#ifndef _ROS_monocam_settler_ConfigAction_h
#define _ROS_monocam_settler_ConfigAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "monocam_settler/ConfigActionGoal.h"
#include "monocam_settler/ConfigActionResult.h"
#include "monocam_settler/ConfigActionFeedback.h"

namespace monocam_settler
{

  class ConfigAction : public ros::Msg
  {
    public:
      monocam_settler::ConfigActionGoal action_goal;
      monocam_settler::ConfigActionResult action_result;
      monocam_settler::ConfigActionFeedback action_feedback;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->action_goal.serialize(outbuffer + offset);
      offset += this->action_result.serialize(outbuffer + offset);
      offset += this->action_feedback.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->action_goal.deserialize(inbuffer + offset);
      offset += this->action_result.deserialize(inbuffer + offset);
      offset += this->action_feedback.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "monocam_settler/ConfigAction"; };
    const char * getMD5(){ return "4a593f02b0920fcc9e7be7341dddd78f"; };

  };

}
#endif