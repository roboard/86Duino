#ifndef _ROS_network_monitor_udp_LinktestResult_h
#define _ROS_network_monitor_udp_LinktestResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace network_monitor_udp
{

  class LinktestResult : public ros::Msg
  {
    public:
      float latency;
      float loss;
      float bandwidth;
      uint8_t latency_histogram_length;
      float st_latency_histogram;
      float * latency_histogram;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_latency;
      u_latency.real = this->latency;
      *(outbuffer + offset + 0) = (u_latency.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_latency.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_latency.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_latency.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->latency);
      union {
        float real;
        uint32_t base;
      } u_loss;
      u_loss.real = this->loss;
      *(outbuffer + offset + 0) = (u_loss.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_loss.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_loss.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_loss.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->loss);
      union {
        float real;
        uint32_t base;
      } u_bandwidth;
      u_bandwidth.real = this->bandwidth;
      *(outbuffer + offset + 0) = (u_bandwidth.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_bandwidth.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_bandwidth.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_bandwidth.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->bandwidth);
      *(outbuffer + offset++) = latency_histogram_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < latency_histogram_length; i++){
      union {
        float real;
        uint32_t base;
      } u_latency_histogrami;
      u_latency_histogrami.real = this->latency_histogram[i];
      *(outbuffer + offset + 0) = (u_latency_histogrami.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_latency_histogrami.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_latency_histogrami.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_latency_histogrami.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->latency_histogram[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_latency;
      u_latency.base = 0;
      u_latency.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_latency.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_latency.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_latency.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->latency = u_latency.real;
      offset += sizeof(this->latency);
      union {
        float real;
        uint32_t base;
      } u_loss;
      u_loss.base = 0;
      u_loss.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_loss.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_loss.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_loss.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->loss = u_loss.real;
      offset += sizeof(this->loss);
      union {
        float real;
        uint32_t base;
      } u_bandwidth;
      u_bandwidth.base = 0;
      u_bandwidth.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_bandwidth.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_bandwidth.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_bandwidth.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->bandwidth = u_bandwidth.real;
      offset += sizeof(this->bandwidth);
      uint8_t latency_histogram_lengthT = *(inbuffer + offset++);
      if(latency_histogram_lengthT > latency_histogram_length)
        this->latency_histogram = (float*)realloc(this->latency_histogram, latency_histogram_lengthT * sizeof(float));
      offset += 3;
      latency_histogram_length = latency_histogram_lengthT;
      for( uint8_t i = 0; i < latency_histogram_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_latency_histogram;
      u_st_latency_histogram.base = 0;
      u_st_latency_histogram.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_latency_histogram.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_latency_histogram.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_latency_histogram.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_latency_histogram = u_st_latency_histogram.real;
      offset += sizeof(this->st_latency_histogram);
        memcpy( &(this->latency_histogram[i]), &(this->st_latency_histogram), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "network_monitor_udp/LinktestResult"; };
    const char * getMD5(){ return "cde18aa0ddbb2c88ca3e62eb27f8aa3f"; };

  };

}
#endif