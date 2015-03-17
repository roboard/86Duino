#ifndef _ROS_people_msgs_PositionMeasurementArray_h
#define _ROS_people_msgs_PositionMeasurementArray_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "people_msgs/PositionMeasurement.h"

namespace people_msgs
{

  class PositionMeasurementArray : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t people_length;
      people_msgs::PositionMeasurement st_people;
      people_msgs::PositionMeasurement * people;
      uint8_t cooccurrence_length;
      float st_cooccurrence;
      float * cooccurrence;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = people_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < people_length; i++){
      offset += this->people[i].serialize(outbuffer + offset);
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
      uint8_t people_lengthT = *(inbuffer + offset++);
      if(people_lengthT > people_length)
        this->people = (people_msgs::PositionMeasurement*)realloc(this->people, people_lengthT * sizeof(people_msgs::PositionMeasurement));
      offset += 3;
      people_length = people_lengthT;
      for( uint8_t i = 0; i < people_length; i++){
      offset += this->st_people.deserialize(inbuffer + offset);
        memcpy( &(this->people[i]), &(this->st_people), sizeof(people_msgs::PositionMeasurement));
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

    const char * getType(){ return "people_msgs/PositionMeasurementArray"; };
    const char * getMD5(){ return "59c860d40aa739ec920eb3ad24ae019e"; };

  };

}
#endif