#ifndef _ROS_wifi_ddwrt_Network_h
#define _ROS_wifi_ddwrt_Network_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace wifi_ddwrt
{

  class Network : public ros::Msg
  {
    public:
      char * macattr;
      char * essid;
      int32_t channel;
      int32_t rssi;
      int32_t noise;
      int32_t beacon;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_macattr = strlen( (const char*) this->macattr);
      memcpy(outbuffer + offset, &length_macattr, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->macattr, length_macattr);
      offset += length_macattr;
      uint32_t length_essid = strlen( (const char*) this->essid);
      memcpy(outbuffer + offset, &length_essid, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->essid, length_essid);
      offset += length_essid;
      union {
        int32_t real;
        uint32_t base;
      } u_channel;
      u_channel.real = this->channel;
      *(outbuffer + offset + 0) = (u_channel.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_channel.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_channel.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_channel.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->channel);
      union {
        int32_t real;
        uint32_t base;
      } u_rssi;
      u_rssi.real = this->rssi;
      *(outbuffer + offset + 0) = (u_rssi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_rssi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_rssi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_rssi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->rssi);
      union {
        int32_t real;
        uint32_t base;
      } u_noise;
      u_noise.real = this->noise;
      *(outbuffer + offset + 0) = (u_noise.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_noise.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_noise.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_noise.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->noise);
      union {
        int32_t real;
        uint32_t base;
      } u_beacon;
      u_beacon.real = this->beacon;
      *(outbuffer + offset + 0) = (u_beacon.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_beacon.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_beacon.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_beacon.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->beacon);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_macattr;
      memcpy(&length_macattr, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_macattr; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_macattr-1]=0;
      this->macattr = (char *)(inbuffer + offset-1);
      offset += length_macattr;
      uint32_t length_essid;
      memcpy(&length_essid, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_essid; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_essid-1]=0;
      this->essid = (char *)(inbuffer + offset-1);
      offset += length_essid;
      union {
        int32_t real;
        uint32_t base;
      } u_channel;
      u_channel.base = 0;
      u_channel.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_channel.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_channel.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_channel.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->channel = u_channel.real;
      offset += sizeof(this->channel);
      union {
        int32_t real;
        uint32_t base;
      } u_rssi;
      u_rssi.base = 0;
      u_rssi.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_rssi.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_rssi.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_rssi.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->rssi = u_rssi.real;
      offset += sizeof(this->rssi);
      union {
        int32_t real;
        uint32_t base;
      } u_noise;
      u_noise.base = 0;
      u_noise.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_noise.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_noise.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_noise.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->noise = u_noise.real;
      offset += sizeof(this->noise);
      union {
        int32_t real;
        uint32_t base;
      } u_beacon;
      u_beacon.base = 0;
      u_beacon.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_beacon.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_beacon.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_beacon.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->beacon = u_beacon.real;
      offset += sizeof(this->beacon);
     return offset;
    }

    const char * getType(){ return "wifi_ddwrt/Network"; };
    const char * getMD5(){ return "b0854419660dc197dd94305843bee07f"; };

  };

}
#endif