#ifndef _ROS_joint_qualification_controllers_CBRunData_h
#define _ROS_joint_qualification_controllers_CBRunData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "joint_qualification_controllers/CBPositionData.h"

namespace joint_qualification_controllers
{

  class CBRunData : public ros::Msg
  {
    public:
      float lift_position;
      uint8_t flex_data_length;
      joint_qualification_controllers::CBPositionData st_flex_data;
      joint_qualification_controllers::CBPositionData * flex_data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_lift_position;
      u_lift_position.real = this->lift_position;
      *(outbuffer + offset + 0) = (u_lift_position.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_lift_position.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_lift_position.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_lift_position.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->lift_position);
      *(outbuffer + offset++) = flex_data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < flex_data_length; i++){
      offset += this->flex_data[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_lift_position;
      u_lift_position.base = 0;
      u_lift_position.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_lift_position.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_lift_position.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_lift_position.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->lift_position = u_lift_position.real;
      offset += sizeof(this->lift_position);
      uint8_t flex_data_lengthT = *(inbuffer + offset++);
      if(flex_data_lengthT > flex_data_length)
        this->flex_data = (joint_qualification_controllers::CBPositionData*)realloc(this->flex_data, flex_data_lengthT * sizeof(joint_qualification_controllers::CBPositionData));
      offset += 3;
      flex_data_length = flex_data_lengthT;
      for( uint8_t i = 0; i < flex_data_length; i++){
      offset += this->st_flex_data.deserialize(inbuffer + offset);
        memcpy( &(this->flex_data[i]), &(this->st_flex_data), sizeof(joint_qualification_controllers::CBPositionData));
      }
     return offset;
    }

    const char * getType(){ return "joint_qualification_controllers/CBRunData"; };
    const char * getMD5(){ return "16bad2df24f5cffc934c2d3ae9bb08e6"; };

  };

}
#endif