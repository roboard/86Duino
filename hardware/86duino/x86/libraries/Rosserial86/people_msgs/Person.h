#ifndef _ROS_people_msgs_Person_h
#define _ROS_people_msgs_Person_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Point.h"

namespace people_msgs
{

  class Person : public ros::Msg
  {
    public:
      char * name;
      geometry_msgs::Point position;
      geometry_msgs::Point velocity;
      float reliability;
      uint8_t tagnames_length;
      char* st_tagnames;
      char* * tagnames;
      uint8_t tags_length;
      char* st_tags;
      char* * tags;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      offset += this->position.serialize(outbuffer + offset);
      offset += this->velocity.serialize(outbuffer + offset);
      int32_t * val_reliability = (int32_t *) &(this->reliability);
      int32_t exp_reliability = (((*val_reliability)>>23)&255);
      if(exp_reliability != 0)
        exp_reliability += 1023-127;
      int32_t sig_reliability = *val_reliability;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_reliability<<5) & 0xff;
      *(outbuffer + offset++) = (sig_reliability>>3) & 0xff;
      *(outbuffer + offset++) = (sig_reliability>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_reliability<<4) & 0xF0) | ((sig_reliability>>19)&0x0F);
      *(outbuffer + offset++) = (exp_reliability>>4) & 0x7F;
      if(this->reliability < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset++) = tagnames_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tagnames_length; i++){
      uint32_t length_tagnamesi = strlen( (const char*) this->tagnames[i]);
      memcpy(outbuffer + offset, &length_tagnamesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->tagnames[i], length_tagnamesi);
      offset += length_tagnamesi;
      }
      *(outbuffer + offset++) = tags_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tags_length; i++){
      uint32_t length_tagsi = strlen( (const char*) this->tags[i]);
      memcpy(outbuffer + offset, &length_tagsi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->tags[i], length_tagsi);
      offset += length_tagsi;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      offset += this->position.deserialize(inbuffer + offset);
      offset += this->velocity.deserialize(inbuffer + offset);
      uint32_t * val_reliability = (uint32_t*) &(this->reliability);
      offset += 3;
      *val_reliability = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_reliability |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_reliability |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_reliability |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_reliability = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_reliability |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_reliability !=0)
        *val_reliability |= ((exp_reliability)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->reliability = -this->reliability;
      uint8_t tagnames_lengthT = *(inbuffer + offset++);
      if(tagnames_lengthT > tagnames_length)
        this->tagnames = (char**)realloc(this->tagnames, tagnames_lengthT * sizeof(char*));
      offset += 3;
      tagnames_length = tagnames_lengthT;
      for( uint8_t i = 0; i < tagnames_length; i++){
      uint32_t length_st_tagnames;
      memcpy(&length_st_tagnames, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_tagnames; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_tagnames-1]=0;
      this->st_tagnames = (char *)(inbuffer + offset-1);
      offset += length_st_tagnames;
        memcpy( &(this->tagnames[i]), &(this->st_tagnames), sizeof(char*));
      }
      uint8_t tags_lengthT = *(inbuffer + offset++);
      if(tags_lengthT > tags_length)
        this->tags = (char**)realloc(this->tags, tags_lengthT * sizeof(char*));
      offset += 3;
      tags_length = tags_lengthT;
      for( uint8_t i = 0; i < tags_length; i++){
      uint32_t length_st_tags;
      memcpy(&length_st_tags, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_tags; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_tags-1]=0;
      this->st_tags = (char *)(inbuffer + offset-1);
      offset += length_st_tags;
        memcpy( &(this->tags[i]), &(this->st_tags), sizeof(char*));
      }
     return offset;
    }

    const char * getType(){ return "people_msgs/Person"; };
    const char * getMD5(){ return "0b7c0818b76476f3863bd13f4d59f8df"; };

  };

}
#endif