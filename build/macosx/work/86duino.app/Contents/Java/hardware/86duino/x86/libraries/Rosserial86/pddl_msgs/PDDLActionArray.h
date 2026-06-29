#ifndef _ROS_pddl_msgs_PDDLActionArray_h
#define _ROS_pddl_msgs_PDDLActionArray_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pddl_msgs/PDDLAction.h"

namespace pddl_msgs
{

  class PDDLActionArray : public ros::Msg
  {
    public:
      uint8_t actions_length;
      pddl_msgs::PDDLAction st_actions;
      pddl_msgs::PDDLAction * actions;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = actions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < actions_length; i++){
      offset += this->actions[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t actions_lengthT = *(inbuffer + offset++);
      if(actions_lengthT > actions_length)
        this->actions = (pddl_msgs::PDDLAction*)realloc(this->actions, actions_lengthT * sizeof(pddl_msgs::PDDLAction));
      offset += 3;
      actions_length = actions_lengthT;
      for( uint8_t i = 0; i < actions_length; i++){
      offset += this->st_actions.deserialize(inbuffer + offset);
        memcpy( &(this->actions[i]), &(this->st_actions), sizeof(pddl_msgs::PDDLAction));
      }
     return offset;
    }

    const char * getType(){ return "pddl_msgs/PDDLActionArray"; };
    const char * getMD5(){ return "477d7d290976c130618806422de7b668"; };

  };

}
#endif