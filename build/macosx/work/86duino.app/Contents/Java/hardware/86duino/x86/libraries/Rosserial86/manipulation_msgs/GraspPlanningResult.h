#ifndef _ROS_manipulation_msgs_GraspPlanningResult_h
#define _ROS_manipulation_msgs_GraspPlanningResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "manipulation_msgs/Grasp.h"
#include "manipulation_msgs/GraspPlanningErrorCode.h"

namespace manipulation_msgs
{

  class GraspPlanningResult : public ros::Msg
  {
    public:
      uint8_t grasps_length;
      manipulation_msgs::Grasp st_grasps;
      manipulation_msgs::Grasp * grasps;
      manipulation_msgs::GraspPlanningErrorCode error_code;

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
      offset += this->error_code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t grasps_lengthT = *(inbuffer + offset++);
      if(grasps_lengthT > grasps_length)
        this->grasps = (manipulation_msgs::Grasp*)realloc(this->grasps, grasps_lengthT * sizeof(manipulation_msgs::Grasp));
      offset += 3;
      grasps_length = grasps_lengthT;
      for( uint8_t i = 0; i < grasps_length; i++){
      offset += this->st_grasps.deserialize(inbuffer + offset);
        memcpy( &(this->grasps[i]), &(this->st_grasps), sizeof(manipulation_msgs::Grasp));
      }
      offset += this->error_code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/GraspPlanningResult"; };
    const char * getMD5(){ return "ff7a88c4aec40207164535a24dc9c440"; };

  };

}
#endif