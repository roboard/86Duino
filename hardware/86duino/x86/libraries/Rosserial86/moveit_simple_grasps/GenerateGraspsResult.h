#ifndef _ROS_moveit_simple_grasps_GenerateGraspsResult_h
#define _ROS_moveit_simple_grasps_GenerateGraspsResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/Grasp.h"

namespace moveit_simple_grasps
{

  class GenerateGraspsResult : public ros::Msg
  {
    public:
      uint8_t grasps_length;
      moveit_msgs::Grasp st_grasps;
      moveit_msgs::Grasp * grasps;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = grasps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < grasps_length; i++){
      offset += this->grasps[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t grasps_lengthT = *(inbuffer + offset++);
      if(grasps_lengthT > grasps_length)
        this->grasps = (moveit_msgs::Grasp*)realloc(this->grasps, grasps_lengthT * sizeof(moveit_msgs::Grasp));
      offset += 3;
      grasps_length = grasps_lengthT;
      for( uint8_t i = 0; i < grasps_length; i++){
      offset += this->st_grasps.deserialize(inbuffer + offset);
        memcpy( &(this->grasps[i]), &(this->st_grasps), sizeof(moveit_msgs::Grasp));
      }
     return offset;
    }

    const char * getType(){ return "moveit_simple_grasps/GenerateGraspsResult"; };
    const char * getMD5(){ return "8c4083a4efa926cd066c287f905843a3"; };

  };

}
#endif