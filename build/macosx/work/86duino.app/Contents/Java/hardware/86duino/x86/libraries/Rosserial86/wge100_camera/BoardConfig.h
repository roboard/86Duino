#ifndef _ROS_SERVICE_BoardConfig_h
#define _ROS_SERVICE_BoardConfig_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace wge100_camera
{

static const char BOARDCONFIG[] = "wge100_camera/BoardConfig";

  class BoardConfigRequest : public ros::Msg
  {
    public:
      uint32_t serial;
      uint8_t mac_length;
      uint8_t st_mac;
      uint8_t * mac;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->serial >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->serial >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->serial >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->serial >> (8 * 3)) & 0xFF;
      offset += sizeof(this->serial);
      *(outbuffer + offset++) = mac_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < mac_length; i++){
      *(outbuffer + offset + 0) = (this->mac[i] >> (8 * 0)) & 0xFF;
      offset += sizeof(this->mac[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->serial =  ((uint32_t) (*(inbuffer + offset)));
      this->serial |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->serial |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->serial |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->serial);
      uint8_t mac_lengthT = *(inbuffer + offset++);
      if(mac_lengthT > mac_length)
        this->mac = (uint8_t*)realloc(this->mac, mac_lengthT * sizeof(uint8_t));
      offset += 3;
      mac_length = mac_lengthT;
      for( uint8_t i = 0; i < mac_length; i++){
      this->st_mac =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->st_mac);
        memcpy( &(this->mac[i]), &(this->st_mac), sizeof(uint8_t));
      }
     return offset;
    }

    const char * getType(){ return BOARDCONFIG; };
    const char * getMD5(){ return "ec9bad54b410ebc79183d761c609dd76"; };

  };

  class BoardConfigResponse : public ros::Msg
  {
    public:
      uint8_t success;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->success >> (8 * 0)) & 0xFF;
      offset += sizeof(this->success);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->success =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->success);
     return offset;
    }

    const char * getType(){ return BOARDCONFIG; };
    const char * getMD5(){ return "6cca7c80398b8b31af04b80534923f16"; };

  };

  class BoardConfig {
    public:
    typedef BoardConfigRequest Request;
    typedef BoardConfigResponse Response;
  };

}
#endif
