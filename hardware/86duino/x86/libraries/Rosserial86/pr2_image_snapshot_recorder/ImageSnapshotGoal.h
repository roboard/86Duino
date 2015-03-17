#ifndef _ROS_pr2_image_snapshot_recorder_ImageSnapshotGoal_h
#define _ROS_pr2_image_snapshot_recorder_ImageSnapshotGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_image_snapshot_recorder
{

  class ImageSnapshotGoal : public ros::Msg
  {
    public:
      char * topic_name;
      int32_t num_images;
      char * output_file_name;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_topic_name = strlen( (const char*) this->topic_name);
      memcpy(outbuffer + offset, &length_topic_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->topic_name, length_topic_name);
      offset += length_topic_name;
      union {
        int32_t real;
        uint32_t base;
      } u_num_images;
      u_num_images.real = this->num_images;
      *(outbuffer + offset + 0) = (u_num_images.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_num_images.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_num_images.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_num_images.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->num_images);
      uint32_t length_output_file_name = strlen( (const char*) this->output_file_name);
      memcpy(outbuffer + offset, &length_output_file_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->output_file_name, length_output_file_name);
      offset += length_output_file_name;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_topic_name;
      memcpy(&length_topic_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_topic_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_topic_name-1]=0;
      this->topic_name = (char *)(inbuffer + offset-1);
      offset += length_topic_name;
      union {
        int32_t real;
        uint32_t base;
      } u_num_images;
      u_num_images.base = 0;
      u_num_images.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_num_images.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_num_images.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_num_images.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->num_images = u_num_images.real;
      offset += sizeof(this->num_images);
      uint32_t length_output_file_name;
      memcpy(&length_output_file_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_output_file_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_output_file_name-1]=0;
      this->output_file_name = (char *)(inbuffer + offset-1);
      offset += length_output_file_name;
     return offset;
    }

    const char * getType(){ return "pr2_image_snapshot_recorder/ImageSnapshotGoal"; };
    const char * getMD5(){ return "13e4842bc6386877d90e53523f6d94ba"; };

  };

}
#endif