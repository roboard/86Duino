#ifndef _ROS_moveit_msgs_LinkPadding_h
#define _ROS_moveit_msgs_LinkPadding_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace moveit_msgs
{

  class LinkPadding : public ros::Msg
  {
    public:
      char * link_name;
      float padding;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_link_name = strlen( (const char*) this->link_name);
      memcpy(outbuffer + offset, &length_link_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->link_name, length_link_name);
      offset += length_link_name;
      int32_t * val_padding = (int32_t *) &(this->padding);
      int32_t exp_padding = (((*val_padding)>>23)&255);
      if(exp_padding != 0)
        exp_padding += 1023-127;
      int32_t sig_padding = *val_padding;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_padding<<5) & 0xff;
      *(outbuffer + offset++) = (sig_padding>>3) & 0xff;
      *(outbuffer + offset++) = (sig_padding>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_padding<<4) & 0xF0) | ((sig_padding>>19)&0x0F);
      *(outbuffer + offset++) = (exp_padding>>4) & 0x7F;
      if(this->padding < 0) *(outbuffer + offset -1) |= 0x80;
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
      uint32_t * val_padding = (uint32_t*) &(this->padding);
      offset += 3;
      *val_padding = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_padding |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_padding |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_padding |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_padding = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_padding |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_padding !=0)
        *val_padding |= ((exp_padding)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->padding = -this->padding;
     return offset;
    }

    const char * getType(){ return "moveit_msgs/LinkPadding"; };
    const char * getMD5(){ return "b3ea75670df55c696fedee97774d5947"; };

  };

}
#endif