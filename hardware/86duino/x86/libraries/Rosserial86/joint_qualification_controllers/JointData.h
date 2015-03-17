#ifndef _ROS_joint_qualification_controllers_JointData_h
#define _ROS_joint_qualification_controllers_JointData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace joint_qualification_controllers
{

  class JointData : public ros::Msg
  {
    public:
      int16_t index;
      char * name;
      int8_t is_cal;
      int8_t has_safety;
      char * type;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int16_t real;
        uint16_t base;
      } u_index;
      u_index.real = this->index;
      *(outbuffer + offset + 0) = (u_index.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_index.base >> (8 * 1)) & 0xFF;
      offset += sizeof(this->index);
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      union {
        int8_t real;
        uint8_t base;
      } u_is_cal;
      u_is_cal.real = this->is_cal;
      *(outbuffer + offset + 0) = (u_is_cal.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->is_cal);
      union {
        int8_t real;
        uint8_t base;
      } u_has_safety;
      u_has_safety.real = this->has_safety;
      *(outbuffer + offset + 0) = (u_has_safety.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->has_safety);
      uint32_t length_type = strlen( (const char*) this->type);
      memcpy(outbuffer + offset, &length_type, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->type, length_type);
      offset += length_type;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int16_t real;
        uint16_t base;
      } u_index;
      u_index.base = 0;
      u_index.base |= ((uint16_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_index.base |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->index = u_index.real;
      offset += sizeof(this->index);
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      union {
        int8_t real;
        uint8_t base;
      } u_is_cal;
      u_is_cal.base = 0;
      u_is_cal.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->is_cal = u_is_cal.real;
      offset += sizeof(this->is_cal);
      union {
        int8_t real;
        uint8_t base;
      } u_has_safety;
      u_has_safety.base = 0;
      u_has_safety.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->has_safety = u_has_safety.real;
      offset += sizeof(this->has_safety);
      uint32_t length_type;
      memcpy(&length_type, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_type; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_type-1]=0;
      this->type = (char *)(inbuffer + offset-1);
      offset += length_type;
     return offset;
    }

    const char * getType(){ return "joint_qualification_controllers/JointData"; };
    const char * getMD5(){ return "191610564bf935172c9424c803ed6939"; };

  };

}
#endif