#ifndef _ROS_image_cb_detector_ConfigAction_h
#define _ROS_image_cb_detector_ConfigAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "image_cb_detector/ConfigActionGoal.h"
#include "image_cb_detector/ConfigActionResult.h"
#include "image_cb_detector/ConfigActionFeedback.h"

namespace image_cb_detector
{

  class ConfigAction : public ros::Msg
  {
    public:
      image_cb_detector::ConfigActionGoal action_goal;
      image_cb_detector::ConfigActionResult action_result;
      image_cb_detector::ConfigActionFeedback action_feedback;

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

    const char * getType(){ return "image_cb_detector/ConfigAction"; };
    const char * getMD5(){ return "1ddd846910edb1716470805de600567f"; };

  };

}
#endif