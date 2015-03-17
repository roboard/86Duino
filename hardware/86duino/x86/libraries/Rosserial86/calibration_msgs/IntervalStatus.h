#ifndef _ROS_calibration_msgs_IntervalStatus_h
#define _ROS_calibration_msgs_IntervalStatus_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace calibration_msgs
{

  class IntervalStatus : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t names_length;
      char* st_names;
      char* * names;
      uint8_t yeild_rates_length;
      float st_yeild_rates;
      float * yeild_rates;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = names_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < names_length; i++){
      uint32_t length_namesi = strlen( (const char*) this->names[i]);
      memcpy(outbuffer + offset, &length_namesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->names[i], length_namesi);
      offset += length_namesi;
      }
      *(outbuffer + offset++) = yeild_rates_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < yeild_rates_length; i++){
      union {
        float real;
        uint32_t base;
      } u_yeild_ratesi;
      u_yeild_ratesi.real = this->yeild_rates[i];
      *(outbuffer + offset + 0) = (u_yeild_ratesi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_yeild_ratesi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_yeild_ratesi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_yeild_ratesi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->yeild_rates[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t names_lengthT = *(inbuffer + offset++);
      if(names_lengthT > names_length)
        this->names = (char**)realloc(this->names, names_lengthT * sizeof(char*));
      offset += 3;
      names_length = names_lengthT;
      for( uint8_t i = 0; i < names_length; i++){
      uint32_t length_st_names;
      memcpy(&length_st_names, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_names; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_names-1]=0;
      this->st_names = (char *)(inbuffer + offset-1);
      offset += length_st_names;
        memcpy( &(this->names[i]), &(this->st_names), sizeof(char*));
      }
      uint8_t yeild_rates_lengthT = *(inbuffer + offset++);
      if(yeild_rates_lengthT > yeild_rates_length)
        this->yeild_rates = (float*)realloc(this->yeild_rates, yeild_rates_lengthT * sizeof(float));
      offset += 3;
      yeild_rates_length = yeild_rates_lengthT;
      for( uint8_t i = 0; i < yeild_rates_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_yeild_rates;
      u_st_yeild_rates.base = 0;
      u_st_yeild_rates.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_yeild_rates.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_yeild_rates.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_yeild_rates.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_yeild_rates = u_st_yeild_rates.real;
      offset += sizeof(this->st_yeild_rates);
        memcpy( &(this->yeild_rates[i]), &(this->st_yeild_rates), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "calibration_msgs/IntervalStatus"; };
    const char * getMD5(){ return "277fe87e9a44a07ab27c97b6b37d5898"; };

  };

}
#endif