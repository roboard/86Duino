#ifndef _ROS_SERVICE_GetModelDescription_h
#define _ROS_SERVICE_GetModelDescription_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "household_objects_database_msgs/DatabaseReturnCode.h"

namespace household_objects_database_msgs
{

static const char GETMODELDESCRIPTION[] = "household_objects_database_msgs/GetModelDescription";

  class GetModelDescriptionRequest : public ros::Msg
  {
    public:
      int32_t model_id;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_model_id;
      u_model_id.real = this->model_id;
      *(outbuffer + offset + 0) = (u_model_id.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_model_id.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_model_id.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_model_id.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->model_id);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_model_id;
      u_model_id.base = 0;
      u_model_id.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_model_id.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_model_id.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_model_id.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->model_id = u_model_id.real;
      offset += sizeof(this->model_id);
     return offset;
    }

    const char * getType(){ return GETMODELDESCRIPTION; };
    const char * getMD5(){ return "28cb0598daf3b969068a38cd07aaa9f6"; };

  };

  class GetModelDescriptionResponse : public ros::Msg
  {
    public:
      household_objects_database_msgs::DatabaseReturnCode return_code;
      uint8_t tags_length;
      char* st_tags;
      char* * tags;
      char * name;
      char * maker;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->return_code.serialize(outbuffer + offset);
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
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_maker = strlen( (const char*) this->maker);
      memcpy(outbuffer + offset, &length_maker, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->maker, length_maker);
      offset += length_maker;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->return_code.deserialize(inbuffer + offset);
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
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      uint32_t length_maker;
      memcpy(&length_maker, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_maker; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_maker-1]=0;
      this->maker = (char *)(inbuffer + offset-1);
      offset += length_maker;
     return offset;
    }

    const char * getType(){ return GETMODELDESCRIPTION; };
    const char * getMD5(){ return "e6c55e34b143695104d37ad9b33c72c0"; };

  };

  class GetModelDescription {
    public:
    typedef GetModelDescriptionRequest Request;
    typedef GetModelDescriptionResponse Response;
  };

}
#endif
