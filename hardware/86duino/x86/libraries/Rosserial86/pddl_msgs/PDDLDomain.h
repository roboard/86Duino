#ifndef _ROS_pddl_msgs_PDDLDomain_h
#define _ROS_pddl_msgs_PDDLDomain_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pddl_msgs/PDDLAction.h"

namespace pddl_msgs
{

  class PDDLDomain : public ros::Msg
  {
    public:
      char * name;
      char * requirements;
      uint8_t types_length;
      char* st_types;
      char* * types;
      uint8_t constants_length;
      char* st_constants;
      char* * constants;
      uint8_t predicates_length;
      char* st_predicates;
      char* * predicates;
      uint8_t actions_length;
      pddl_msgs::PDDLAction st_actions;
      pddl_msgs::PDDLAction * actions;
      uint8_t functions_length;
      char* st_functions;
      char* * functions;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_requirements = strlen( (const char*) this->requirements);
      memcpy(outbuffer + offset, &length_requirements, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->requirements, length_requirements);
      offset += length_requirements;
      *(outbuffer + offset++) = types_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < types_length; i++){
      uint32_t length_typesi = strlen( (const char*) this->types[i]);
      memcpy(outbuffer + offset, &length_typesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->types[i], length_typesi);
      offset += length_typesi;
      }
      *(outbuffer + offset++) = constants_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < constants_length; i++){
      uint32_t length_constantsi = strlen( (const char*) this->constants[i]);
      memcpy(outbuffer + offset, &length_constantsi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->constants[i], length_constantsi);
      offset += length_constantsi;
      }
      *(outbuffer + offset++) = predicates_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < predicates_length; i++){
      uint32_t length_predicatesi = strlen( (const char*) this->predicates[i]);
      memcpy(outbuffer + offset, &length_predicatesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->predicates[i], length_predicatesi);
      offset += length_predicatesi;
      }
      *(outbuffer + offset++) = actions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < actions_length; i++){
      offset += this->actions[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = functions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < functions_length; i++){
      uint32_t length_functionsi = strlen( (const char*) this->functions[i]);
      memcpy(outbuffer + offset, &length_functionsi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->functions[i], length_functionsi);
      offset += length_functionsi;
      }
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
      uint32_t length_requirements;
      memcpy(&length_requirements, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_requirements; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_requirements-1]=0;
      this->requirements = (char *)(inbuffer + offset-1);
      offset += length_requirements;
      uint8_t types_lengthT = *(inbuffer + offset++);
      if(types_lengthT > types_length)
        this->types = (char**)realloc(this->types, types_lengthT * sizeof(char*));
      offset += 3;
      types_length = types_lengthT;
      for( uint8_t i = 0; i < types_length; i++){
      uint32_t length_st_types;
      memcpy(&length_st_types, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_types; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_types-1]=0;
      this->st_types = (char *)(inbuffer + offset-1);
      offset += length_st_types;
        memcpy( &(this->types[i]), &(this->st_types), sizeof(char*));
      }
      uint8_t constants_lengthT = *(inbuffer + offset++);
      if(constants_lengthT > constants_length)
        this->constants = (char**)realloc(this->constants, constants_lengthT * sizeof(char*));
      offset += 3;
      constants_length = constants_lengthT;
      for( uint8_t i = 0; i < constants_length; i++){
      uint32_t length_st_constants;
      memcpy(&length_st_constants, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_constants; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_constants-1]=0;
      this->st_constants = (char *)(inbuffer + offset-1);
      offset += length_st_constants;
        memcpy( &(this->constants[i]), &(this->st_constants), sizeof(char*));
      }
      uint8_t predicates_lengthT = *(inbuffer + offset++);
      if(predicates_lengthT > predicates_length)
        this->predicates = (char**)realloc(this->predicates, predicates_lengthT * sizeof(char*));
      offset += 3;
      predicates_length = predicates_lengthT;
      for( uint8_t i = 0; i < predicates_length; i++){
      uint32_t length_st_predicates;
      memcpy(&length_st_predicates, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_predicates; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_predicates-1]=0;
      this->st_predicates = (char *)(inbuffer + offset-1);
      offset += length_st_predicates;
        memcpy( &(this->predicates[i]), &(this->st_predicates), sizeof(char*));
      }
      uint8_t actions_lengthT = *(inbuffer + offset++);
      if(actions_lengthT > actions_length)
        this->actions = (pddl_msgs::PDDLAction*)realloc(this->actions, actions_lengthT * sizeof(pddl_msgs::PDDLAction));
      offset += 3;
      actions_length = actions_lengthT;
      for( uint8_t i = 0; i < actions_length; i++){
      offset += this->st_actions.deserialize(inbuffer + offset);
        memcpy( &(this->actions[i]), &(this->st_actions), sizeof(pddl_msgs::PDDLAction));
      }
      uint8_t functions_lengthT = *(inbuffer + offset++);
      if(functions_lengthT > functions_length)
        this->functions = (char**)realloc(this->functions, functions_lengthT * sizeof(char*));
      offset += 3;
      functions_length = functions_lengthT;
      for( uint8_t i = 0; i < functions_length; i++){
      uint32_t length_st_functions;
      memcpy(&length_st_functions, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_functions; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_functions-1]=0;
      this->st_functions = (char *)(inbuffer + offset-1);
      offset += length_st_functions;
        memcpy( &(this->functions[i]), &(this->st_functions), sizeof(char*));
      }
     return offset;
    }

    const char * getType(){ return "pddl_msgs/PDDLDomain"; };
    const char * getMD5(){ return "efae1c485597d25a68729e978f828ed8"; };

  };

}
#endif