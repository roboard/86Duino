#ifndef _ROS_SERVICE_GetConstraintAwarePositionIK_h
#define _ROS_SERVICE_GetConstraintAwarePositionIK_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/MoveItErrorCodes.h"
#include "moveit_msgs/PositionIKRequest.h"
#include "moveit_msgs/RobotState.h"
#include "moveit_msgs/Constraints.h"

namespace moveit_msgs
{

static const char GETCONSTRAINTAWAREPOSITIONIK[] = "moveit_msgs/GetConstraintAwarePositionIK";

  class GetConstraintAwarePositionIKRequest : public ros::Msg
  {
    public:
      moveit_msgs::PositionIKRequest ik_request;
      moveit_msgs::Constraints constraints;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->ik_request.serialize(outbuffer + offset);
      offset += this->constraints.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->ik_request.deserialize(inbuffer + offset);
      offset += this->constraints.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETCONSTRAINTAWAREPOSITIONIK; };
    const char * getMD5(){ return "7397fbef2589fd33df21251c96c43f11"; };

  };

  class GetConstraintAwarePositionIKResponse : public ros::Msg
  {
    public:
      moveit_msgs::RobotState solution;
      moveit_msgs::MoveItErrorCodes error_code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->solution.serialize(outbuffer + offset);
      offset += this->error_code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->solution.deserialize(inbuffer + offset);
      offset += this->error_code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETCONSTRAINTAWAREPOSITIONIK; };
    const char * getMD5(){ return "ad50fe5fa0ddb482909be313121ea148"; };

  };

  class GetConstraintAwarePositionIK {
    public:
    typedef GetConstraintAwarePositionIKRequest Request;
    typedef GetConstraintAwarePositionIKResponse Response;
  };

}
#endif
