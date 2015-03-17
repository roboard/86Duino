#ifndef _ROS_pddl_msgs_PDDLAction_h
#define _ROS_pddl_msgs_PDDLAction_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pddl_msgs
{

  class PDDLAction : public ros::Msg
  {
    public:
      char * name;
      char * parameters;
      char * precondition;
      char * effect;
      char * on_condition;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_parameters = strlen( (const char*) this->parameters);
      memcpy(outbuffer + offset, &length_parameters, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->parameters, length_parameters);
      offset += length_parameters;
      uint32_t length_precondition = strlen( (const char*) this->precondition);
      memcpy(outbuffer + offset, &length_precondition, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->precondition, length_precondition);
      offset += length_precondition;
      uint32_t length_effect = strlen( (const char*) this->effect);
      memcpy(outbuffer + offset, &length_effect, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->effect, length_effect);
      offset += length_effect;
      uint32_t length_on_condition = strlen( (const char*) this->on_condition);
      memcpy(outbuffer + offset, &length_on_condition, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->on_condition, length_on_condition);
      offset += length_on_condition;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      uint32_t length_parameters;
      memcpy(&length_parameters, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_parameters; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_parameters-1]=0;
      this->parameters = (char *)(inbuffer + offset-1);
      offset += length_parameters;
      uint32_t length_precondition;
      memcpy(&length_precondition, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_precondition; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_precondition-1]=0;
      this->precondition = (char *)(inbuffer + offset-1);
      offset += length_precondition;
      uint32_t length_effect;
      memcpy(&length_effect, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_effect; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_effect-1]=0;
      this->effect = (char *)(inbuffer + offset-1);
      offset += length_effect;
      uint32_t length_on_condition;
      memcpy(&length_on_condition, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_on_condition; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_on_condition-1]=0;
      this->on_condition = (char *)(inbuffer + offset-1);
      offset += length_on_condition;
     return offset;
    }

    const char * getType(){ return "pddl_msgs/PDDLAction"; };
    const char * getMD5(){ return "bfaa943a9c9df1ee5bb56495ada2cdd8"; };

  };

}
#endif