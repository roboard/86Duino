#ifndef _ROS_pr2_plugs_msgs_OutletPoseArray_h
#define _ROS_pr2_plugs_msgs_OutletPoseArray_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/OutletPose.h"

namespace pr2_plugs_msgs
{

  class OutletPoseArray : public ros::Msg
  {
    public:
      uint8_t poses_length;
      pr2_plugs_msgs::OutletPose st_poses;
      pr2_plugs_msgs::OutletPose * poses;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = poses_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < poses_length; i++){
      offset += this->poses[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t poses_lengthT = *(inbuffer + offset++);
      if(poses_lengthT > poses_length)
        this->poses = (pr2_plugs_msgs::OutletPose*)realloc(this->poses, poses_lengthT * sizeof(pr2_plugs_msgs::OutletPose));
      offset += 3;
      poses_length = poses_lengthT;
      for( uint8_t i = 0; i < poses_length; i++){
      offset += this->st_poses.deserialize(inbuffer + offset);
        memcpy( &(this->poses[i]), &(this->st_poses), sizeof(pr2_plugs_msgs::OutletPose));
      }
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/OutletPoseArray"; };
    const char * getMD5(){ return "c7e139d4be72c44607919f5a39000803"; };

  };

}
#endif