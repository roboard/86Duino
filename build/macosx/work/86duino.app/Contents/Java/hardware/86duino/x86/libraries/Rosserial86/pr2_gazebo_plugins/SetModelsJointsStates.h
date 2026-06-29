#ifndef _ROS_SERVICE_SetModelsJointsStates_h
#define _ROS_SERVICE_SetModelsJointsStates_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_gazebo_plugins/ModelJointsState.h"

namespace pr2_gazebo_plugins
{

static const char SETMODELSJOINTSSTATES[] = "pr2_gazebo_plugins/SetModelsJointsStates";

  class SetModelsJointsStatesRequest : public ros::Msg
  {
    public:
      uint8_t model_names_length;
      char* st_model_names;
      char* * model_names;
      uint8_t model_joints_states_length;
      pr2_gazebo_plugins::ModelJointsState st_model_joints_states;
      pr2_gazebo_plugins::ModelJointsState * model_joints_states;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = model_names_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < model_names_length; i++){
      uint32_t length_model_namesi = strlen( (const char*) this->model_names[i]);
      memcpy(outbuffer + offset, &length_model_namesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->model_names[i], length_model_namesi);
      offset += length_model_namesi;
      }
      *(outbuffer + offset++) = model_joints_states_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < model_joints_states_length; i++){
      offset += this->model_joints_states[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t model_names_lengthT = *(inbuffer + offset++);
      if(model_names_lengthT > model_names_length)
        this->model_names = (char**)realloc(this->model_names, model_names_lengthT * sizeof(char*));
      offset += 3;
      model_names_length = model_names_lengthT;
      for( uint8_t i = 0; i < model_names_length; i++){
      uint32_t length_st_model_names;
      memcpy(&length_st_model_names, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_model_names; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_model_names-1]=0;
      this->st_model_names = (char *)(inbuffer + offset-1);
      offset += length_st_model_names;
        memcpy( &(this->model_names[i]), &(this->st_model_names), sizeof(char*));
      }
      uint8_t model_joints_states_lengthT = *(inbuffer + offset++);
      if(model_joints_states_lengthT > model_joints_states_length)
        this->model_joints_states = (pr2_gazebo_plugins::ModelJointsState*)realloc(this->model_joints_states, model_joints_states_lengthT * sizeof(pr2_gazebo_plugins::ModelJointsState));
      offset += 3;
      model_joints_states_length = model_joints_states_lengthT;
      for( uint8_t i = 0; i < model_joints_states_length; i++){
      offset += this->st_model_joints_states.deserialize(inbuffer + offset);
        memcpy( &(this->model_joints_states[i]), &(this->st_model_joints_states), sizeof(pr2_gazebo_plugins::ModelJointsState));
      }
     return offset;
    }

    const char * getType(){ return SETMODELSJOINTSSTATES; };
    const char * getMD5(){ return "ecf71b483df7b70447575a8231727200"; };

  };

  class SetModelsJointsStatesResponse : public ros::Msg
  {
    public:
      bool success;
      char * status_message;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_success;
      u_success.real = this->success;
      *(outbuffer + offset + 0) = (u_success.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->success);
      uint32_t length_status_message = strlen( (const char*) this->status_message);
      memcpy(outbuffer + offset, &length_status_message, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->status_message, length_status_message);
      offset += length_status_message;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_success;
      u_success.base = 0;
      u_success.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->success = u_success.real;
      offset += sizeof(this->success);
      uint32_t length_status_message;
      memcpy(&length_status_message, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_status_message; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_status_message-1]=0;
      this->status_message = (char *)(inbuffer + offset-1);
      offset += length_status_message;
     return offset;
    }

    const char * getType(){ return SETMODELSJOINTSSTATES; };
    const char * getMD5(){ return "2ec6f3eff0161f4257b808b12bc830c2"; };

  };

  class SetModelsJointsStates {
    public:
    typedef SetModelsJointsStatesRequest Request;
    typedef SetModelsJointsStatesResponse Response;
  };

}
#endif
