#ifndef _ROS_pano_ros_PanoCaptureGoal_h
#define _ROS_pano_ros_PanoCaptureGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pano_ros
{

  class PanoCaptureGoal : public ros::Msg
  {
    public:
      char * camera_topic;
      char * bag_filename;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_camera_topic = strlen( (const char*) this->camera_topic);
      memcpy(outbuffer + offset, &length_camera_topic, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->camera_topic, length_camera_topic);
      offset += length_camera_topic;
      uint32_t length_bag_filename = strlen( (const char*) this->bag_filename);
      memcpy(outbuffer + offset, &length_bag_filename, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->bag_filename, length_bag_filename);
      offset += length_bag_filename;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_camera_topic;
      memcpy(&length_camera_topic, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_camera_topic; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_camera_topic-1]=0;
      this->camera_topic = (char *)(inbuffer + offset-1);
      offset += length_camera_topic;
      uint32_t length_bag_filename;
      memcpy(&length_bag_filename, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_bag_filename; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_bag_filename-1]=0;
      this->bag_filename = (char *)(inbuffer + offset-1);
      offset += length_bag_filename;
     return offset;
    }

    const char * getType(){ return "pano_ros/PanoCaptureGoal"; };
    const char * getMD5(){ return "d8d56113630df8b56f39dd99ce661288"; };

  };

}
#endif