#ifndef _ROS_pr2_plugs_msgs_VisionOutletDetectionGoal_h
#define _ROS_pr2_plugs_msgs_VisionOutletDetectionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Vector3Stamped.h"

namespace pr2_plugs_msgs
{

  class VisionOutletDetectionGoal : public ros::Msg
  {
    public:
      char * camera_name;
      geometry_msgs::PoseStamped prior;
      geometry_msgs::Vector3Stamped wall_normal;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_camera_name = strlen( (const char*) this->camera_name);
      memcpy(outbuffer + offset, &length_camera_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->camera_name, length_camera_name);
      offset += length_camera_name;
      offset += this->prior.serialize(outbuffer + offset);
      offset += this->wall_normal.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_camera_name;
      memcpy(&length_camera_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_camera_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_camera_name-1]=0;
      this->camera_name = (char *)(inbuffer + offset-1);
      offset += length_camera_name;
      offset += this->prior.deserialize(inbuffer + offset);
      offset += this->wall_normal.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/VisionOutletDetectionGoal"; };
    const char * getMD5(){ return "e01805506b7947845c6db890b01b4876"; };

  };

}
#endif