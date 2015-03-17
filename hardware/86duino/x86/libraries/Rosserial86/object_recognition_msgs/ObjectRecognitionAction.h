#ifndef _ROS_object_recognition_msgs_ObjectRecognitionAction_h
#define _ROS_object_recognition_msgs_ObjectRecognitionAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "object_recognition_msgs/ObjectRecognitionActionGoal.h"
#include "object_recognition_msgs/ObjectRecognitionActionResult.h"
#include "object_recognition_msgs/ObjectRecognitionActionFeedback.h"

namespace object_recognition_msgs
{

  class ObjectRecognitionAction : public ros::Msg
  {
    public:
      object_recognition_msgs::ObjectRecognitionActionGoal action_goal;
      object_recognition_msgs::ObjectRecognitionActionResult action_result;
      object_recognition_msgs::ObjectRecognitionActionFeedback action_feedback;

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

    const char * getType(){ return "object_recognition_msgs/ObjectRecognitionAction"; };
    const char * getMD5(){ return "7d8979a0cf97e5078553ee3efee047d2"; };

  };

}
#endif