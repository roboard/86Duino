#ifndef _ROS_ethercat_hardware_RawFTData_h
#define _ROS_ethercat_hardware_RawFTData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ethercat_hardware/RawFTDataSample.h"

namespace ethercat_hardware
{

  class RawFTData : public ros::Msg
  {
    public:
      uint8_t samples_length;
      ethercat_hardware::RawFTDataSample st_samples;
      ethercat_hardware::RawFTDataSample * samples;
      int64_t sample_count;
      int64_t missed_samples;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = samples_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < samples_length; i++){
      offset += this->samples[i].serialize(outbuffer + offset);
      }
      union {
        int64_t real;
        uint64_t base;
      } u_sample_count;
      u_sample_count.real = this->sample_count;
      *(outbuffer + offset + 0) = (u_sample_count.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_sample_count.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_sample_count.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_sample_count.base >> (8 * 3)) & 0xFF;
      *(outbuffer + offset + 4) = (u_sample_count.base >> (8 * 4)) & 0xFF;
      *(outbuffer + offset + 5) = (u_sample_count.base >> (8 * 5)) & 0xFF;
      *(outbuffer + offset + 6) = (u_sample_count.base >> (8 * 6)) & 0xFF;
      *(outbuffer + offset + 7) = (u_sample_count.base >> (8 * 7)) & 0xFF;
      offset += sizeof(this->sample_count);
      union {
        int64_t real;
        uint64_t base;
      } u_missed_samples;
      u_missed_samples.real = this->missed_samples;
      *(outbuffer + offset + 0) = (u_missed_samples.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_missed_samples.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_missed_samples.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_missed_samples.base >> (8 * 3)) & 0xFF;
      *(outbuffer + offset + 4) = (u_missed_samples.base >> (8 * 4)) & 0xFF;
      *(outbuffer + offset + 5) = (u_missed_samples.base >> (8 * 5)) & 0xFF;
      *(outbuffer + offset + 6) = (u_missed_samples.base >> (8 * 6)) & 0xFF;
      *(outbuffer + offset + 7) = (u_missed_samples.base >> (8 * 7)) & 0xFF;
      offset += sizeof(this->missed_samples);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t samples_lengthT = *(inbuffer + offset++);
      if(samples_lengthT > samples_length)
        this->samples = (ethercat_hardware::RawFTDataSample*)realloc(this->samples, samples_lengthT * sizeof(ethercat_hardware::RawFTDataSample));
      offset += 3;
      samples_length = samples_lengthT;
      for( uint8_t i = 0; i < samples_length; i++){
      offset += this->st_samples.deserialize(inbuffer + offset);
        memcpy( &(this->samples[i]), &(this->st_samples), sizeof(ethercat_hardware::RawFTDataSample));
      }
      union {
        int64_t real;
        uint64_t base;
      } u_sample_count;
      u_sample_count.base = 0;
      u_sample_count.base |= ((uint64_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_sample_count.base |= ((uint64_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_sample_count.base |= ((uint64_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_sample_count.base |= ((uint64_t) (*(inbuffer + offset + 3))) << (8 * 3);
      u_sample_count.base |= ((uint64_t) (*(inbuffer + offset + 4))) << (8 * 4);
      u_sample_count.base |= ((uint64_t) (*(inbuffer + offset + 5))) << (8 * 5);
      u_sample_count.base |= ((uint64_t) (*(inbuffer + offset + 6))) << (8 * 6);
      u_sample_count.base |= ((uint64_t) (*(inbuffer + offset + 7))) << (8 * 7);
      this->sample_count = u_sample_count.real;
      offset += sizeof(this->sample_count);
      union {
        int64_t real;
        uint64_t base;
      } u_missed_samples;
      u_missed_samples.base = 0;
      u_missed_samples.base |= ((uint64_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_missed_samples.base |= ((uint64_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_missed_samples.base |= ((uint64_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_missed_samples.base |= ((uint64_t) (*(inbuffer + offset + 3))) << (8 * 3);
      u_missed_samples.base |= ((uint64_t) (*(inbuffer + offset + 4))) << (8 * 4);
      u_missed_samples.base |= ((uint64_t) (*(inbuffer + offset + 5))) << (8 * 5);
      u_missed_samples.base |= ((uint64_t) (*(inbuffer + offset + 6))) << (8 * 6);
      u_missed_samples.base |= ((uint64_t) (*(inbuffer + offset + 7))) << (8 * 7);
      this->missed_samples = u_missed_samples.real;
      offset += sizeof(this->missed_samples);
     return offset;
    }

    const char * getType(){ return "ethercat_hardware/RawFTData"; };
    const char * getMD5(){ return "85f5ed45095367bfb8fb2e57954c0b89"; };

  };

}
#endif