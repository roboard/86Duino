#ifndef _ROS_pano_ros_PanoCaptureAction_h
#define _ROS_pano_ros_PanoCaptureAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pano_ros/PanoCaptureActionGoal.h"
#include "pano_ros/PanoCaptureActionResult.h"
#include "pano_ros/PanoCaptureActionFeedback.h"

namespace pano_ros
{

  class PanoCaptureAction : public ros::Msg
  {
    public:
      pano_ros::PanoCaptureActionGoal action_goal;
      pano_ros::PanoCaptureActionResult action_result;
      pano_ros::PanoCaptureActionFeedback action_feedback;

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

    const char * getType(){ return "pano_ros/PanoCaptureAction"; };
    const char * getMD5(){ return "29aaf854ae11bc4f1d21580cf870cfb6"; };

  };

}
#endif