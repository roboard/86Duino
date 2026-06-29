#ifndef _ROS_razer_hydra_HydraPaddle_h
#define _ROS_razer_hydra_HydraPaddle_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Transform.h"

namespace razer_hydra
{

  class HydraPaddle : public ros::Msg
  {
    public:
      geometry_msgs::Transform transform;
      bool buttons[7];
      float joy[2];
      float trigger;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->transform.serialize(outbuffer + offset);
      unsigned char * buttons_val = (unsigned char *) this->buttons;
      for( uint8_t i = 0; i < 7; i++){
      union {
        bool real;
        uint8_t base;
      } u_buttonsi;
      u_buttonsi.real = this->buttons[i];
      *(outbuffer + offset + 0) = (u_buttonsi.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->buttons[i]);
      }
      unsigned char * joy_val = (unsigned char *) this->joy;
      for( uint8_t i = 0; i < 2; i++){
      union {
        float real;
        uint32_t base;
      } u_joyi;
      u_joyi.real = this->joy[i];
      *(outbuffer + offset + 0) = (u_joyi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_joyi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_joyi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_joyi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->joy[i]);
      }
      union {
        float real;
        uint32_t base;
      } u_trigger;
      u_trigger.real = this->trigger;
      *(outbuffer + offset + 0) = (u_trigger.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_trigger.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_trigger.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_trigger.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->trigger);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->transform.deserialize(inbuffer + offset);
      uint8_t * buttons_val = (uint8_t*) this->buttons;
      for( uint8_t i = 0; i < 7; i++){
      union {
        bool real;
        uint8_t base;
      } u_buttonsi;
      u_buttonsi.base = 0;
      u_buttonsi.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->buttons[i] = u_buttonsi.real;
      offset += sizeof(this->buttons[i]);
      }
      uint8_t * joy_val = (uint8_t*) this->joy;
      for( uint8_t i = 0; i < 2; i++){
      union {
        float real;
        uint32_t base;
      } u_joyi;
      u_joyi.base = 0;
      u_joyi.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_joyi.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_joyi.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_joyi.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->joy[i] = u_joyi.real;
      offset += sizeof(this->joy[i]);
      }
      union {
        float real;
        uint32_t base;
      } u_trigger;
      u_trigger.base = 0;
      u_trigger.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_trigger.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_trigger.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_trigger.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->trigger = u_trigger.real;
      offset += sizeof(this->trigger);
     return offset;
    }

    const char * getType(){ return "razer_hydra/HydraPaddle"; };
    const char * getMD5(){ return "9aa188f8a684b5f86f071eb093833ed5"; };

  };

}
#endif