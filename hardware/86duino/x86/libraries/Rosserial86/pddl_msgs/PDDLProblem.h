#ifndef _ROS_pddl_msgs_PDDLProblem_h
#define _ROS_pddl_msgs_PDDLProblem_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pddl_msgs/PDDLObject.h"

namespace pddl_msgs
{

  class PDDLProblem : public ros::Msg
  {
    public:
      char * name;
      char * domain;
      uint8_t objects_length;
      pddl_msgs::PDDLObject st_objects;
      pddl_msgs::PDDLObject * objects;
      uint8_t initial_length;
      char* st_initial;
      char* * initial;
      char * goal;
      char * metric;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_domain = strlen( (const char*) this->domain);
      memcpy(outbuffer + offset, &length_domain, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->domain, length_domain);
      offset += length_domain;
      *(outbuffer + offset++) = objects_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < objects_length; i++){
      offset += this->objects[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = initial_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < initial_length; i++){
      uint32_t length_initiali = strlen( (const char*) this->initial[i]);
      memcpy(outbuffer + offset, &length_initiali, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->initial[i], length_initiali);
      offset += length_initiali;
      }
      uint32_t length_goal = strlen( (const char*) this->goal);
      memcpy(outbuffer + offset, &length_goal, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->goal, length_goal);
      offset += length_goal;
      uint32_t length_metric = strlen( (const char*) this->metric);
      memcpy(outbuffer + offset, &length_metric, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->metric, length_metric);
      offset += length_metric;
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
      uint32_t length_domain;
      memcpy(&length_domain, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_domain; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_domain-1]=0;
      this->domain = (char *)(inbuffer + offset-1);
      offset += length_domain;
      uint8_t objects_lengthT = *(inbuffer + offset++);
      if(objects_lengthT > objects_length)
        this->objects = (pddl_msgs::PDDLObject*)realloc(this->objects, objects_lengthT * sizeof(pddl_msgs::PDDLObject));
      offset += 3;
      objects_length = objects_lengthT;
      for( uint8_t i = 0; i < objects_length; i++){
      offset += this->st_objects.deserialize(inbuffer + offset);
        memcpy( &(this->objects[i]), &(this->st_objects), sizeof(pddl_msgs::PDDLObject));
      }
      uint8_t initial_lengthT = *(inbuffer + offset++);
      if(initial_lengthT > initial_length)
        this->initial = (char**)realloc(this->initial, initial_lengthT * sizeof(char*));
      offset += 3;
      initial_length = initial_lengthT;
      for( uint8_t i = 0; i < initial_length; i++){
      uint32_t length_st_initial;
      memcpy(&length_st_initial, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_initial; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_initial-1]=0;
      this->st_initial = (char *)(inbuffer + offset-1);
      offset += length_st_initial;
        memcpy( &(this->initial[i]), &(this->st_initial), sizeof(char*));
      }
      uint32_t length_goal;
      memcpy(&length_goal, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_goal; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_goal-1]=0;
      this->goal = (char *)(inbuffer + offset-1);
      offset += length_goal;
      uint32_t length_metric;
      memcpy(&length_metric, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_metric; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_metric-1]=0;
      this->metric = (char *)(inbuffer + offset-1);
      offset += length_metric;
     return offset;
    }

    const char * getType(){ return "pddl_msgs/PDDLProblem"; };
    const char * getMD5(){ return "dfd9bdd094d91245128f960694763882"; };

  };

}
#endif