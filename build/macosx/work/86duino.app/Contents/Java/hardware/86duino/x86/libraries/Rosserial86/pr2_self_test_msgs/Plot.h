#ifndef _ROS_pr2_self_test_msgs_Plot_h
#define _ROS_pr2_self_test_msgs_Plot_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_self_test_msgs
{

  class Plot : public ros::Msg
  {
    public:
      char * title;
      uint8_t image_length;
      int8_t st_image;
      int8_t * image;
      char * image_format;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_title = strlen( (const char*) this->title);
      memcpy(outbuffer + offset, &length_title, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->title, length_title);
      offset += length_title;
      *(outbuffer + offset++) = image_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < image_length; i++){
      union {
        int8_t real;
        uint8_t base;
      } u_imagei;
      u_imagei.real = this->image[i];
      *(outbuffer + offset + 0) = (u_imagei.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->image[i]);
      }
      uint32_t length_image_format = strlen( (const char*) this->image_format);
      memcpy(outbuffer + offset, &length_image_format, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->image_format, length_image_format);
      offset += length_image_format;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_title;
      memcpy(&length_title, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_title; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_title-1]=0;
      this->title = (char *)(inbuffer + offset-1);
      offset += length_title;
      uint8_t image_lengthT = *(inbuffer + offset++);
      if(image_lengthT > image_length)
        this->image = (int8_t*)realloc(this->image, image_lengthT * sizeof(int8_t));
      offset += 3;
      image_length = image_lengthT;
      for( uint8_t i = 0; i < image_length; i++){
      union {
        int8_t real;
        uint8_t base;
      } u_st_image;
      u_st_image.base = 0;
      u_st_image.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->st_image = u_st_image.real;
      offset += sizeof(this->st_image);
        memcpy( &(this->image[i]), &(this->st_image), sizeof(int8_t));
      }
      uint32_t length_image_format;
      memcpy(&length_image_format, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_image_format; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_image_format-1]=0;
      this->image_format = (char *)(inbuffer + offset-1);
      offset += length_image_format;
     return offset;
    }

    const char * getType(){ return "pr2_self_test_msgs/Plot"; };
    const char * getMD5(){ return "7b2b3d1ff7d1699544a2479e9175f3fb"; };

  };

}
#endif