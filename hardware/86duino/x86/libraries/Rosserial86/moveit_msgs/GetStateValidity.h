#ifndef _ROS_SERVICE_GetStateValidity_h
#define _ROS_SERVICE_GetStateValidity_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/ConstraintEvalResult.h"
#include "moveit_msgs/CostSource.h"
#include "moveit_msgs/ContactInformation.h"
#include "moveit_msgs/RobotState.h"
#include "moveit_msgs/Constraints.h"

namespace moveit_msgs
{

static const char GETSTATEVALIDITY[] = "moveit_msgs/GetStateValidity";

  class GetStateValidityRequest : public ros::Msg
  {
    public:
      moveit_msgs::RobotState robot_state;
      char * group_name;
      moveit_msgs::Constraints constraints;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->robot_state.serialize(outbuffer + offset);
      uint32_t length_group_name = strlen( (const char*) this->group_name);
      memcpy(outbuffer + offset, &length_group_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->group_name, length_group_name);
      offset += length_group_name;
      offset += this->constraints.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->robot_state.deserialize(inbuffer + offset);
      uint32_t length_group_name;
      memcpy(&length_group_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_group_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_group_name-1]=0;
      this->group_name = (char *)(inbuffer + offset-1);
      offset += length_group_name;
      offset += this->constraints.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETSTATEVALIDITY; };
    const char * getMD5(){ return "b569c609cafad20ba7d0e46e70e7cab1"; };

  };

  class GetStateValidityResponse : public ros::Msg
  {
    public:
      bool valid;
      uint8_t contacts_length;
      moveit_msgs::ContactInformation st_contacts;
      moveit_msgs::ContactInformation * contacts;
      uint8_t cost_sources_length;
      moveit_msgs::CostSource st_cost_sources;
      moveit_msgs::CostSource * cost_sources;
      uint8_t constraint_result_length;
      moveit_msgs::ConstraintEvalResult st_constraint_result;
      moveit_msgs::ConstraintEvalResult * constraint_result;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_valid;
      u_valid.real = this->valid;
      *(outbuffer + offset + 0) = (u_valid.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->valid);
      *(outbuffer + offset++) = contacts_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < contacts_length; i++){
      offset += this->contacts[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = cost_sources_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < cost_sources_length; i++){
      offset += this->cost_sources[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = constraint_result_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < constraint_result_length; i++){
      offset += this->constraint_result[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_valid;
      u_valid.base = 0;
      u_valid.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->valid = u_valid.real;
      offset += sizeof(this->valid);
      uint8_t contacts_lengthT = *(inbuffer + offset++);
      if(contacts_lengthT > contacts_length)
        this->contacts = (moveit_msgs::ContactInformation*)realloc(this->contacts, contacts_lengthT * sizeof(moveit_msgs::ContactInformation));
      offset += 3;
      contacts_length = contacts_lengthT;
      for( uint8_t i = 0; i < contacts_length; i++){
      offset += this->st_contacts.deserialize(inbuffer + offset);
        memcpy( &(this->contacts[i]), &(this->st_contacts), sizeof(moveit_msgs::ContactInformation));
      }
      uint8_t cost_sources_lengthT = *(inbuffer + offset++);
      if(cost_sources_lengthT > cost_sources_length)
        this->cost_sources = (moveit_msgs::CostSource*)realloc(this->cost_sources, cost_sources_lengthT * sizeof(moveit_msgs::CostSource));
      offset += 3;
      cost_sources_length = cost_sources_lengthT;
      for( uint8_t i = 0; i < cost_sources_length; i++){
      offset += this->st_cost_sources.deserialize(inbuffer + offset);
        memcpy( &(this->cost_sources[i]), &(this->st_cost_sources), sizeof(moveit_msgs::CostSource));
      }
      uint8_t constraint_result_lengthT = *(inbuffer + offset++);
      if(constraint_result_lengthT > constraint_result_length)
        this->constraint_result = (moveit_msgs::ConstraintEvalResult*)realloc(this->constraint_result, constraint_result_lengthT * sizeof(moveit_msgs::ConstraintEvalResult));
      offset += 3;
      constraint_result_length = constraint_result_lengthT;
      for( uint8_t i = 0; i < constraint_result_length; i++){
      offset += this->st_constraint_result.deserialize(inbuffer + offset);
        memcpy( &(this->constraint_result[i]), &(this->st_constraint_result), sizeof(moveit_msgs::ConstraintEvalResult));
      }
     return offset;
    }

    const char * getType(){ return GETSTATEVALIDITY; };
    const char * getMD5(){ return "e326fb22b7448f29c0e726d9270d9929"; };

  };

  class GetStateValidity {
    public:
    typedef GetStateValidityRequest Request;
    typedef GetStateValidityResponse Response;
  };

}
#endif
