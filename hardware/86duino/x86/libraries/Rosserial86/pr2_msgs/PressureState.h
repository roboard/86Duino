#ifndef _ROS_pr2_msgs_PressureState_h
#define _ROS_pr2_msgs_PressureState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace pr2_msgs
{

  class PressureState : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t l_finger_tip_length;
      int16_t st_l_finger_tip;
      int16_t * l_finger_tip;
      uint8_t r_finger_tip_length;
      int16_t st_r_finger_tip;
      int16_t * r_finger_tip;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = l_finger_tip_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < l_finger_tip_length; i++){
      union {
        int16_t real;
        uint16_t base;
      } u_l_finger_tipi;
      u_l_finger_tipi.real = this->l_finger_tip[i];
      *(outbuffer + offset + 0) = (u_l_finger_tipi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_l_finger_tipi.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->l_finger_tip[i]);
      }
      *(outbuffer + offset++) = r_finger_tip_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < r_finger_tip_length; i++){
      union {
        int16_t real;
        uint16_t base;
      } u_r_finger_tipi;
      u_r_finger_tipi.real = this->r_finger_tip[i];
      *(outbuffer + offset + 0) = (u_r_finger_tipi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_r_finger_tipi.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->r_finger_tip[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t l_finger_tip_lengthT = *(inbuffer + offset++);
      if(l_finger_tip_lengthT > l_finger_tip_length)
        this->l_finger_tip = (int16_t*)realloc(this->l_finger_tip, l_finger_tip_lengthT * sizeof(int16_t));
      offset += 3;
      l_finger_tip_length = l_finger_tip_lengthT;
      for( uint8_t i = 0; i < l_finger_tip_length; i++){
      union {
        int16_t real;
        uint16_t base;
      } u_st_l_finger_tip;
      u_st_l_finger_tip.base = 0;
      u_st_l_finger_tip.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_l_finger_tip.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->st_l_finger_tip = u_st_l_finger_tip.real;
      offset += sizeof(this->st_l_finger_tip);
        memcpy( &(this->l_finger_tip[i]), &(this->st_l_finger_tip), sizeof(int16_t));
      }
      uint8_t r_finger_tip_lengthT = *(inbuffer + offset++);
      if(r_finger_tip_lengthT > r_finger_tip_length)
        this->r_finger_tip = (int16_t*)realloc(this->r_finger_tip, r_finger_tip_lengthT * sizeof(int16_t));
      offset += 3;
      r_finger_tip_length = r_finger_tip_lengthT;
      for( uint8_t i = 0; i < r_finger_tip_length; i++){
      union {
        int16_t real;
        uint16_t base;
      } u_st_r_finger_tip;
      u_st_r_finger_tip.base = 0;
      u_st_r_finger_tip.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_r_finger_tip.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->st_r_finger_tip = u_st_r_finger_tip.real;
      offset += sizeof(this->st_r_finger_tip);
        memcpy( &(this->r_finger_tip[i]), &(this->st_r_finger_tip), sizeof(int16_t));
      }
     return offset;
    }

    const char * getType(){ return "pr2_msgs/PressureState"; };
    const char * getMD5(){ return "756fb3b75fa8884524fd0789a78eb04b"; };

  };

}
#endif