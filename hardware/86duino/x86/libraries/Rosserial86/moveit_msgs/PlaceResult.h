#ifndef _ROS_moveit_msgs_PlaceResult_h
#define _ROS_moveit_msgs_PlaceResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/MoveItErrorCodes.h"
#include "moveit_msgs/RobotState.h"
#include "moveit_msgs/RobotTrajectory.h"
#include "moveit_msgs/PlaceLocation.h"

namespace moveit_msgs
{

  class PlaceResult : public ros::Msg
  {
    public:
      moveit_msgs::MoveItErrorCodes error_code;
      moveit_msgs::RobotState trajectory_start;
      uint8_t trajectory_stages_length;
      moveit_msgs::RobotTrajectory st_trajectory_stages;
      moveit_msgs::RobotTrajectory * trajectory_stages;
      uint8_t trajectory_descriptions_length;
      char* st_trajectory_descriptions;
      char* * trajectory_descriptions;
      moveit_msgs::PlaceLocation place_location;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->error_code.serialize(outbuffer + offset);
      offset += this->trajectory_start.serialize(outbuffer + offset);
      *(outbuffer + offset++) = trajectory_stages_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < trajectory_stages_length; i++){
      offset += this->trajectory_stages[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = trajectory_descriptions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < trajectory_descriptions_length; i++){
      uint32_t length_trajectory_descriptionsi = strlen( (const char*) this->trajectory_descriptions[i]);
      memcpy(outbuffer + offset, &length_trajectory_descriptionsi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->trajectory_descriptions[i], length_trajectory_descriptionsi);
      offset += length_trajectory_descriptionsi;
      }
      offset += this->place_location.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->error_code.deserialize(inbuffer + offset);
      offset += this->trajectory_start.deserialize(inbuffer + offset);
      uint8_t trajectory_stages_lengthT = *(inbuffer + offset++);
      if(trajectory_stages_lengthT > trajectory_stages_length)
        this->trajectory_stages = (moveit_msgs::RobotTrajectory*)realloc(this->trajectory_stages, trajectory_stages_lengthT * sizeof(moveit_msgs::RobotTrajectory));
      offset += 3;
      trajectory_stages_length = trajectory_stages_lengthT;
      for( uint8_t i = 0; i < trajectory_stages_length; i++){
      offset += this->st_trajectory_stages.deserialize(inbuffer + offset);
        memcpy( &(this->trajectory_stages[i]), &(this->st_trajectory_stages), sizeof(moveit_msgs::RobotTrajectory));
      }
      uint8_t trajectory_descriptions_lengthT = *(inbuffer + offset++);
      if(trajectory_descriptions_lengthT > trajectory_descriptions_length)
        this->trajectory_descriptions = (char**)realloc(this->trajectory_descriptions, trajectory_descriptions_lengthT * sizeof(char*));
      offset += 3;
      trajectory_descriptions_length = trajectory_descriptions_lengthT;
      for( uint8_t i = 0; i < trajectory_descriptions_length; i++){
      uint32_t length_st_trajectory_descriptions;
      memcpy(&length_st_trajectory_descriptions, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_trajectory_descriptions; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_trajectory_descriptions-1]=0;
      this->st_trajectory_descriptions = (char *)(inbuffer + offset-1);
      offset += length_st_trajectory_descriptions;
        memcpy( &(this->trajectory_descriptions[i]), &(this->st_trajectory_descriptions), sizeof(char*));
      }
      offset += this->place_location.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "moveit_msgs/PlaceResult"; };
    const char * getMD5(){ return "da2eea14de05cf0aa280f150a84ded50"; };

  };

}
#endif