#ifndef _ROS_moveit_simple_grasps_GraspGeneratorOptions_h
#define _ROS_moveit_simple_grasps_GraspGeneratorOptions_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace moveit_simple_grasps
{

  class GraspGeneratorOptions : public ros::Msg
  {
    public:
      uint8_t grasp_axis;
      uint8_t grasp_direction;
      uint8_t grasp_rotation;
      enum { GRASP_AXIS_X =  0 };
      enum { GRASP_AXIS_Y =  1 };
      enum { GRASP_AXIS_Z =  2 };
      enum { GRASP_DIRECTION_UP =  0 };
      enum { GRASP_DIRECTION_DOWN =  1 };
      enum { GRASP_ROTATION_HALF =  0 };
      enum { GRASP_ROTATION_FULL =  1 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->grasp_axis >> (8 * 0)) & 0xFF;
      offset += sizeof(this->grasp_axis);
      *(outbuffer + offset + 0) = (this->grasp_direction >> (8 * 0)) & 0xFF;
      offset += sizeof(this->grasp_direction);
      *(outbuffer + offset + 0) = (this->grasp_rotation >> (8 * 0)) & 0xFF;
      offset += sizeof(this->grasp_rotation);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->grasp_axis =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->grasp_axis);
      this->grasp_direction =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->grasp_direction);
      this->grasp_rotation =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->grasp_rotation);
     return offset;
    }

    const char * getType(){ return "moveit_simple_grasps/GraspGeneratorOptions"; };
    const char * getMD5(){ return "99863eab001728611523218e2a9810a3"; };

  };

}
#endif