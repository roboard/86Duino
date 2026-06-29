#ifndef _ROS_ur_msgs_IOStates_h
#define _ROS_ur_msgs_IOStates_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ur_msgs/Digital.h"
#include "ur_msgs/Analog.h"

namespace ur_msgs
{

  class IOStates : public ros::Msg
  {
    public:
      uint8_t digital_in_states_length;
      ur_msgs::Digital st_digital_in_states;
      ur_msgs::Digital * digital_in_states;
      uint8_t digital_out_states_length;
      ur_msgs::Digital st_digital_out_states;
      ur_msgs::Digital * digital_out_states;
      uint8_t flag_states_length;
      ur_msgs::Digital st_flag_states;
      ur_msgs::Digital * flag_states;
      uint8_t analog_in_states_length;
      ur_msgs::Analog st_analog_in_states;
      ur_msgs::Analog * analog_in_states;
      uint8_t analog_out_states_length;
      ur_msgs::Analog st_analog_out_states;
      ur_msgs::Analog * analog_out_states;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = digital_in_states_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < digital_in_states_length; i++){
      offset += this->digital_in_states[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = digital_out_states_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < digital_out_states_length; i++){
      offset += this->digital_out_states[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = flag_states_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < flag_states_length; i++){
      offset += this->flag_states[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = analog_in_states_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < analog_in_states_length; i++){
      offset += this->analog_in_states[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = analog_out_states_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < analog_out_states_length; i++){
      offset += this->analog_out_states[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t digital_in_states_lengthT = *(inbuffer + offset++);
      if(digital_in_states_lengthT > digital_in_states_length)
        this->digital_in_states = (ur_msgs::Digital*)realloc(this->digital_in_states, digital_in_states_lengthT * sizeof(ur_msgs::Digital));
      offset += 3;
      digital_in_states_length = digital_in_states_lengthT;
      for( uint8_t i = 0; i < digital_in_states_length; i++){
      offset += this->st_digital_in_states.deserialize(inbuffer + offset);
        memcpy( &(this->digital_in_states[i]), &(this->st_digital_in_states), sizeof(ur_msgs::Digital));
      }
      uint8_t digital_out_states_lengthT = *(inbuffer + offset++);
      if(digital_out_states_lengthT > digital_out_states_length)
        this->digital_out_states = (ur_msgs::Digital*)realloc(this->digital_out_states, digital_out_states_lengthT * sizeof(ur_msgs::Digital));
      offset += 3;
      digital_out_states_length = digital_out_states_lengthT;
      for( uint8_t i = 0; i < digital_out_states_length; i++){
      offset += this->st_digital_out_states.deserialize(inbuffer + offset);
        memcpy( &(this->digital_out_states[i]), &(this->st_digital_out_states), sizeof(ur_msgs::Digital));
      }
      uint8_t flag_states_lengthT = *(inbuffer + offset++);
      if(flag_states_lengthT > flag_states_length)
        this->flag_states = (ur_msgs::Digital*)realloc(this->flag_states, flag_states_lengthT * sizeof(ur_msgs::Digital));
      offset += 3;
      flag_states_length = flag_states_lengthT;
      for( uint8_t i = 0; i < flag_states_length; i++){
      offset += this->st_flag_states.deserialize(inbuffer + offset);
        memcpy( &(this->flag_states[i]), &(this->st_flag_states), sizeof(ur_msgs::Digital));
      }
      uint8_t analog_in_states_lengthT = *(inbuffer + offset++);
      if(analog_in_states_lengthT > analog_in_states_length)
        this->analog_in_states = (ur_msgs::Analog*)realloc(this->analog_in_states, analog_in_states_lengthT * sizeof(ur_msgs::Analog));
      offset += 3;
      analog_in_states_length = analog_in_states_lengthT;
      for( uint8_t i = 0; i < analog_in_states_length; i++){
      offset += this->st_analog_in_states.deserialize(inbuffer + offset);
        memcpy( &(this->analog_in_states[i]), &(this->st_analog_in_states), sizeof(ur_msgs::Analog));
      }
      uint8_t analog_out_states_lengthT = *(inbuffer + offset++);
      if(analog_out_states_lengthT > analog_out_states_length)
        this->analog_out_states = (ur_msgs::Analog*)realloc(this->analog_out_states, analog_out_states_lengthT * sizeof(ur_msgs::Analog));
      offset += 3;
      analog_out_states_length = analog_out_states_lengthT;
      for( uint8_t i = 0; i < analog_out_states_length; i++){
      offset += this->st_analog_out_states.deserialize(inbuffer + offset);
        memcpy( &(this->analog_out_states[i]), &(this->st_analog_out_states), sizeof(ur_msgs::Analog));
      }
     return offset;
    }

    const char * getType(){ return "ur_msgs/IOStates"; };
    const char * getMD5(){ return "0a5c7b73e3189e9a2caf8583d1bae2e2"; };

  };

}
#endif