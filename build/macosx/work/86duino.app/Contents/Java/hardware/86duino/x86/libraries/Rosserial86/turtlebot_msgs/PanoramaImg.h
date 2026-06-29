#ifndef _ROS_turtlebot_msgs_PanoramaImg_h
#define _ROS_turtlebot_msgs_PanoramaImg_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "sensor_msgs/Image.h"

namespace turtlebot_msgs
{

  class PanoramaImg : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * pano_id;
      float latitude;
      float longitude;
      float heading;
      char * geo_tag;
      sensor_msgs::Image image;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_pano_id = strlen( (const char*) this->pano_id);
      memcpy(outbuffer + offset, &length_pano_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->pano_id, length_pano_id);
      offset += length_pano_id;
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
      int32_t * val_heading = (int32_t *) &(this->heading);
      int32_t exp_heading = (((*val_heading)>>23)&255);
      if(exp_heading != 0)
        exp_heading += 1023-127;
      int32_t sig_heading = *val_heading;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_heading<<5) & 0xff;
      *(outbuffer + offset++) = (sig_heading>>3) & 0xff;
      *(outbuffer + offset++) = (sig_heading>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_heading<<4) & 0xF0) | ((sig_heading>>19)&0x0F);
      *(outbuffer + offset++) = (exp_heading>>4) & 0x7F;
      if(this->heading < 0) *(outbuffer + offset -1) |= 0x80;
      uint32_t length_geo_tag = strlen( (const char*) this->geo_tag);
      memcpy(outbuffer + offset, &length_geo_tag, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->geo_tag, length_geo_tag);
      offset += length_geo_tag;
      offset += this->image.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_pano_id;
      memcpy(&length_pano_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_pano_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_pano_id-1]=0;
      this->pano_id = (char *)(inbuffer + offset-1);
      offset += length_pano_id;
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
      uint32_t * val_heading = (uint32_t*) &(this->heading);
      offset += 3;
      *val_heading = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_heading |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_heading |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_heading |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_heading = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_heading |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_heading !=0)
        *val_heading |= ((exp_heading)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->heading = -this->heading;
      uint32_t length_geo_tag;
      memcpy(&length_geo_tag, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_geo_tag; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_geo_tag-1]=0;
      this->geo_tag = (char *)(inbuffer + offset-1);
      offset += length_geo_tag;
      offset += this->image.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "turtlebot_msgs/PanoramaImg"; };
    const char * getMD5(){ return "aedf66295b374a7249a786af27aecc87"; };

  };

}
#endif