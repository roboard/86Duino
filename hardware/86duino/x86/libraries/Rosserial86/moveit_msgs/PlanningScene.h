#ifndef _ROS_moveit_msgs_PlanningScene_h
#define _ROS_moveit_msgs_PlanningScene_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/RobotState.h"
#include "geometry_msgs/TransformStamped.h"
#include "moveit_msgs/AllowedCollisionMatrix.h"
#include "moveit_msgs/LinkPadding.h"
#include "moveit_msgs/LinkScale.h"
#include "moveit_msgs/ObjectColor.h"
#include "moveit_msgs/PlanningSceneWorld.h"

namespace moveit_msgs
{

  class PlanningScene : public ros::Msg
  {
    public:
      char * name;
      moveit_msgs::RobotState robot_state;
      char * robot_model_name;
      uint8_t fixed_frame_transforms_length;
      geometry_msgs::TransformStamped st_fixed_frame_transforms;
      geometry_msgs::TransformStamped * fixed_frame_transforms;
      moveit_msgs::AllowedCollisionMatrix allowed_collision_matrix;
      uint8_t link_padding_length;
      moveit_msgs::LinkPadding st_link_padding;
      moveit_msgs::LinkPadding * link_padding;
      uint8_t link_scale_length;
      moveit_msgs::LinkScale st_link_scale;
      moveit_msgs::LinkScale * link_scale;
      uint8_t object_colors_length;
      moveit_msgs::ObjectColor st_object_colors;
      moveit_msgs::ObjectColor * object_colors;
      moveit_msgs::PlanningSceneWorld world;
      bool is_diff;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      offset += this->robot_state.serialize(outbuffer + offset);
      uint32_t length_robot_model_name = strlen( (const char*) this->robot_model_name);
      memcpy(outbuffer + offset, &length_robot_model_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->robot_model_name, length_robot_model_name);
      offset += length_robot_model_name;
      *(outbuffer + offset++) = fixed_frame_transforms_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < fixed_frame_transforms_length; i++){
      offset += this->fixed_frame_transforms[i].serialize(outbuffer + offset);
      }
      offset += this->allowed_collision_matrix.serialize(outbuffer + offset);
      *(outbuffer + offset++) = link_padding_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < link_padding_length; i++){
      offset += this->link_padding[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = link_scale_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < link_scale_length; i++){
      offset += this->link_scale[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = object_colors_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < object_colors_length; i++){
      offset += this->object_colors[i].serialize(outbuffer + offset);
      }
      offset += this->world.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_is_diff;
      u_is_diff.real = this->is_diff;
      *(outbuffer + offset + 0) = (u_is_diff.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->is_diff);
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
      offset += this->robot_state.deserialize(inbuffer + offset);
      uint32_t length_robot_model_name;
      memcpy(&length_robot_model_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_robot_model_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_robot_model_name-1]=0;
      this->robot_model_name = (char *)(inbuffer + offset-1);
      offset += length_robot_model_name;
      uint8_t fixed_frame_transforms_lengthT = *(inbuffer + offset++);
      if(fixed_frame_transforms_lengthT > fixed_frame_transforms_length)
        this->fixed_frame_transforms = (geometry_msgs::TransformStamped*)realloc(this->fixed_frame_transforms, fixed_frame_transforms_lengthT * sizeof(geometry_msgs::TransformStamped));
      offset += 3;
      fixed_frame_transforms_length = fixed_frame_transforms_lengthT;
      for( uint8_t i = 0; i < fixed_frame_transforms_length; i++){
      offset += this->st_fixed_frame_transforms.deserialize(inbuffer + offset);
        memcpy( &(this->fixed_frame_transforms[i]), &(this->st_fixed_frame_transforms), sizeof(geometry_msgs::TransformStamped));
      }
      offset += this->allowed_collision_matrix.deserialize(inbuffer + offset);
      uint8_t link_padding_lengthT = *(inbuffer + offset++);
      if(link_padding_lengthT > link_padding_length)
        this->link_padding = (moveit_msgs::LinkPadding*)realloc(this->link_padding, link_padding_lengthT * sizeof(moveit_msgs::LinkPadding));
      offset += 3;
      link_padding_length = link_padding_lengthT;
      for( uint8_t i = 0; i < link_padding_length; i++){
      offset += this->st_link_padding.deserialize(inbuffer + offset);
        memcpy( &(this->link_padding[i]), &(this->st_link_padding), sizeof(moveit_msgs::LinkPadding));
      }
      uint8_t link_scale_lengthT = *(inbuffer + offset++);
      if(link_scale_lengthT > link_scale_length)
        this->link_scale = (moveit_msgs::LinkScale*)realloc(this->link_scale, link_scale_lengthT * sizeof(moveit_msgs::LinkScale));
      offset += 3;
      link_scale_length = link_scale_lengthT;
      for( uint8_t i = 0; i < link_scale_length; i++){
      offset += this->st_link_scale.deserialize(inbuffer + offset);
        memcpy( &(this->link_scale[i]), &(this->st_link_scale), sizeof(moveit_msgs::LinkScale));
      }
      uint8_t object_colors_lengthT = *(inbuffer + offset++);
      if(object_colors_lengthT > object_colors_length)
        this->object_colors = (moveit_msgs::ObjectColor*)realloc(this->object_colors, object_colors_lengthT * sizeof(moveit_msgs::ObjectColor));
      offset += 3;
      object_colors_length = object_colors_lengthT;
      for( uint8_t i = 0; i < object_colors_length; i++){
      offset += this->st_object_colors.deserialize(inbuffer + offset);
        memcpy( &(this->object_colors[i]), &(this->st_object_colors), sizeof(moveit_msgs::ObjectColor));
      }
      offset += this->world.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_is_diff;
      u_is_diff.base = 0;
      u_is_diff.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->is_diff = u_is_diff.real;
      offset += sizeof(this->is_diff);
     return offset;
    }

    const char * getType(){ return "moveit_msgs/PlanningScene"; };
    const char * getMD5(){ return "89aac6d20db967ba716cba5a86b1b9d5"; };

  };

}
#endif