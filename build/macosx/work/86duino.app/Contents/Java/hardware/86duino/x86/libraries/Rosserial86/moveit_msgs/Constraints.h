#ifndef _ROS_moveit_msgs_Constraints_h
#define _ROS_moveit_msgs_Constraints_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/JointConstraint.h"
#include "moveit_msgs/PositionConstraint.h"
#include "moveit_msgs/OrientationConstraint.h"
#include "moveit_msgs/VisibilityConstraint.h"

namespace moveit_msgs
{

  class Constraints : public ros::Msg
  {
    public:
      char * name;
      uint8_t joint_constraints_length;
      moveit_msgs::JointConstraint st_joint_constraints;
      moveit_msgs::JointConstraint * joint_constraints;
      uint8_t position_constraints_length;
      moveit_msgs::PositionConstraint st_position_constraints;
      moveit_msgs::PositionConstraint * position_constraints;
      uint8_t orientation_constraints_length;
      moveit_msgs::OrientationConstraint st_orientation_constraints;
      moveit_msgs::OrientationConstraint * orientation_constraints;
      uint8_t visibility_constraints_length;
      moveit_msgs::VisibilityConstraint st_visibility_constraints;
      moveit_msgs::VisibilityConstraint * visibility_constraints;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      *(outbuffer + offset++) = joint_constraints_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_constraints_length; i++){
      offset += this->joint_constraints[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = position_constraints_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < position_constraints_length; i++){
      offset += this->position_constraints[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = orientation_constraints_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < orientation_constraints_length; i++){
      offset += this->orientation_constraints[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = visibility_constraints_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < visibility_constraints_length; i++){
      offset += this->visibility_constraints[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      uint8_t joint_constraints_lengthT = *(inbuffer + offset++);
      if(joint_constraints_lengthT > joint_constraints_length)
        this->joint_constraints = (moveit_msgs::JointConstraint*)realloc(this->joint_constraints, joint_constraints_lengthT * sizeof(moveit_msgs::JointConstraint));
      offset += 3;
      joint_constraints_length = joint_constraints_lengthT;
      for( uint8_t i = 0; i < joint_constraints_length; i++){
      offset += this->st_joint_constraints.deserialize(inbuffer + offset);
        memcpy( &(this->joint_constraints[i]), &(this->st_joint_constraints), sizeof(moveit_msgs::JointConstraint));
      }
      uint8_t position_constraints_lengthT = *(inbuffer + offset++);
      if(position_constraints_lengthT > position_constraints_length)
        this->position_constraints = (moveit_msgs::PositionConstraint*)realloc(this->position_constraints, position_constraints_lengthT * sizeof(moveit_msgs::PositionConstraint));
      offset += 3;
      position_constraints_length = position_constraints_lengthT;
      for( uint8_t i = 0; i < position_constraints_length; i++){
      offset += this->st_position_constraints.deserialize(inbuffer + offset);
        memcpy( &(this->position_constraints[i]), &(this->st_position_constraints), sizeof(moveit_msgs::PositionConstraint));
      }
      uint8_t orientation_constraints_lengthT = *(inbuffer + offset++);
      if(orientation_constraints_lengthT > orientation_constraints_length)
        this->orientation_constraints = (moveit_msgs::OrientationConstraint*)realloc(this->orientation_constraints, orientation_constraints_lengthT * sizeof(moveit_msgs::OrientationConstraint));
      offset += 3;
      orientation_constraints_length = orientation_constraints_lengthT;
      for( uint8_t i = 0; i < orientation_constraints_length; i++){
      offset += this->st_orientation_constraints.deserialize(inbuffer + offset);
        memcpy( &(this->orientation_constraints[i]), &(this->st_orientation_constraints), sizeof(moveit_msgs::OrientationConstraint));
      }
      uint8_t visibility_constraints_lengthT = *(inbuffer + offset++);
      if(visibility_constraints_lengthT > visibility_constraints_length)
        this->visibility_constraints = (moveit_msgs::VisibilityConstraint*)realloc(this->visibility_constraints, visibility_constraints_lengthT * sizeof(moveit_msgs::VisibilityConstraint));
      offset += 3;
      visibility_constraints_length = visibility_constraints_lengthT;
      for( uint8_t i = 0; i < visibility_constraints_length; i++){
      offset += this->st_visibility_constraints.deserialize(inbuffer + offset);
        memcpy( &(this->visibility_constraints[i]), &(this->st_visibility_constraints), sizeof(moveit_msgs::VisibilityConstraint));
      }
     return offset;
    }

    const char * getType(){ return "moveit_msgs/Constraints"; };
    const char * getMD5(){ return "8d5ce8d34ef26c65fb5d43c9e99bf6e0"; };

  };

}
#endif