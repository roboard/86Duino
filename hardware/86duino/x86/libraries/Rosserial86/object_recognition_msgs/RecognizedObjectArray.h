#ifndef _ROS_object_recognition_msgs_RecognizedObjectArray_h
#define _ROS_object_recognition_msgs_RecognizedObjectArray_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "object_recognition_msgs/RecognizedObject.h"

namespace object_recognition_msgs
{

  class RecognizedObjectArray : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t objects_length;
      object_recognition_msgs::RecognizedObject st_objects;
      object_recognition_msgs::RecognizedObject * objects;
      uint8_t cooccurrence_length;
      float st_cooccurrence;
      float * cooccurrence;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = objects_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < objects_length; i++){
      offset += this->objects[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = cooccurrence_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < cooccurrence_length; i++){
      union {
        float real;
        uint32_t base;
      } u_cooccurrencei;
      u_cooccurrencei.real = this->cooccurrence[i];
      *(outbuffer + offset + 0) = (u_cooccurrencei.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_cooccurrencei.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_cooccurrencei.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_cooccurrencei.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->cooccurrence[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t objects_lengthT = *(inbuffer + offset++);
      if(objects_lengthT > objects_length)
        this->objects = (object_recognition_msgs::RecognizedObject*)realloc(this->objects, objects_lengthT * sizeof(object_recognition_msgs::RecognizedObject));
      offset += 3;
      objects_length = objects_lengthT;
      for( uint8_t i = 0; i < objects_length; i++){
      offset += this->st_objects.deserialize(inbuffer + offset);
        memcpy( &(this->objects[i]), &(this->st_objects), sizeof(object_recognition_msgs::RecognizedObject));
      }
      uint8_t cooccurrence_lengthT = *(inbuffer + offset++);
      if(cooccurrence_lengthT > cooccurrence_length)
        this->cooccurrence = (float*)realloc(this->cooccurrence, cooccurrence_lengthT * sizeof(float));
      offset += 3;
      cooccurrence_length = cooccurrence_lengthT;
      for( uint8_t i = 0; i < cooccurrence_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_cooccurrence;
      u_st_cooccurrence.base = 0;
      u_st_cooccurrence.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_cooccurrence.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_cooccurrence.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_cooccurrence.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_cooccurrence = u_st_cooccurrence.real;
      offset += sizeof(this->st_cooccurrence);
        memcpy( &(this->cooccurrence[i]), &(this->st_cooccurrence), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "object_recognition_msgs/RecognizedObjectArray"; };
    const char * getMD5(){ return "bad6b1546b9ebcabb49fb3b858d78964"; };

  };

}
#endif