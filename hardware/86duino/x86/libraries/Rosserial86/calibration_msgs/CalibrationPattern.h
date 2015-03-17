#ifndef _ROS_calibration_msgs_CalibrationPattern_h
#define _ROS_calibration_msgs_CalibrationPattern_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Point.h"

namespace calibration_msgs
{

  class CalibrationPattern : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t object_points_length;
      geometry_msgs::Point st_object_points;
      geometry_msgs::Point * object_points;
      uint8_t image_points_length;
      geometry_msgs::Point st_image_points;
      geometry_msgs::Point * image_points;
      uint8_t success;

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
      *(outbuffer + offset++) = image_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < image_points_length; i++){
      offset += this->image_points[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset + 0) = (this->success >> (8 * 0)) & 0xFF;
      offset += sizeof(this->success);
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
      uint8_t image_points_lengthT = *(inbuffer + offset++);
      if(image_points_lengthT > image_points_length)
        this->image_points = (geometry_msgs::Point*)realloc(this->image_points, image_points_lengthT * sizeof(geometry_msgs::Point));
      offset += 3;
      image_points_length = image_points_lengthT;
      for( uint8_t i = 0; i < image_points_length; i++){
      offset += this->st_image_points.deserialize(inbuffer + offset);
        memcpy( &(this->image_points[i]), &(this->st_image_points), sizeof(geometry_msgs::Point));
      }
      this->success =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->success);
     return offset;
    }

    const char * getType(){ return "calibration_msgs/CalibrationPattern"; };
    const char * getMD5(){ return "5854af5462e19a169f68917c875a6238"; };

  };

}
#endif