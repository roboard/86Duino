#ifndef _ROS_pano_ros_StitchGoal_h
#define _ROS_pano_ros_StitchGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pano_ros
{

  class StitchGoal : public ros::Msg
  {
    public:
      char * bag_file_name;
      char * stitch_file_name;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_bag_file_name = strlen( (const char*) this->bag_file_name);
      memcpy(outbuffer + offset, &length_bag_file_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->bag_file_name, length_bag_file_name);
      offset += length_bag_file_name;
      uint32_t length_stitch_file_name = strlen( (const char*) this->stitch_file_name);
      memcpy(outbuffer + offset, &length_stitch_file_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->stitch_file_name, length_stitch_file_name);
      offset += length_stitch_file_name;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_bag_file_name;
      memcpy(&length_bag_file_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_bag_file_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_bag_file_name-1]=0;
      this->bag_file_name = (char *)(inbuffer + offset-1);
      offset += length_bag_file_name;
      uint32_t length_stitch_file_name;
      memcpy(&length_stitch_file_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_stitch_file_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_stitch_file_name-1]=0;
      this->stitch_file_name = (char *)(inbuffer + offset-1);
      offset += length_stitch_file_name;
     return offset;
    }

    const char * getType(){ return "pano_ros/StitchGoal"; };
    const char * getMD5(){ return "2286405d213866ca1e7b46f37127c016"; };

  };

}
#endif