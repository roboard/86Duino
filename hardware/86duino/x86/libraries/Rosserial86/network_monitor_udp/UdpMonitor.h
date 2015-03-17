#ifndef _ROS_network_monitor_udp_UdpMonitor_h
#define _ROS_network_monitor_udp_UdpMonitor_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace network_monitor_udp
{

  class UdpMonitor : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * server_host;
      uint32_t server_port;
      char * source_interface;
      float packet_rate;
      uint32_t packet_size;
      uint8_t latency_bin_limits_length;
      float st_latency_bin_limits;
      float * latency_bin_limits;
      uint8_t latency_bin_values_length;
      float st_latency_bin_values;
      float * latency_bin_values;
      float average_latency_fresh;
      float loss_fresh;
      float average_latency_all;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_server_host = strlen( (const char*) this->server_host);
      memcpy(outbuffer + offset, &length_server_host, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->server_host, length_server_host);
      offset += length_server_host;
      *(outbuffer + offset + 0) = (this->server_port >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->server_port >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->server_port >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->server_port >> (8 * 3)) & 0xFF;
      offset += sizeof(this->server_port);
      uint32_t length_source_interface = strlen( (const char*) this->source_interface);
      memcpy(outbuffer + offset, &length_source_interface, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->source_interface, length_source_interface);
      offset += length_source_interface;
      union {
        float real;
        uint32_t base;
      } u_packet_rate;
      u_packet_rate.real = this->packet_rate;
      *(outbuffer + offset + 0) = (u_packet_rate.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_packet_rate.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_packet_rate.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_packet_rate.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->packet_rate);
      *(outbuffer + offset + 0) = (this->packet_size >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->packet_size >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->packet_size >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->packet_size >> (8 * 3)) & 0xFF;
      offset += sizeof(this->packet_size);
      *(outbuffer + offset++) = latency_bin_limits_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < latency_bin_limits_length; i++){
      union {
        float real;
        uint32_t base;
      } u_latency_bin_limitsi;
      u_latency_bin_limitsi.real = this->latency_bin_limits[i];
      *(outbuffer + offset + 0) = (u_latency_bin_limitsi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_latency_bin_limitsi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_latency_bin_limitsi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_latency_bin_limitsi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->latency_bin_limits[i]);
      }
      *(outbuffer + offset++) = latency_bin_values_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < latency_bin_values_length; i++){
      union {
        float real;
        uint32_t base;
      } u_latency_bin_valuesi;
      u_latency_bin_valuesi.real = this->latency_bin_values[i];
      *(outbuffer + offset + 0) = (u_latency_bin_valuesi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_latency_bin_valuesi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_latency_bin_valuesi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_latency_bin_valuesi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->latency_bin_values[i]);
      }
      union {
        float real;
        uint32_t base;
      } u_average_latency_fresh;
      u_average_latency_fresh.real = this->average_latency_fresh;
      *(outbuffer + offset + 0) = (u_average_latency_fresh.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_average_latency_fresh.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_average_latency_fresh.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_average_latency_fresh.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->average_latency_fresh);
      union {
        float real;
        uint32_t base;
      } u_loss_fresh;
      u_loss_fresh.real = this->loss_fresh;
      *(outbuffer + offset + 0) = (u_loss_fresh.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_loss_fresh.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_loss_fresh.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_loss_fresh.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->loss_fresh);
      union {
        float real;
        uint32_t base;
      } u_average_latency_all;
      u_average_latency_all.real = this->average_latency_all;
      *(outbuffer + offset + 0) = (u_average_latency_all.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_average_latency_all.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_average_latency_all.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_average_latency_all.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->average_latency_all);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_server_host;
      memcpy(&length_server_host, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_server_host; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_server_host-1]=0;
      this->server_host = (char *)(inbuffer + offset-1);
      offset += length_server_host;
      this->server_port =  ((uint32_t) (*(inbuffer + offset)));
      this->server_port |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->server_port |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->server_port |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->server_port);
      uint32_t length_source_interface;
      memcpy(&length_source_interface, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_source_interface; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_source_interface-1]=0;
      this->source_interface = (char *)(inbuffer + offset-1);
      offset += length_source_interface;
      union {
        float real;
        uint32_t base;
      } u_packet_rate;
      u_packet_rate.base = 0;
      u_packet_rate.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_packet_rate.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_packet_rate.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_packet_rate.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->packet_rate = u_packet_rate.real;
      offset += sizeof(this->packet_rate);
      this->packet_size =  ((uint32_t) (*(inbuffer + offset)));
      this->packet_size |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->packet_size |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->packet_size |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->packet_size);
      uint8_t latency_bin_limits_lengthT = *(inbuffer + offset++);
      if(latency_bin_limits_lengthT > latency_bin_limits_length)
        this->latency_bin_limits = (float*)realloc(this->latency_bin_limits, latency_bin_limits_lengthT * sizeof(float));
      offset += 3;
      latency_bin_limits_length = latency_bin_limits_lengthT;
      for( uint8_t i = 0; i < latency_bin_limits_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_latency_bin_limits;
      u_st_latency_bin_limits.base = 0;
      u_st_latency_bin_limits.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_latency_bin_limits.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_latency_bin_limits.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_latency_bin_limits.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_latency_bin_limits = u_st_latency_bin_limits.real;
      offset += sizeof(this->st_latency_bin_limits);
        memcpy( &(this->latency_bin_limits[i]), &(this->st_latency_bin_limits), sizeof(float));
      }
      uint8_t latency_bin_values_lengthT = *(inbuffer + offset++);
      if(latency_bin_values_lengthT > latency_bin_values_length)
        this->latency_bin_values = (float*)realloc(this->latency_bin_values, latency_bin_values_lengthT * sizeof(float));
      offset += 3;
      latency_bin_values_length = latency_bin_values_lengthT;
      for( uint8_t i = 0; i < latency_bin_values_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_latency_bin_values;
      u_st_latency_bin_values.base = 0;
      u_st_latency_bin_values.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_latency_bin_values.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_latency_bin_values.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_latency_bin_values.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_latency_bin_values = u_st_latency_bin_values.real;
      offset += sizeof(this->st_latency_bin_values);
        memcpy( &(this->latency_bin_values[i]), &(this->st_latency_bin_values), sizeof(float));
      }
      union {
        float real;
        uint32_t base;
      } u_average_latency_fresh;
      u_average_latency_fresh.base = 0;
      u_average_latency_fresh.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_average_latency_fresh.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_average_latency_fresh.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_average_latency_fresh.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->average_latency_fresh = u_average_latency_fresh.real;
      offset += sizeof(this->average_latency_fresh);
      union {
        float real;
        uint32_t base;
      } u_loss_fresh;
      u_loss_fresh.base = 0;
      u_loss_fresh.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_loss_fresh.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_loss_fresh.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_loss_fresh.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->loss_fresh = u_loss_fresh.real;
      offset += sizeof(this->loss_fresh);
      union {
        float real;
        uint32_t base;
      } u_average_latency_all;
      u_average_latency_all.base = 0;
      u_average_latency_all.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_average_latency_all.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_average_latency_all.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_average_latency_all.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->average_latency_all = u_average_latency_all.real;
      offset += sizeof(this->average_latency_all);
     return offset;
    }

    const char * getType(){ return "network_monitor_udp/UdpMonitor"; };
    const char * getMD5(){ return "61e78eeecffb9257ae40ca291408f8cc"; };

  };

}
#endif