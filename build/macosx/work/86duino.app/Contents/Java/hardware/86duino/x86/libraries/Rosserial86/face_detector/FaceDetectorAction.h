#ifndef _ROS_face_detector_FaceDetectorAction_h
#define _ROS_face_detector_FaceDetectorAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "face_detector/FaceDetectorActionGoal.h"
#include "face_detector/FaceDetectorActionResult.h"
#include "face_detector/FaceDetectorActionFeedback.h"

namespace face_detector
{

  class FaceDetectorAction : public ros::Msg
  {
    public:
      face_detector::FaceDetectorActionGoal action_goal;
      face_detector::FaceDetectorActionResult action_result;
      face_detector::FaceDetectorActionFeedback action_feedback;

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

    const char * getType(){ return "face_detector/FaceDetectorAction"; };
    const char * getMD5(){ return "665c888633df000242196f7098a55805"; };

  };

}
#endif