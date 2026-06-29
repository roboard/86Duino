#ifndef _ROS_pr2_msgs_AccessPoint_h
#define _ROS_pr2_msgs_AccessPoint_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace pr2_msgs
{

  class AccessPoint : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * essid;
      char * macaddr;
      int32_t signal;
      int32_t noise;
      int32_t snr;
      int32_t channel;
      char * rate;
      char * tx_power;
      int32_t quality;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_essid = strlen( (const char*) this->essid);
      memcpy(outbuffer + offset, &length_essid, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->essid, length_essid);
      offset += length_essid;
      uint32_t length_macaddr = strlen( (const char*) this->macaddr);
      memcpy(outbuffer + offset, &length_macaddr, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->macaddr, length_macaddr);
      offset += length_macaddr;
      union {
        int32_t real;
        uint32_t base;
      } u_signal;
      u_signal.real = this->signal;
      *(outbuffer + offset + 0) = (u_signal.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_signal.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_signal.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_signal.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->signal);
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
      } u_snr;
      u_snr.real = this->snr;
      *(outbuffer + offset + 0) = (u_snr.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_snr.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_snr.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_snr.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->snr);
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
      uint32_t length_rate = strlen( (const char*) this->rate);
      memcpy(outbuffer + offset, &length_rate, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->rate, length_rate);
      offset += length_rate;
      uint32_t length_tx_power = strlen( (const char*) this->tx_power);
      memcpy(outbuffer + offset, &length_tx_power, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->tx_power, length_tx_power);
      offset += length_tx_power;
      union {
        int32_t real;
        uint32_t base;
      } u_quality;
      u_quality.real = this->quality;
      *(outbuffer + offset + 0) = (u_quality.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_quality.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_quality.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_quality.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->quality);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_essid;
      memcpy(&length_essid, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_essid; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_essid-1]=0;
      this->essid = (char *)(inbuffer + offset-1);
      offset += length_essid;
      uint32_t length_macaddr;
      memcpy(&length_macaddr, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_macaddr; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_macaddr-1]=0;
      this->macaddr = (char *)(inbuffer + offset-1);
      offset += length_macaddr;
      union {
        int32_t real;
        uint32_t base;
      } u_signal;
      u_signal.base = 0;
      u_signal.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_signal.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_signal.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_signal.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->signal = u_signal.real;
      offset += sizeof(this->signal);
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
      } u_snr;
      u_snr.base = 0;
      u_snr.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_snr.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_snr.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_snr.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->snr = u_snr.real;
      offset += sizeof(this->snr);
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
      uint32_t length_rate;
      memcpy(&length_rate, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_rate; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_rate-1]=0;
      this->rate = (char *)(inbuffer + offset-1);
      offset += length_rate;
      uint32_t length_tx_power;
      memcpy(&length_tx_power, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_tx_power; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_tx_power-1]=0;
      this->tx_power = (char *)(inbuffer + offset-1);
      offset += length_tx_power;
      union {
        int32_t real;
        uint32_t base;
      } u_quality;
      u_quality.base = 0;
      u_quality.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_quality.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_quality.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_quality.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->quality = u_quality.real;
      offset += sizeof(this->quality);
     return offset;
    }

    const char * getType(){ return "pr2_msgs/AccessPoint"; };
    const char * getMD5(){ return "810217d9e35df31ffb396ea5673d7d1b"; };

  };

}
#endif