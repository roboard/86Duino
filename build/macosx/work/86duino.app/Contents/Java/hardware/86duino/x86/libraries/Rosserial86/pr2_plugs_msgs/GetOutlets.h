#ifndef _ROS_SERVICE_GetOutlets_h
#define _ROS_SERVICE_GetOutlets_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_plugs_msgs/OutletPose.h"

namespace pr2_plugs_msgs
{

static const char GETOUTLETS[] = "pr2_plugs_msgs/GetOutlets";

  class GetOutletsRequest : public ros::Msg
  {
    public:

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return GETOUTLETS; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class GetOutletsResponse : public ros::Msg
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

    const char * getType(){ return GETOUTLETS; };
    const char * getMD5(){ return "c7e139d4be72c44607919f5a39000803"; };

  };

  class GetOutlets {
    public:
    typedef GetOutletsRequest Request;
    typedef GetOutletsResponse Response;
  };

}
#endif
