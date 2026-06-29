#ifndef _ROS_moveit_msgs_KinematicSolverInfo_h
#define _ROS_moveit_msgs_KinematicSolverInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/JointLimits.h"

namespace moveit_msgs
{

  class KinematicSolverInfo : public ros::Msg
  {
    public:
      uint8_t joint_names_length;
      char* st_joint_names;
      char* * joint_names;
      uint8_t limits_length;
      moveit_msgs::JointLimits st_limits;
      moveit_msgs::JointLimits * limits;
      uint8_t link_names_length;
      char* st_link_names;
      char* * link_names;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = joint_names_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_names_length; i++){
      uint32_t length_joint_namesi = strlen( (const char*) this->joint_names[i]);
      memcpy(outbuffer + offset, &length_joint_namesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->joint_names[i], length_joint_namesi);
      offset += length_joint_namesi;
      }
      *(outbuffer + offset++) = limits_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < limits_length; i++){
      offset += this->limits[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = link_names_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < link_names_length; i++){
      uint32_t length_link_namesi = strlen( (const char*) this->link_names[i]);
      memcpy(outbuffer + offset, &length_link_namesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->link_names[i], length_link_namesi);
      offset += length_link_namesi;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t joint_names_lengthT = *(inbuffer + offset++);
      if(joint_names_lengthT > joint_names_length)
        this->joint_names = (char**)realloc(this->joint_names, joint_names_lengthT * sizeof(char*));
      offset += 3;
      joint_names_length = joint_names_lengthT;
      for( uint8_t i = 0; i < joint_names_length; i++){
      uint32_t length_st_joint_names;
      memcpy(&length_st_joint_names, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_joint_names; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_joint_names-1]=0;
      this->st_joint_names = (char *)(inbuffer + offset-1);
      offset += length_st_joint_names;
        memcpy( &(this->joint_names[i]), &(this->st_joint_names), sizeof(char*));
      }
      uint8_t limits_lengthT = *(inbuffer + offset++);
      if(limits_lengthT > limits_length)
        this->limits = (moveit_msgs::JointLimits*)realloc(this->limits, limits_lengthT * sizeof(moveit_msgs::JointLimits));
      offset += 3;
      limits_length = limits_lengthT;
      for( uint8_t i = 0; i < limits_length; i++){
      offset += this->st_limits.deserialize(inbuffer + offset);
        memcpy( &(this->limits[i]), &(this->st_limits), sizeof(moveit_msgs::JointLimits));
      }
      uint8_t link_names_lengthT = *(inbuffer + offset++);
      if(link_names_lengthT > link_names_length)
        this->link_names = (char**)realloc(this->link_names, link_names_lengthT * sizeof(char*));
      offset += 3;
      link_names_length = link_names_lengthT;
      for( uint8_t i = 0; i < link_names_length; i++){
      uint32_t length_st_link_names;
      memcpy(&length_st_link_names, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_link_names; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_link_names-1]=0;
      this->st_link_names = (char *)(inbuffer + offset-1);
      offset += length_st_link_names;
        memcpy( &(this->link_names[i]), &(this->st_link_names), sizeof(char*));
      }
     return offset;
    }

    const char * getType(){ return "moveit_msgs/KinematicSolverInfo"; };
    const char * getMD5(){ return "cc048557c0f9795c392dd80f8bb00489"; };

  };

}
#endif