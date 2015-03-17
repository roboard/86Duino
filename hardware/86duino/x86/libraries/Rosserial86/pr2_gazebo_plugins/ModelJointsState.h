#ifndef _ROS_pr2_gazebo_plugins_ModelJointsState_h
#define _ROS_pr2_gazebo_plugins_ModelJointsState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Pose.h"

namespace pr2_gazebo_plugins
{

  class ModelJointsState : public ros::Msg
  {
    public:
      uint8_t model_pose_length;
      geometry_msgs::Pose st_model_pose;
      geometry_msgs::Pose * model_pose;
      uint8_t joint_names_length;
      char* st_joint_names;
      char* * joint_names;
      uint8_t joint_positions_length;
      float st_joint_positions;
      float * joint_positions;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = model_pose_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < model_pose_length; i++){
      offset += this->model_pose[i].serialize(outbuffer + offset);
      }
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
      *(outbuffer + offset++) = joint_positions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_positions_length; i++){
      int32_t * val_joint_positionsi = (int32_t *) &(this->joint_positions[i]);
      int32_t exp_joint_positionsi = (((*val_joint_positionsi)>>23)&255);
      if(exp_joint_positionsi != 0)
        exp_joint_positionsi += 1023-127;
      int32_t sig_joint_positionsi = *val_joint_positionsi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_positionsi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_positionsi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_positionsi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_positionsi<<4) & 0xF0) | ((sig_joint_positionsi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_positionsi>>4) & 0x7F;
      if(this->joint_positions[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t model_pose_lengthT = *(inbuffer + offset++);
      if(model_pose_lengthT > model_pose_length)
        this->model_pose = (geometry_msgs::Pose*)realloc(this->model_pose, model_pose_lengthT * sizeof(geometry_msgs::Pose));
      offset += 3;
      model_pose_length = model_pose_lengthT;
      for( uint8_t i = 0; i < model_pose_length; i++){
      offset += this->st_model_pose.deserialize(inbuffer + offset);
        memcpy( &(this->model_pose[i]), &(this->st_model_pose), sizeof(geometry_msgs::Pose));
      }
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
      uint8_t joint_positions_lengthT = *(inbuffer + offset++);
      if(joint_positions_lengthT > joint_positions_length)
        this->joint_positions = (float*)realloc(this->joint_positions, joint_positions_lengthT * sizeof(float));
      offset += 3;
      joint_positions_length = joint_positions_lengthT;
      for( uint8_t i = 0; i < joint_positions_length; i++){
      uint32_t * val_st_joint_positions = (uint32_t*) &(this->st_joint_positions);
      offset += 3;
      *val_st_joint_positions = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_positions |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_positions |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_positions |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_positions = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_positions |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_positions !=0)
        *val_st_joint_positions |= ((exp_st_joint_positions)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_positions = -this->st_joint_positions;
        memcpy( &(this->joint_positions[i]), &(this->st_joint_positions), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "pr2_gazebo_plugins/ModelJointsState"; };
    const char * getMD5(){ return "f700a74958b6566fae4cd77fbb80ffd4"; };

  };

}
#endif