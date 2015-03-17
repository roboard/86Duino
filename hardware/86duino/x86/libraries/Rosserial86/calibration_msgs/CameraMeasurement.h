#ifndef _ROS_calibration_msgs_CameraMeasurement_h
#define _ROS_calibration_msgs_CameraMeasurement_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Point.h"
#include "sensor_msgs/CameraInfo.h"
#include "sensor_msgs/Image.h"
#include "calibration_msgs/CalibrationPattern.h"

namespace calibration_msgs
{

  class CameraMeasurement : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * camera_id;
      uint8_t image_points_length;
      geometry_msgs::Point st_image_points;
      geometry_msgs::Point * image_points;
      sensor_msgs::CameraInfo cam_info;
      bool verbose;
      sensor_msgs::Image image;
      sensor_msgs::Image image_rect;
      calibration_msgs::CalibrationPattern features;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_camera_id = strlen( (const char*) this->camera_id);
      memcpy(outbuffer + offset, &length_camera_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->camera_id, length_camera_id);
      offset += length_camera_id;
      *(outbuffer + offset++) = image_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < image_points_length; i++){
      offset += this->image_points[i].serialize(outbuffer + offset);
      }
      offset += this->cam_info.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_verbose;
      u_verbose.real = this->verbose;
      *(outbuffer + offset + 0) = (u_verbose.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->verbose);
      offset += this->image.serialize(outbuffer + offset);
      offset += this->image_rect.serialize(outbuffer + offset);
      offset += this->features.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_camera_id;
      memcpy(&length_camera_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_camera_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_camera_id-1]=0;
      this->camera_id = (char *)(inbuffer + offset-1);
      offset += length_camera_id;
      uint8_t image_points_lengthT = *(inbuffer + offset++);
      if(image_points_lengthT > image_points_length)
        this->image_points = (geometry_msgs::Point*)realloc(this->image_points, image_points_lengthT * sizeof(geometry_msgs::Point));
      offset += 3;
      image_points_length = image_points_lengthT;
      for( uint8_t i = 0; i < image_points_length; i++){
      offset += this->st_image_points.deserialize(inbuffer + offset);
        memcpy( &(this->image_points[i]), &(this->st_image_points), sizeof(geometry_msgs::Point));
      }
      offset += this->cam_info.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_verbose;
      u_verbose.base = 0;
      u_verbose.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->verbose = u_verbose.real;
      offset += sizeof(this->verbose);
      offset += this->image.deserialize(inbuffer + offset);
      offset += this->image_rect.deserialize(inbuffer + offset);
      offset += this->features.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "calibration_msgs/CameraMeasurement"; };
    const char * getMD5(){ return "f7a0cca96cdd8e17d1424338e086252f"; };

  };

}
#endif