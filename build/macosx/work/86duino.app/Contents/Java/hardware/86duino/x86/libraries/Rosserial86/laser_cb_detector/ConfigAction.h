#ifndef _ROS_laser_cb_detector_ConfigAction_h
#define _ROS_laser_cb_detector_ConfigAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "laser_cb_detector/ConfigActionGoal.h"
#include "laser_cb_detector/ConfigActionResult.h"
#include "laser_cb_detector/ConfigActionFeedback.h"

namespace laser_cb_detector
{

  class ConfigAction : public ros::Msg
  {
    public:
      laser_cb_detector::ConfigActionGoal action_goal;
      laser_cb_detector::ConfigActionResult action_result;
      laser_cb_detector::ConfigActionFeedback action_feedback;

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

    const char * getType(){ return "laser_cb_detector/ConfigAction"; };
    const char * getMD5(){ return "9ab17a2bdd4d1b9a55316f3c57556453"; };

  };

}
#endif