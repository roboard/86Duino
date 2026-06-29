#ifndef _ROS_gateway_msgs_RemoteGateway_h
#define _ROS_gateway_msgs_RemoteGateway_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "gateway_msgs/Rule.h"
#include "gateway_msgs/RemoteRule.h"

namespace gateway_msgs
{

  class RemoteGateway : public ros::Msg
  {
    public:
      char * name;
      char * ip;
      bool firewall;
      uint8_t public_interface_length;
      gateway_msgs::Rule st_public_interface;
      gateway_msgs::Rule * public_interface;
      uint8_t flipped_interface_length;
      gateway_msgs::RemoteRule st_flipped_interface;
      gateway_msgs::RemoteRule * flipped_interface;
      uint8_t pulled_interface_length;
      gateway_msgs::RemoteRule st_pulled_interface;
      gateway_msgs::RemoteRule * pulled_interface;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_ip = strlen( (const char*) this->ip);
      memcpy(outbuffer + offset, &length_ip, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->ip, length_ip);
      offset += length_ip;
      union {
        bool real;
        uint8_t base;
      } u_firewall;
      u_firewall.real = this->firewall;
      *(outbuffer + offset + 0) = (u_firewall.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->firewall);
      *(outbuffer + offset++) = public_interface_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < public_interface_length; i++){
      offset += this->public_interface[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = flipped_interface_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < flipped_interface_length; i++){
      offset += this->flipped_interface[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = pulled_interface_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < pulled_interface_length; i++){
      offset += this->pulled_interface[i].serialize(outbuffer + offset);
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
      uint32_t length_ip;
      memcpy(&length_ip, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_ip; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_ip-1]=0;
      this->ip = (char *)(inbuffer + offset-1);
      offset += length_ip;
      union {
        bool real;
        uint8_t base;
      } u_firewall;
      u_firewall.base = 0;
      u_firewall.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->firewall = u_firewall.real;
      offset += sizeof(this->firewall);
      uint8_t public_interface_lengthT = *(inbuffer + offset++);
      if(public_interface_lengthT > public_interface_length)
        this->public_interface = (gateway_msgs::Rule*)realloc(this->public_interface, public_interface_lengthT * sizeof(gateway_msgs::Rule));
      offset += 3;
      public_interface_length = public_interface_lengthT;
      for( uint8_t i = 0; i < public_interface_length; i++){
      offset += this->st_public_interface.deserialize(inbuffer + offset);
        memcpy( &(this->public_interface[i]), &(this->st_public_interface), sizeof(gateway_msgs::Rule));
      }
      uint8_t flipped_interface_lengthT = *(inbuffer + offset++);
      if(flipped_interface_lengthT > flipped_interface_length)
        this->flipped_interface = (gateway_msgs::RemoteRule*)realloc(this->flipped_interface, flipped_interface_lengthT * sizeof(gateway_msgs::RemoteRule));
      offset += 3;
      flipped_interface_length = flipped_interface_lengthT;
      for( uint8_t i = 0; i < flipped_interface_length; i++){
      offset += this->st_flipped_interface.deserialize(inbuffer + offset);
        memcpy( &(this->flipped_interface[i]), &(this->st_flipped_interface), sizeof(gateway_msgs::RemoteRule));
      }
      uint8_t pulled_interface_lengthT = *(inbuffer + offset++);
      if(pulled_interface_lengthT > pulled_interface_length)
        this->pulled_interface = (gateway_msgs::RemoteRule*)realloc(this->pulled_interface, pulled_interface_lengthT * sizeof(gateway_msgs::RemoteRule));
      offset += 3;
      pulled_interface_length = pulled_interface_lengthT;
      for( uint8_t i = 0; i < pulled_interface_length; i++){
      offset += this->st_pulled_interface.deserialize(inbuffer + offset);
        memcpy( &(this->pulled_interface[i]), &(this->st_pulled_interface), sizeof(gateway_msgs::RemoteRule));
      }
     return offset;
    }

    const char * getType(){ return "gateway_msgs/RemoteGateway"; };
    const char * getMD5(){ return "c2825a110d4bc0a2e94421a89778e5f4"; };

  };

}
#endif