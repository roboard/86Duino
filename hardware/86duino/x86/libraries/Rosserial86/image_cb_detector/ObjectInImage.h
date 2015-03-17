#ifndef _ROS_image_cb_detector_ObjectInImage_h
#define _ROS_image_cb_detector_ObjectInImage_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Point.h"
#include "image_cb_detector/ImagePoint.h"

namespace image_cb_detector
{

  class ObjectInImage : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t model_points_length;
      geometry_msgs::Point st_model_points;
      geometry_msgs::Point * model_points;
      uint8_t image_points_length;
      image_cb_detector::ImagePoint st_image_points;
      image_cb_detector::ImagePoint * image_points;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = model_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < model_points_length; i++){
      offset += this->model_points[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = image_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < image_points_length; i++){
      offset += this->image_points[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t model_points_lengthT = *(inbuffer + offset++);
      if(model_points_lengthT > model_points_length)
        this->model_points = (geometry_msgs::Point*)realloc(this->model_points, model_points_lengthT * sizeof(geometry_msgs::Point));
      offset += 3;
      model_points_length = model_points_lengthT;
      for( uint8_t i = 0; i < model_points_length; i++){
      offset += this->st_model_points.deserialize(inbuffer + offset);
        memcpy( &(this->model_points[i]), &(this->st_model_points), sizeof(geometry_msgs::Point));
      }
      uint8_t image_points_lengthT = *(inbuffer + offset++);
      if(image_points_lengthT > image_points_length)
        this->image_points = (image_cb_detector::ImagePoint*)realloc(this->image_points, image_points_lengthT * sizeof(image_cb_detector::ImagePoint));
      offset += 3;
      image_points_length = image_points_lengthT;
      for( uint8_t i = 0; i < image_points_length; i++){
      offset += this->st_image_points.deserialize(inbuffer + offset);
        memcpy( &(this->image_points[i]), &(this->st_image_points), sizeof(image_cb_detector::ImagePoint));
      }
     return offset;
    }

    const char * getType(){ return "image_cb_detector/ObjectInImage"; };
    const char * getMD5(){ return "0996b0d8499882526b533fe6e96aa418"; };

  };

}
#endif