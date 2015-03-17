#ifndef _ROS_manipulation_msgs_SceneRegion_h
#define _ROS_manipulation_msgs_SceneRegion_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "sensor_msgs/PointCloud2.h"
#include "sensor_msgs/Image.h"
#include "sensor_msgs/CameraInfo.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Vector3.h"

namespace manipulation_msgs
{

  class SceneRegion : public ros::Msg
  {
    public:
      sensor_msgs::PointCloud2 cloud;
      uint8_t mask_length;
      int32_t st_mask;
      int32_t * mask;
      sensor_msgs::Image image;
      sensor_msgs::Image disparity_image;
      sensor_msgs::CameraInfo cam_info;
      geometry_msgs::PoseStamped roi_box_pose;
      geometry_msgs::Vector3 roi_box_dims;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->cloud.serialize(outbuffer + offset);
      *(outbuffer + offset++) = mask_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < mask_length; i++){
      union {
        int32_t real;
        uint32_t base;
      } u_maski;
      u_maski.real = this->mask[i];
      *(outbuffer + offset + 0) = (u_maski.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_maski.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_maski.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_maski.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->mask[i]);
      }
      offset += this->image.serialize(outbuffer + offset);
      offset += this->disparity_image.serialize(outbuffer + offset);
      offset += this->cam_info.serialize(outbuffer + offset);
      offset += this->roi_box_pose.serialize(outbuffer + offset);
      offset += this->roi_box_dims.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->cloud.deserialize(inbuffer + offset);
      uint8_t mask_lengthT = *(inbuffer + offset++);
      if(mask_lengthT > mask_length)
        this->mask = (int32_t*)realloc(this->mask, mask_lengthT * sizeof(int32_t));
      offset += 3;
      mask_length = mask_lengthT;
      for( uint8_t i = 0; i < mask_length; i++){
      union {
        int32_t real;
        uint32_t base;
      } u_st_mask;
      u_st_mask.base = 0;
      u_st_mask.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_mask.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_mask.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_mask.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_mask = u_st_mask.real;
      offset += sizeof(this->st_mask);
        memcpy( &(this->mask[i]), &(this->st_mask), sizeof(int32_t));
      }
      offset += this->image.deserialize(inbuffer + offset);
      offset += this->disparity_image.deserialize(inbuffer + offset);
      offset += this->cam_info.deserialize(inbuffer + offset);
      offset += this->roi_box_pose.deserialize(inbuffer + offset);
      offset += this->roi_box_dims.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/SceneRegion"; };
    const char * getMD5(){ return "0a9ab02b19292633619876c9d247690c"; };

  };

}
#endif