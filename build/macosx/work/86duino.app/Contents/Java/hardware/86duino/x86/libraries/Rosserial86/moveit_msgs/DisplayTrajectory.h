#ifndef _ROS_moveit_msgs_DisplayTrajectory_h
#define _ROS_moveit_msgs_DisplayTrajectory_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/RobotTrajectory.h"
#include "moveit_msgs/RobotState.h"

namespace moveit_msgs
{

  class DisplayTrajectory : public ros::Msg
  {
    public:
      char * model_id;
      uint8_t trajectory_length;
      moveit_msgs::RobotTrajectory st_trajectory;
      moveit_msgs::RobotTrajectory * trajectory;
      moveit_msgs::RobotState trajectory_start;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_model_id = strlen( (const char*) this->model_id);
      memcpy(outbuffer + offset, &length_model_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->model_id, length_model_id);
      offset += length_model_id;
      *(outbuffer + offset++) = trajectory_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < trajectory_length; i++){
      offset += this->trajectory[i].serialize(outbuffer + offset);
      }
      offset += this->trajectory_start.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_model_id;
      memcpy(&length_model_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_model_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_model_id-1]=0;
      this->model_id = (char *)(inbuffer + offset-1);
      offset += length_model_id;
      uint8_t trajectory_lengthT = *(inbuffer + offset++);
      if(trajectory_lengthT > trajectory_length)
        this->trajectory = (moveit_msgs::RobotTrajectory*)realloc(this->trajectory, trajectory_lengthT * sizeof(moveit_msgs::RobotTrajectory));
      offset += 3;
      trajectory_length = trajectory_lengthT;
      for( uint8_t i = 0; i < trajectory_length; i++){
      offset += this->st_trajectory.deserialize(inbuffer + offset);
        memcpy( &(this->trajectory[i]), &(this->st_trajectory), sizeof(moveit_msgs::RobotTrajectory));
      }
      offset += this->trajectory_start.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "moveit_msgs/DisplayTrajectory"; };
    const char * getMD5(){ return "c3c039261ab9e8a11457dac56b6316c8"; };

  };

}
#endif