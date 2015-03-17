#ifndef _ROS_pr2_image_snapshot_recorder_ImageSnapshotAction_h
#define _ROS_pr2_image_snapshot_recorder_ImageSnapshotAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_image_snapshot_recorder/ImageSnapshotActionGoal.h"
#include "pr2_image_snapshot_recorder/ImageSnapshotActionResult.h"
#include "pr2_image_snapshot_recorder/ImageSnapshotActionFeedback.h"

namespace pr2_image_snapshot_recorder
{

  class ImageSnapshotAction : public ros::Msg
  {
    public:
      pr2_image_snapshot_recorder::ImageSnapshotActionGoal action_goal;
      pr2_image_snapshot_recorder::ImageSnapshotActionResult action_result;
      pr2_image_snapshot_recorder::ImageSnapshotActionFeedback action_feedback;

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

    const char * getType(){ return "pr2_image_snapshot_recorder/ImageSnapshotAction"; };
    const char * getMD5(){ return "249f2e989d34e5520a02bd3409df9473"; };

  };

}
#endif