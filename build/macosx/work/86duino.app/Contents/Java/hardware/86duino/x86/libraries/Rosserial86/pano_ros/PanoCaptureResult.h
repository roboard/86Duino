#ifndef _ROS_pano_ros_PanoCaptureResult_h
#define _ROS_pano_ros_PanoCaptureResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pano_ros
{

  class PanoCaptureResult : public ros::Msg
  {
    public:
      uint32_t pano_id;
      uint32_t n_captures;
      char * bag_filename;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->pano_id >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->pano_id >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->pano_id >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->pano_id >> (8 * 3)) & 0xFF;
      offset += sizeof(this->pano_id);
      *(outbuffer + offset + 0) = (this->n_captures >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->n_captures >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->n_captures >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->n_captures >> (8 * 3)) & 0xFF;
      offset += sizeof(this->n_captures);
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
      this->pano_id =  ((uint32_t) (*(inbuffer + offset)));
      this->pano_id |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->pano_id |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->pano_id |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->pano_id);
      this->n_captures =  ((uint32_t) (*(inbuffer + offset)));
      this->n_captures |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->n_captures |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->n_captures |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->n_captures);
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

    const char * getType(){ return "pano_ros/PanoCaptureResult"; };
    const char * getMD5(){ return "7d1ff824dbed21bb16f220c4d06a45fb"; };

  };

}
#endif