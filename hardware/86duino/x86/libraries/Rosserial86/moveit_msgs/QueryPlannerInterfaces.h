#ifndef _ROS_SERVICE_QueryPlannerInterfaces_h
#define _ROS_SERVICE_QueryPlannerInterfaces_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/PlannerInterfaceDescription.h"

namespace moveit_msgs
{

static const char QUERYPLANNERINTERFACES[] = "moveit_msgs/QueryPlannerInterfaces";

  class QueryPlannerInterfacesRequest : public ros::Msg
  {
    public:

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return QUERYPLANNERINTERFACES; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class QueryPlannerInterfacesResponse : public ros::Msg
  {
    public:
      uint8_t planner_interfaces_length;
      moveit_msgs::PlannerInterfaceDescription st_planner_interfaces;
      moveit_msgs::PlannerInterfaceDescription * planner_interfaces;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = planner_interfaces_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < planner_interfaces_length; i++){
      offset += this->planner_interfaces[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t planner_interfaces_lengthT = *(inbuffer + offset++);
      if(planner_interfaces_lengthT > planner_interfaces_length)
        this->planner_interfaces = (moveit_msgs::PlannerInterfaceDescription*)realloc(this->planner_interfaces, planner_interfaces_lengthT * sizeof(moveit_msgs::PlannerInterfaceDescription));
      offset += 3;
      planner_interfaces_length = planner_interfaces_lengthT;
      for( uint8_t i = 0; i < planner_interfaces_length; i++){
      offset += this->st_planner_interfaces.deserialize(inbuffer + offset);
        memcpy( &(this->planner_interfaces[i]), &(this->st_planner_interfaces), sizeof(moveit_msgs::PlannerInterfaceDescription));
      }
     return offset;
    }

    const char * getType(){ return QUERYPLANNERINTERFACES; };
    const char * getMD5(){ return "acd3317a4c5631f33127fb428209db05"; };

  };

  class QueryPlannerInterfaces {
    public:
    typedef QueryPlannerInterfacesRequest Request;
    typedef QueryPlannerInterfacesResponse Response;
  };

}
#endif
