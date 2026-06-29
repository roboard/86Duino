#ifndef _ROS_geographic_msgs_GeoPoint_h
#define _ROS_geographic_msgs_GeoPoint_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace geographic_msgs
{

  class GeoPoint : public ros::Msg
  {
    public:
      float latitude;
      float longitude;
      float altitude;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_latitude = (int32_t *) &(this->latitude);
      int32_t exp_latitude = (((*val_latitude)>>23)&255);
      if(exp_latitude != 0)
        exp_latitude += 1023-127;
      int32_t sig_latitude = *val_latitude;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_latitude<<5) & 0xff;
      *(outbuffer + offset++) = (sig_latitude>>3) & 0xff;
      *(outbuffer + offset++) = (sig_latitude>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_latitude<<4) & 0xF0) | ((sig_latitude>>19)&0x0F);
      *(outbuffer + offset++) = (exp_latitude>>4) & 0x7F;
      if(this->latitude < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_longitude = (int32_t *) &(this->longitude);
      int32_t exp_longitude = (((*val_longitude)>>23)&255);
      if(exp_longitude != 0)
        exp_longitude += 1023-127;
      int32_t sig_longitude = *val_longitude;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_longitude<<5) & 0xff;
      *(outbuffer + offset++) = (sig_longitude>>3) & 0xff;
      *(outbuffer + offset++) = (sig_longitude>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_longitude<<4) & 0xF0) | ((sig_longitude>>19)&0x0F);
      *(outbuffer + offset++) = (exp_longitude>>4) & 0x7F;
      if(this->longitude < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_altitude = (int32_t *) &(this->altitude);
      int32_t exp_altitude = (((*val_altitude)>>23)&255);
      if(exp_altitude != 0)
        exp_altitude += 1023-127;
      int32_t sig_altitude = *val_altitude;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_altitude<<5) & 0xff;
      *(outbuffer + offset++) = (sig_altitude>>3) & 0xff;
      *(outbuffer + offset++) = (sig_altitude>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_altitude<<4) & 0xF0) | ((sig_altitude>>19)&0x0F);
      *(outbuffer + offset++) = (exp_altitude>>4) & 0x7F;
      if(this->altitude < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_latitude = (uint32_t*) &(this->latitude);
      offset += 3;
      *val_latitude = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_latitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_latitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_latitude |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_latitude = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_latitude |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_latitude !=0)
        *val_latitude |= ((exp_latitude)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->latitude = -this->latitude;
      uint32_t * val_longitude = (uint32_t*) &(this->longitude);
      offset += 3;
      *val_longitude = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_longitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_longitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_longitude |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_longitude = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_longitude |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_longitude !=0)
        *val_longitude |= ((exp_longitude)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->longitude = -this->longitude;
      uint32_t * val_altitude = (uint32_t*) &(this->altitude);
      offset += 3;
      *val_altitude = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_altitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_altitude |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_altitude |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_altitude = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_altitude |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_altitude !=0)
        *val_altitude |= ((exp_altitude)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->altitude = -this->altitude;
     return offset;
    }

    const char * getType(){ return "geographic_msgs/GeoPoint"; };
    const char * getMD5(){ return "c48027a852aeff972be80478ff38e81a"; };

  };

}
#endif