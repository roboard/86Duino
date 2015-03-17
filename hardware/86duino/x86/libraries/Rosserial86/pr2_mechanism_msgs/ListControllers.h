#ifndef _ROS_SERVICE_ListControllers_h
#define _ROS_SERVICE_ListControllers_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_mechanism_msgs
{

static const char LISTCONTROLLERS[] = "pr2_mechanism_msgs/ListControllers";

  class ListControllersRequest : public ros::Msg
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

    const char * getType(){ return LISTCONTROLLERS; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class ListControllersResponse : public ros::Msg
  {
    public:
      uint8_t controllers_length;
      char* st_controllers;
      char* * controllers;
      uint8_t state_length;
      char* st_state;
      char* * state;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = controllers_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < controllers_length; i++){
      uint32_t length_controllersi = strlen( (const char*) this->controllers[i]);
      memcpy(outbuffer + offset, &length_controllersi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->controllers[i], length_controllersi);
      offset += length_controllersi;
      }
      *(outbuffer + offset++) = state_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < state_length; i++){
      uint32_t length_statei = strlen( (const char*) this->state[i]);
      memcpy(outbuffer + offset, &length_statei, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->state[i], length_statei);
      offset += length_statei;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t controllers_lengthT = *(inbuffer + offset++);
      if(controllers_lengthT > controllers_length)
        this->controllers = (char**)realloc(this->controllers, controllers_lengthT * sizeof(char*));
      offset += 3;
      controllers_length = controllers_lengthT;
      for( uint8_t i = 0; i < controllers_length; i++){
      uint32_t length_st_controllers;
      memcpy(&length_st_controllers, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_controllers; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_controllers-1]=0;
      this->st_controllers = (char *)(inbuffer + offset-1);
      offset += length_st_controllers;
        memcpy( &(this->controllers[i]), &(this->st_controllers), sizeof(char*));
      }
      uint8_t state_lengthT = *(inbuffer + offset++);
      if(state_lengthT > state_length)
        this->state = (char**)realloc(this->state, state_lengthT * sizeof(char*));
      offset += 3;
      state_length = state_lengthT;
      for( uint8_t i = 0; i < state_length; i++){
      uint32_t length_st_state;
      memcpy(&length_st_state, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_state; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_state-1]=0;
      this->st_state = (char *)(inbuffer + offset-1);
      offset += length_st_state;
        memcpy( &(this->state[i]), &(this->st_state), sizeof(char*));
      }
     return offset;
    }

    const char * getType(){ return LISTCONTROLLERS; };
    const char * getMD5(){ return "39c8d39516aed5c7d76284ac06c220e5"; };

  };

  class ListControllers {
    public:
    typedef ListControllersRequest Request;
    typedef ListControllersResponse Response;
  };

}
#endif
