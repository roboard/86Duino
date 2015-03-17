#ifndef _ROS_calibration_msgs_DenseLaserObjectFeatures_h
#define _ROS_calibration_msgs_DenseLaserObjectFeatures_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "calibration_msgs/DenseLaserPoint.h"
#include "geometry_msgs/Point.h"

namespace calibration_msgs
{

  class DenseLaserObjectFeatures : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t dense_laser_points_length;
      calibration_msgs::DenseLaserPoint st_dense_laser_points;
      calibration_msgs::DenseLaserPoint * dense_laser_points;
      uint8_t object_points_length;
      geometry_msgs::Point st_object_points;
      geometry_msgs::Point * object_points;
      uint8_t success;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = dense_laser_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < dense_laser_points_length; i++){
      offset += this->dense_laser_points[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = object_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < object_points_length; i++){
      offset += this->object_points[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset + 0) = (this->success >> (8 * 0)) & 0xFF;
      offset += sizeof(this->success);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t dense_laser_points_lengthT = *(inbuffer + offset++);
      if(dense_laser_points_lengthT > dense_laser_points_length)
        this->dense_laser_points = (calibration_msgs::DenseLaserPoint*)realloc(this->dense_laser_points, dense_laser_points_lengthT * sizeof(calibration_msgs::DenseLaserPoint));
      offset += 3;
      dense_laser_points_length = dense_laser_points_lengthT;
      for( uint8_t i = 0; i < dense_laser_points_length; i++){
      offset += this->st_dense_laser_points.deserialize(inbuffer + offset);
        memcpy( &(this->dense_laser_points[i]), &(this->st_dense_laser_points), sizeof(calibration_msgs::DenseLaserPoint));
      }
      uint8_t object_points_lengthT = *(inbuffer + offset++);
      if(object_points_lengthT > object_points_length)
        this->object_points = (geometry_msgs::Point*)realloc(this->object_points, object_points_lengthT * sizeof(geometry_msgs::Point));
      offset += 3;
      object_points_length = object_points_lengthT;
      for( uint8_t i = 0; i < object_points_length; i++){
      offset += this->st_object_points.deserialize(inbuffer + offset);
        memcpy( &(this->object_points[i]), &(this->st_object_points), sizeof(geometry_msgs::Point));
      }
      this->success =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->success);
     return offset;
    }

    const char * getType(){ return "calibration_msgs/DenseLaserObjectFeatures"; };
    const char * getMD5(){ return "b642d46e47d54e00f98a3d98b02b5cc6"; };

  };

}
#endif