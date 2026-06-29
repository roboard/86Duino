#ifndef _ROS_pano_ros_PanoCaptureActionFeedback_h
#define _ROS_pano_ros_PanoCaptureActionFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "pano_ros/PanoCaptureFeedback.h"

namespace pano_ros
{

  class PanoCaptureActionFeedback : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      pano_ros::PanoCaptureFeedback feedback;

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

    const char * getType(){ return "pano_ros/PanoCaptureActionFeedback"; };
    const char * getMD5(){ return "1b42fdb4af6b0953df8fd3a166ffc74b"; };

  };

}
#endif