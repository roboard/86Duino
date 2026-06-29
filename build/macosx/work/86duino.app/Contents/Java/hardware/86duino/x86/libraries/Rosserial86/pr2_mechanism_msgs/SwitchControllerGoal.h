#ifndef _ROS_pr2_mechanism_msgs_SwitchControllerGoal_h
#define _ROS_pr2_mechanism_msgs_SwitchControllerGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_mechanism_msgs
{

  class SwitchControllerGoal : public ros::Msg
  {
    public:
      uint8_t start_controllers_length;
      char* st_start_controllers;
      char* * start_controllers;
      uint8_t stop_controllers_length;
      char* st_stop_controllers;
      char* * stop_controllers;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = start_controllers_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < start_controllers_length; i++){
      uint32_t length_start_controllersi = strlen( (const char*) this->start_controllers[i]);
      memcpy(outbuffer + offset, &length_start_controllersi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->start_controllers[i], length_start_controllersi);
      offset += length_start_controllersi;
      }
      *(outbuffer + offset++) = stop_controllers_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < stop_controllers_length; i++){
      uint32_t length_stop_controllersi = strlen( (const char*) this->stop_controllers[i]);
      memcpy(outbuffer + offset, &length_stop_controllersi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->stop_controllers[i], length_stop_controllersi);
      offset += length_stop_controllersi;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t start_controllers_lengthT = *(inbuffer + offset++);
      if(start_controllers_lengthT > start_controllers_length)
        this->start_controllers = (char**)realloc(this->start_controllers, start_controllers_lengthT * sizeof(char*));
      offset += 3;
      start_controllers_length = start_controllers_lengthT;
      for( uint8_t i = 0; i < start_controllers_length; i++){
      uint32_t length_st_start_controllers;
      memcpy(&length_st_start_controllers, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_start_controllers; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_start_controllers-1]=0;
      this->st_start_controllers = (char *)(inbuffer + offset-1);
      offset += length_st_start_controllers;
        memcpy( &(this->start_controllers[i]), &(this->st_start_controllers), sizeof(char*));
      }
      uint8_t stop_controllers_lengthT = *(inbuffer + offset++);
      if(stop_controllers_lengthT > stop_controllers_length)
        this->stop_controllers = (char**)realloc(this->stop_controllers, stop_controllers_lengthT * sizeof(char*));
      offset += 3;
      stop_controllers_length = stop_controllers_lengthT;
      for( uint8_t i = 0; i < stop_controllers_length; i++){
      uint32_t length_st_stop_controllers;
      memcpy(&length_st_stop_controllers, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_stop_controllers; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_stop_controllers-1]=0;
      this->st_stop_controllers = (char *)(inbuffer + offset-1);
      offset += length_st_stop_controllers;
        memcpy( &(this->stop_controllers[i]), &(this->st_stop_controllers), sizeof(char*));
      }
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_msgs/SwitchControllerGoal"; };
    const char * getMD5(){ return "c3f1879cbb2d6cd8732c0e031574dde5"; };

  };

}
#endif