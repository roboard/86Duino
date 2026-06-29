#ifndef _ROS_network_monitor_udp_UdpSink_h
#define _ROS_network_monitor_udp_UdpSink_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace network_monitor_udp
{

  class UdpSink : public ros::Msg
  {
    public:
      uint8_t magic[4];
      float send_time;
      float echo_time;
      int32_t seqnum;
      int32_t source_id;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      unsigned char * magic_val = (unsigned char *) this->magic;
      for( uint8_t i = 0; i < 4; i++){
      *(outbuffer + offset + 0) = (this->magic[i] >> (8 * 0)) & 0xFF;
      offset += sizeof(this->magic[i]);
      }
      int32_t * val_send_time = (int32_t *) &(this->send_time);
      int32_t exp_send_time = (((*val_send_time)>>23)&255);
      if(exp_send_time != 0)
        exp_send_time += 1023-127;
      int32_t sig_send_time = *val_send_time;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_send_time<<5) & 0xff;
      *(outbuffer + offset++) = (sig_send_time>>3) & 0xff;
      *(outbuffer + offset++) = (sig_send_time>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_send_time<<4) & 0xF0) | ((sig_send_time>>19)&0x0F);
      *(outbuffer + offset++) = (exp_send_time>>4) & 0x7F;
      if(this->send_time < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_echo_time = (int32_t *) &(this->echo_time);
      int32_t exp_echo_time = (((*val_echo_time)>>23)&255);
      if(exp_echo_time != 0)
        exp_echo_time += 1023-127;
      int32_t sig_echo_time = *val_echo_time;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_echo_time<<5) & 0xff;
      *(outbuffer + offset++) = (sig_echo_time>>3) & 0xff;
      *(outbuffer + offset++) = (sig_echo_time>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_echo_time<<4) & 0xF0) | ((sig_echo_time>>19)&0x0F);
      *(outbuffer + offset++) = (exp_echo_time>>4) & 0x7F;
      if(this->echo_time < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        int32_t real;
        uint32_t base;
      } u_seqnum;
      u_seqnum.real = this->seqnum;
      *(outbuffer + offset + 0) = (u_seqnum.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_seqnum.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_seqnum.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_seqnum.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->seqnum);
      union {
        int32_t real;
        uint32_t base;
      } u_source_id;
      u_source_id.real = this->source_id;
      *(outbuffer + offset + 0) = (u_source_id.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_source_id.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_source_id.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_source_id.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->source_id);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t * magic_val = (uint8_t*) this->magic;
      for( uint8_t i = 0; i < 4; i++){
      this->magic[i] =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->magic[i]);
      }
      uint32_t * val_send_time = (uint32_t*) &(this->send_time);
      offset += 3;
      *val_send_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_send_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_send_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_send_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_send_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_send_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_send_time !=0)
        *val_send_time |= ((exp_send_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->send_time = -this->send_time;
      uint32_t * val_echo_time = (uint32_t*) &(this->echo_time);
      offset += 3;
      *val_echo_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_echo_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_echo_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_echo_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_echo_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_echo_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_echo_time !=0)
        *val_echo_time |= ((exp_echo_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->echo_time = -this->echo_time;
      union {
        int32_t real;
        uint32_t base;
      } u_seqnum;
      u_seqnum.base = 0;
      u_seqnum.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_seqnum.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_seqnum.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_seqnum.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->seqnum = u_seqnum.real;
      offset += sizeof(this->seqnum);
      union {
        int32_t real;
        uint32_t base;
      } u_source_id;
      u_source_id.base = 0;
      u_source_id.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_source_id.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_source_id.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_source_id.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->source_id = u_source_id.real;
      offset += sizeof(this->source_id);
     return offset;
    }

    const char * getType(){ return "network_monitor_udp/UdpSink"; };
    const char * getMD5(){ return "a56e2a33942a368e87b357cc9e894ec5"; };

  };

}
#endif