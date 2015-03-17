#ifndef _ROS_object_recognition_msgs_ObjectRecognitionActionResult_h
#define _ROS_object_recognition_msgs_ObjectRecognitionActionResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "actionlib_msgs/GoalStatus.h"
#include "object_recognition_msgs/ObjectRecognitionResult.h"

namespace object_recognition_msgs
{

  class ObjectRecognitionActionResult : public ros::Msg
  {
    public:
      std_msgs::Header header;
      actionlib_msgs::GoalStatus status;
      object_recognition_msgs::ObjectRecognitionResult result;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->status.serialize(outbuffer + offset);
      offset += this->result.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->status.deserialize(inbuffer + offset);
      offset += this->result.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "object_recognition_msgs/ObjectRecognitionActionResult"; };
    const char * getMD5(){ return "1ef766aeca50bc1bb70773fc73d4471d"; };

  };

}
#endif