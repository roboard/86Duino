#ifndef _ROS_SERVICE_TranslateRecognitionId_h
#define _ROS_SERVICE_TranslateRecognitionId_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace household_objects_database_msgs
{

static const char TRANSLATERECOGNITIONID[] = "household_objects_database_msgs/TranslateRecognitionId";

  class TranslateRecognitionIdRequest : public ros::Msg
  {
    public:
      char * recognition_id;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_recognition_id = strlen( (const char*) this->recognition_id);
      memcpy(outbuffer + offset, &length_recognition_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->recognition_id, length_recognition_id);
      offset += length_recognition_id;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_recognition_id;
      memcpy(&length_recognition_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_recognition_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_recognition_id-1]=0;
      this->recognition_id = (char *)(inbuffer + offset-1);
      offset += length_recognition_id;
     return offset;
    }

    const char * getType(){ return TRANSLATERECOGNITIONID; };
    const char * getMD5(){ return "4d95610f63ed69a670a8f8aaa3c6aa36"; };

  };

  class TranslateRecognitionIdResponse : public ros::Msg
  {
    public:
      int32_t household_objects_id;
      int32_t ID_NOT_FOUND;
      int32_t DATABASE_ERROR;
      int32_t OTHER_ERROR;
      int32_t result;
      enum { SUCCESS = 0 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_household_objects_id;
      u_household_objects_id.real = this->household_objects_id;
      *(outbuffer + offset + 0) = (u_household_objects_id.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_household_objects_id.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_household_objects_id.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_household_objects_id.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->household_objects_id);
      union {
        int32_t real;
        uint32_t base;
      } u_ID_NOT_FOUND;
      u_ID_NOT_FOUND.real = this->ID_NOT_FOUND;
      *(outbuffer + offset + 0) = (u_ID_NOT_FOUND.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_ID_NOT_FOUND.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_ID_NOT_FOUND.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_ID_NOT_FOUND.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->ID_NOT_FOUND);
      union {
        int32_t real;
        uint32_t base;
      } u_DATABASE_ERROR;
      u_DATABASE_ERROR.real = this->DATABASE_ERROR;
      *(outbuffer + offset + 0) = (u_DATABASE_ERROR.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_DATABASE_ERROR.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_DATABASE_ERROR.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_DATABASE_ERROR.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->DATABASE_ERROR);
      union {
        int32_t real;
        uint32_t base;
      } u_OTHER_ERROR;
      u_OTHER_ERROR.real = this->OTHER_ERROR;
      *(outbuffer + offset + 0) = (u_OTHER_ERROR.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_OTHER_ERROR.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_OTHER_ERROR.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_OTHER_ERROR.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->OTHER_ERROR);
      union {
        int32_t real;
        uint32_t base;
      } u_result;
      u_result.real = this->result;
      *(outbuffer + offset + 0) = (u_result.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_result.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_result.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_result.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->result);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_household_objects_id;
      u_household_objects_id.base = 0;
      u_household_objects_id.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_household_objects_id.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_household_objects_id.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_household_objects_id.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->household_objects_id = u_household_objects_id.real;
      offset += sizeof(this->household_objects_id);
      union {
        int32_t real;
        uint32_t base;
      } u_ID_NOT_FOUND;
      u_ID_NOT_FOUND.base = 0;
      u_ID_NOT_FOUND.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_ID_NOT_FOUND.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_ID_NOT_FOUND.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_ID_NOT_FOUND.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->ID_NOT_FOUND = u_ID_NOT_FOUND.real;
      offset += sizeof(this->ID_NOT_FOUND);
      union {
        int32_t real;
        uint32_t base;
      } u_DATABASE_ERROR;
      u_DATABASE_ERROR.base = 0;
      u_DATABASE_ERROR.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_DATABASE_ERROR.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_DATABASE_ERROR.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_DATABASE_ERROR.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->DATABASE_ERROR = u_DATABASE_ERROR.real;
      offset += sizeof(this->DATABASE_ERROR);
      union {
        int32_t real;
        uint32_t base;
      } u_OTHER_ERROR;
      u_OTHER_ERROR.base = 0;
      u_OTHER_ERROR.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_OTHER_ERROR.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_OTHER_ERROR.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_OTHER_ERROR.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->OTHER_ERROR = u_OTHER_ERROR.real;
      offset += sizeof(this->OTHER_ERROR);
      union {
        int32_t real;
        uint32_t base;
      } u_result;
      u_result.base = 0;
      u_result.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_result.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_result.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_result.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->result = u_result.real;
      offset += sizeof(this->result);
     return offset;
    }

    const char * getType(){ return TRANSLATERECOGNITIONID; };
    const char * getMD5(){ return "b4afd505fbee150bf06acaffa82e84cf"; };

  };

  class TranslateRecognitionId {
    public:
    typedef TranslateRecognitionIdRequest Request;
    typedef TranslateRecognitionIdResponse Response;
  };

}
#endif
