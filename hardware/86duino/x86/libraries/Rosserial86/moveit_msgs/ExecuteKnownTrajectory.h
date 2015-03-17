#ifndef _ROS_SERVICE_ExecuteKnownTrajectory_h
#define _ROS_SERVICE_ExecuteKnownTrajectory_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/MoveItErrorCodes.h"
#include "moveit_msgs/RobotTrajectory.h"

namespace moveit_msgs
{

static const char EXECUTEKNOWNTRAJECTORY[] = "moveit_msgs/ExecuteKnownTrajectory";

  class ExecuteKnownTrajectoryRequest : public ros::Msg
  {
    public:
      moveit_msgs::RobotTrajectory trajectory;
      bool wait_for_execution;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->trajectory.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_wait_for_execution;
      u_wait_for_execution.real = this->wait_for_execution;
      *(outbuffer + offset + 0) = (u_wait_for_execution.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->wait_for_execution);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->trajectory.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_wait_for_execution;
      u_wait_for_execution.base = 0;
      u_wait_for_execution.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->wait_for_execution = u_wait_for_execution.real;
      offset += sizeof(this->wait_for_execution);
     return offset;
    }

    const char * getType(){ return EXECUTEKNOWNTRAJECTORY; };
    const char * getMD5(){ return "2a3d2a0b337c6a27da4468bb351928e5"; };

  };

  class ExecuteKnownTrajectoryResponse : public ros::Msg
  {
    public:
      moveit_msgs::MoveItErrorCodes error_code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->error_code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->error_code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return EXECUTEKNOWNTRAJECTORY; };
    const char * getMD5(){ return "1f7ab918f5d0c5312f25263d3d688122"; };

  };

  class ExecuteKnownTrajectory {
    public:
    typedef ExecuteKnownTrajectoryRequest Request;
    typedef ExecuteKnownTrajectoryResponse Response;
  };

}
#endif
