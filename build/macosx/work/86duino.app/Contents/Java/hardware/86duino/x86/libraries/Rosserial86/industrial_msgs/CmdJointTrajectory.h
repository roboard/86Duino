#ifndef _ROS_SERVICE_CmdJointTrajectory_h
#define _ROS_SERVICE_CmdJointTrajectory_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "trajectory_msgs/JointTrajectory.h"
#include "industrial_msgs/ServiceReturnCode.h"

namespace industrial_msgs
{

static const char CMDJOINTTRAJECTORY[] = "industrial_msgs/CmdJointTrajectory";

  class CmdJointTrajectoryRequest : public ros::Msg
  {
    public:
      trajectory_msgs::JointTrajectory trajectory;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->trajectory.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->trajectory.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return CMDJOINTTRAJECTORY; };
    const char * getMD5(){ return "2a0eff76c870e8595636c2a562ca298e"; };

  };

  class CmdJointTrajectoryResponse : public ros::Msg
  {
    public:
      industrial_msgs::ServiceReturnCode code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return CMDJOINTTRAJECTORY; };
    const char * getMD5(){ return "50b1f38f75f5677e5692f3b3e7e1ea48"; };

  };

  class CmdJointTrajectory {
    public:
    typedef CmdJointTrajectoryRequest Request;
    typedef CmdJointTrajectoryResponse Response;
  };

}
#endif
