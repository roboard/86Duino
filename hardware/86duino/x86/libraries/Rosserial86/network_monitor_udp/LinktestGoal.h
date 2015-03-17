#ifndef _ROS_network_monitor_udp_LinktestGoal_h
#define _ROS_network_monitor_udp_LinktestGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace network_monitor_udp
{

  class LinktestGoal : public ros::Msg
  {
    public:
      float duration;
      float update_interval;
      float bw;
      uint8_t bw_type;
      float latency_threshold;
      float pktloss_threshold;
      uint8_t tos;
      int32_t pktsize;
      bool ros_returnpath;
      bool roundtrip;
      float max_return_time;
      char * rostopic_prefix;
      char * sink_ip;
      int32_t sink_port;
      uint8_t latencybins_length;
      float st_latencybins;
      float * latencybins;
      enum { DEFAULT_UPDATE_INTERVAL = 0.15     };
      enum { DEFAULT_BW = 5000000.0             };
      enum { DEFAULT_BWTYPE = 99                   };
      enum { BW_CONSTANT = 99                      };
      enum { BW_ADAPTIVE = 97                      };
      enum { LIM1 = -0.3                        };
      enum { LIM2 = 0.3 };
      enum { C1 = -0.05 };
      enum { C2 = 0.2 };
      enum { DEFAULT_LATENCY_THRESHOLD = 0.01   };
      enum { DEFAULT_PKTLOSS_THRESHOLD = 0.5    };
      enum { DEFAULT_PKTSIZE = 1500               };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_duration;
      u_duration.real = this->duration;
      *(outbuffer + offset + 0) = (u_duration.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_duration.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_duration.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_duration.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->duration);
      union {
        float real;
        uint32_t base;
      } u_update_interval;
      u_update_interval.real = this->update_interval;
      *(outbuffer + offset + 0) = (u_update_interval.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_update_interval.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_update_interval.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_update_interval.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->update_interval);
      union {
        float real;
        uint32_t base;
      } u_bw;
      u_bw.real = this->bw;
      *(outbuffer + offset + 0) = (u_bw.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_bw.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_bw.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_bw.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->bw);
      *(outbuffer + offset + 0) = (this->bw_type >> (8 * 0)) & 0xFF;
      offset += sizeof(this->bw_type);
      union {
        float real;
        uint32_t base;
      } u_latency_threshold;
      u_latency_threshold.real = this->latency_threshold;
      *(outbuffer + offset + 0) = (u_latency_threshold.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_latency_threshold.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_latency_threshold.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_latency_threshold.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->latency_threshold);
      union {
        float real;
        uint32_t base;
      } u_pktloss_threshold;
      u_pktloss_threshold.real = this->pktloss_threshold;
      *(outbuffer + offset + 0) = (u_pktloss_threshold.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_pktloss_threshold.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_pktloss_threshold.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_pktloss_threshold.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->pktloss_threshold);
      *(outbuffer + offset + 0) = (this->tos >> (8 * 0)) & 0xFF;
      offset += sizeof(this->tos);
      union {
        int32_t real;
        uint32_t base;
      } u_pktsize;
      u_pktsize.real = this->pktsize;
      *(outbuffer + offset + 0) = (u_pktsize.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_pktsize.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_pktsize.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_pktsize.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->pktsize);
      union {
        bool real;
        uint8_t base;
      } u_ros_returnpath;
      u_ros_returnpath.real = this->ros_returnpath;
      *(outbuffer + offset + 0) = (u_ros_returnpath.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->ros_returnpath);
      union {
        bool real;
        uint8_t base;
      } u_roundtrip;
      u_roundtrip.real = this->roundtrip;
      *(outbuffer + offset + 0) = (u_roundtrip.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->roundtrip);
      union {
        float real;
        uint32_t base;
      } u_max_return_time;
      u_max_return_time.real = this->max_return_time;
      *(outbuffer + offset + 0) = (u_max_return_time.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_max_return_time.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_max_return_time.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_max_return_time.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->max_return_time);
      uint32_t length_rostopic_prefix = strlen( (const char*) this->rostopic_prefix);
      memcpy(outbuffer + offset, &length_rostopic_prefix, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->rostopic_prefix, length_rostopic_prefix);
      offset += length_rostopic_prefix;
      uint32_t length_sink_ip = strlen( (const char*) this->sink_ip);
      memcpy(outbuffer + offset, &length_sink_ip, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->sink_ip, length_sink_ip);
      offset += length_sink_ip;
      union {
        int32_t real;
        uint32_t base;
      } u_sink_port;
      u_sink_port.real = this->sink_port;
      *(outbuffer + offset + 0) = (u_sink_port.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_sink_port.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_sink_port.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_sink_port.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->sink_port);
      *(outbuffer + offset++) = latencybins_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < latencybins_length; i++){
      union {
        float real;
        uint32_t base;
      } u_latencybinsi;
      u_latencybinsi.real = this->latencybins[i];
      *(outbuffer + offset + 0) = (u_latencybinsi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_latencybinsi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_latencybinsi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_latencybinsi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->latencybins[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_duration;
      u_duration.base = 0;
      u_duration.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_duration.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_duration.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_duration.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->duration = u_duration.real;
      offset += sizeof(this->duration);
      union {
        float real;
        uint32_t base;
      } u_update_interval;
      u_update_interval.base = 0;
      u_update_interval.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_update_interval.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_update_interval.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_update_interval.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->update_interval = u_update_interval.real;
      offset += sizeof(this->update_interval);
      union {
        float real;
        uint32_t base;
      } u_bw;
      u_bw.base = 0;
      u_bw.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_bw.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_bw.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_bw.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->bw = u_bw.real;
      offset += sizeof(this->bw);
      this->bw_type =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->bw_type);
      union {
        float real;
        uint32_t base;
      } u_latency_threshold;
      u_latency_threshold.base = 0;
      u_latency_threshold.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_latency_threshold.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_latency_threshold.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_latency_threshold.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->latency_threshold = u_latency_threshold.real;
      offset += sizeof(this->latency_threshold);
      union {
        float real;
        uint32_t base;
      } u_pktloss_threshold;
      u_pktloss_threshold.base = 0;
      u_pktloss_threshold.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_pktloss_threshold.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_pktloss_threshold.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_pktloss_threshold.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->pktloss_threshold = u_pktloss_threshold.real;
      offset += sizeof(this->pktloss_threshold);
      this->tos =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->tos);
      union {
        int32_t real;
        uint32_t base;
      } u_pktsize;
      u_pktsize.base = 0;
      u_pktsize.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_pktsize.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_pktsize.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_pktsize.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->pktsize = u_pktsize.real;
      offset += sizeof(this->pktsize);
      union {
        bool real;
        uint8_t base;
      } u_ros_returnpath;
      u_ros_returnpath.base = 0;
      u_ros_returnpath.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->ros_returnpath = u_ros_returnpath.real;
      offset += sizeof(this->ros_returnpath);
      union {
        bool real;
        uint8_t base;
      } u_roundtrip;
      u_roundtrip.base = 0;
      u_roundtrip.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->roundtrip = u_roundtrip.real;
      offset += sizeof(this->roundtrip);
      union {
        float real;
        uint32_t base;
      } u_max_return_time;
      u_max_return_time.base = 0;
      u_max_return_time.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_max_return_time.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_max_return_time.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_max_return_time.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->max_return_time = u_max_return_time.real;
      offset += sizeof(this->max_return_time);
      uint32_t length_rostopic_prefix;
      memcpy(&length_rostopic_prefix, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_rostopic_prefix; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_rostopic_prefix-1]=0;
      this->rostopic_prefix = (char *)(inbuffer + offset-1);
      offset += length_rostopic_prefix;
      uint32_t length_sink_ip;
      memcpy(&length_sink_ip, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_sink_ip; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_sink_ip-1]=0;
      this->sink_ip = (char *)(inbuffer + offset-1);
      offset += length_sink_ip;
      union {
        int32_t real;
        uint32_t base;
      } u_sink_port;
      u_sink_port.base = 0;
      u_sink_port.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_sink_port.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_sink_port.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_sink_port.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->sink_port = u_sink_port.real;
      offset += sizeof(this->sink_port);
      uint8_t latencybins_lengthT = *(inbuffer + offset++);
      if(latencybins_lengthT > latencybins_length)
        this->latencybins = (float*)realloc(this->latencybins, latencybins_lengthT * sizeof(float));
      offset += 3;
      latencybins_length = latencybins_lengthT;
      for( uint8_t i = 0; i < latencybins_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_latencybins;
      u_st_latencybins.base = 0;
      u_st_latencybins.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_latencybins.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_latencybins.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_latencybins.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_latencybins = u_st_latencybins.real;
      offset += sizeof(this->st_latencybins);
        memcpy( &(this->latencybins[i]), &(this->st_latencybins), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "network_monitor_udp/LinktestGoal"; };
    const char * getMD5(){ return "a319f2787ce16837363051a27c7fd49f"; };

  };

}
#endif