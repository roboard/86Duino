#ifndef _ROS_object_recognition_msgs_ObjectRecognitionGoal_h
#define _ROS_object_recognition_msgs_ObjectRecognitionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace object_recognition_msgs
{

  class ObjectRecognitionGoal : public ros::Msg
  {
    public:
      bool use_roi;
      uint8_t filter_limits_length;
      float st_filter_limits;
      float * filter_limits;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_use_roi;
      u_use_roi.real = this->use_roi;
      *(outbuffer + offset + 0) = (u_use_roi.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->use_roi);
      *(outbuffer + offset++) = filter_limits_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < filter_limits_length; i++){
      union {
        float real;
        uint32_t base;
      } u_filter_limitsi;
      u_filter_limitsi.real = this->filter_limits[i];
      *(outbuffer + offset + 0) = (u_filter_limitsi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_filter_limitsi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_filter_limitsi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_filter_limitsi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->filter_limits[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        bool real;
        uint8_t base;
      } u_use_roi;
      u_use_roi.base = 0;
      u_use_roi.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->use_roi = u_use_roi.real;
      offset += sizeof(this->use_roi);
      uint8_t filter_limits_lengthT = *(inbuffer + offset++);
      if(filter_limits_lengthT > filter_limits_length)
        this->filter_limits = (float*)realloc(this->filter_limits, filter_limits_lengthT * sizeof(float));
      offset += 3;
      filter_limits_length = filter_limits_lengthT;
      for( uint8_t i = 0; i < filter_limits_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_filter_limits;
      u_st_filter_limits.base = 0;
      u_st_filter_limits.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_filter_limits.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_filter_limits.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_filter_limits.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_filter_limits = u_st_filter_limits.real;
      offset += sizeof(this->st_filter_limits);
        memcpy( &(this->filter_limits[i]), &(this->st_filter_limits), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "object_recognition_msgs/ObjectRecognitionGoal"; };
    const char * getMD5(){ return "49bea2f03a1bba0ad05926e01e3525fa"; };

  };

}
#endif