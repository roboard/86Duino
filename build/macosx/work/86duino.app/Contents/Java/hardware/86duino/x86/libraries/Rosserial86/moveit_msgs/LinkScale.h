#ifndef _ROS_moveit_msgs_LinkScale_h
#define _ROS_moveit_msgs_LinkScale_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace moveit_msgs
{

  class LinkScale : public ros::Msg
  {
    public:
      char * link_name;
      float scale;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_link_name = strlen( (const char*) this->link_name);
      memcpy(outbuffer + offset, &length_link_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->link_name, length_link_name);
      offset += length_link_name;
      int32_t * val_scale = (int32_t *) &(this->scale);
      int32_t exp_scale = (((*val_scale)>>23)&255);
      if(exp_scale != 0)
        exp_scale += 1023-127;
      int32_t sig_scale = *val_scale;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_scale<<5) & 0xff;
      *(outbuffer + offset++) = (sig_scale>>3) & 0xff;
      *(outbuffer + offset++) = (sig_scale>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_scale<<4) & 0xF0) | ((sig_scale>>19)&0x0F);
      *(outbuffer + offset++) = (exp_scale>>4) & 0x7F;
      if(this->scale < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_link_name;
      memcpy(&length_link_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_link_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_link_name-1]=0;
      this->link_name = (char *)(inbuffer + offset-1);
      offset += length_link_name;
      uint32_t * val_scale = (uint32_t*) &(this->scale);
      offset += 3;
      *val_scale = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_scale |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_scale |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_scale |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_scale = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_scale |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_scale !=0)
        *val_scale |= ((exp_scale)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->scale = -this->scale;
     return offset;
    }

    const char * getType(){ return "moveit_msgs/LinkScale"; };
    const char * getMD5(){ return "19faf226446bfb2f645a4da6f2a56166"; };

  };

}
#endif