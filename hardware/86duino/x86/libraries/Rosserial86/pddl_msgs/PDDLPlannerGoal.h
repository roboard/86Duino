#ifndef _ROS_pddl_msgs_PDDLPlannerGoal_h
#define _ROS_pddl_msgs_PDDLPlannerGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pddl_msgs/PDDLDomain.h"
#include "pddl_msgs/PDDLProblem.h"

namespace pddl_msgs
{

  class PDDLPlannerGoal : public ros::Msg
  {
    public:
      pddl_msgs::PDDLDomain domain;
      pddl_msgs::PDDLProblem problem;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->domain.serialize(outbuffer + offset);
      offset += this->problem.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->domain.deserialize(inbuffer + offset);
      offset += this->problem.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pddl_msgs/PDDLPlannerGoal"; };
    const char * getMD5(){ return "1c616a007e540287d36771fae579cf3a"; };

  };

}
#endif