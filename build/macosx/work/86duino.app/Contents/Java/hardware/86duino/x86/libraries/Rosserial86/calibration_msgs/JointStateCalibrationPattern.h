#ifndef _ROS_calibration_msgs_JointStateCalibrationPattern_h
#define _ROS_calibration_msgs_JointStateCalibrationPattern_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Point.h"
#include "sensor_msgs/JointState.h"

namespace calibration_msgs
{

  class JointStateCalibrationPattern : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t object_points_length;
      geometry_msgs::Point st_object_points;
      geometry_msgs::Point * object_points;
      uint8_t joint_points_length;
      sensor_msgs::JointState st_joint_points;
      sensor_msgs::JointState * joint_points;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = object_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < object_points_length; i++){
      offset += this->object_points[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = joint_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_points_length; i++){
      offset += this->joint_points[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t object_points_lengthT = *(inbuffer + offset++);
      if(object_points_lengthT > object_points_length)
        this->object_points = (geometry_msgs::Point*)realloc(this->object_points, object_points_lengthT * sizeof(geometry_msgs::Point));
      offset += 3;
      object_points_length = object_points_lengthT;
      for( uint8_t i = 0; i < object_points_length; i++){
      offset += this->st_object_points.deserialize(inbuffer + offset);
        memcpy( &(this->object_points[i]), &(this->st_object_points), sizeof(geometry_msgs::Point));
      }
      uint8_t joint_points_lengthT = *(inbuffer + offset++);
      if(joint_points_lengthT > joint_points_length)
        this->joint_points = (sensor_msgs::JointState*)realloc(this->joint_points, joint_points_lengthT * sizeof(sensor_msgs::JointState));
      offset += 3;
      joint_points_length = joint_points_lengthT;
      for( uint8_t i = 0; i < joint_points_length; i++){
      offset += this->st_joint_points.deserialize(inbuffer + offset);
        memcpy( &(this->joint_points[i]), &(this->st_joint_points), sizeof(sensor_msgs::JointState));
      }
     return offset;
    }

    const char * getType(){ return "calibration_msgs/JointStateCalibrationPattern"; };
    const char * getMD5(){ return "c80e9cf8e7942eba44a6d32e3c75bf59"; };

  };

}
#endif