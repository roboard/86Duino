#ifndef _ROS_moveit_msgs_PickupGoal_h
#define _ROS_moveit_msgs_PickupGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/Grasp.h"
#include "moveit_msgs/Constraints.h"
#include "moveit_msgs/PlanningOptions.h"

namespace moveit_msgs
{

  class PickupGoal : public ros::Msg
  {
    public:
      char * target_name;
      char * group_name;
      char * end_effector;
      uint8_t possible_grasps_length;
      moveit_msgs::Grasp st_possible_grasps;
      moveit_msgs::Grasp * possible_grasps;
      char * support_surface_name;
      bool allow_gripper_support_collision;
      uint8_t attached_object_touch_links_length;
      char* st_attached_object_touch_links;
      char* * attached_object_touch_links;
      bool minimize_object_distance;
      moveit_msgs::Constraints path_constraints;
      char * planner_id;
      uint8_t allowed_touch_objects_length;
      char* st_allowed_touch_objects;
      char* * allowed_touch_objects;
      float allowed_planning_time;
      moveit_msgs::PlanningOptions planning_options;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_target_name = strlen( (const char*) this->target_name);
      memcpy(outbuffer + offset, &length_target_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->target_name, length_target_name);
      offset += length_target_name;
      uint32_t length_group_name = strlen( (const char*) this->group_name);
      memcpy(outbuffer + offset, &length_group_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->group_name, length_group_name);
      offset += length_group_name;
      uint32_t length_end_effector = strlen( (const char*) this->end_effector);
      memcpy(outbuffer + offset, &length_end_effector, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->end_effector, length_end_effector);
      offset += length_end_effector;
      *(outbuffer + offset++) = possible_grasps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < possible_grasps_length; i++){
      offset += this->possible_grasps[i].serialize(outbuffer + offset);
      }
      uint32_t length_support_surface_name = strlen( (const char*) this->support_surface_name);
      memcpy(outbuffer + offset, &length_support_surface_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->support_surface_name, length_support_surface_name);
      offset += length_support_surface_name;
      union {
        bool real;
        uint8_t base;
      } u_allow_gripper_support_collision;
      u_allow_gripper_support_collision.real = this->allow_gripper_support_collision;
      *(outbuffer + offset + 0) = (u_allow_gripper_support_collision.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->allow_gripper_support_collision);
      *(outbuffer + offset++) = attached_object_touch_links_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < attached_object_touch_links_length; i++){
      uint32_t length_attached_object_touch_linksi = strlen( (const char*) this->attached_object_touch_links[i]);
      memcpy(outbuffer + offset, &length_attached_object_touch_linksi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->attached_object_touch_links[i], length_attached_object_touch_linksi);
      offset += length_attached_object_touch_linksi;
      }
      union {
        bool real;
        uint8_t base;
      } u_minimize_object_distance;
      u_minimize_object_distance.real = this->minimize_object_distance;
      *(outbuffer + offset + 0) = (u_minimize_object_distance.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->minimize_object_distance);
      offset += this->path_constraints.serialize(outbuffer + offset);
      uint32_t length_planner_id = strlen( (const char*) this->planner_id);
      memcpy(outbuffer + offset, &length_planner_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->planner_id, length_planner_id);
      offset += length_planner_id;
      *(outbuffer + offset++) = allowed_touch_objects_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < allowed_touch_objects_length; i++){
      uint32_t length_allowed_touch_objectsi = strlen( (const char*) this->allowed_touch_objects[i]);
      memcpy(outbuffer + offset, &length_allowed_touch_objectsi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->allowed_touch_objects[i], length_allowed_touch_objectsi);
      offset += length_allowed_touch_objectsi;
      }
      int32_t * val_allowed_planning_time = (int32_t *) &(this->allowed_planning_time);
      int32_t exp_allowed_planning_time = (((*val_allowed_planning_time)>>23)&255);
      if(exp_allowed_planning_time != 0)
        exp_allowed_planning_time += 1023-127;
      int32_t sig_allowed_planning_time = *val_allowed_planning_time;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_allowed_planning_time<<5) & 0xff;
      *(outbuffer + offset++) = (sig_allowed_planning_time>>3) & 0xff;
      *(outbuffer + offset++) = (sig_allowed_planning_time>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_allowed_planning_time<<4) & 0xF0) | ((sig_allowed_planning_time>>19)&0x0F);
      *(outbuffer + offset++) = (exp_allowed_planning_time>>4) & 0x7F;
      if(this->allowed_planning_time < 0) *(outbuffer + offset -1) |= 0x80;
      offset += this->planning_options.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_target_name;
      memcpy(&length_target_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_target_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_target_name-1]=0;
      this->target_name = (char *)(inbuffer + offset-1);
      offset += length_target_name;
      uint32_t length_group_name;
      memcpy(&length_group_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_group_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_group_name-1]=0;
      this->group_name = (char *)(inbuffer + offset-1);
      offset += length_group_name;
      uint32_t length_end_effector;
      memcpy(&length_end_effector, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_end_effector; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_end_effector-1]=0;
      this->end_effector = (char *)(inbuffer + offset-1);
      offset += length_end_effector;
      uint8_t possible_grasps_lengthT = *(inbuffer + offset++);
      if(possible_grasps_lengthT > possible_grasps_length)
        this->possible_grasps = (moveit_msgs::Grasp*)realloc(this->possible_grasps, possible_grasps_lengthT * sizeof(moveit_msgs::Grasp));
      offset += 3;
      possible_grasps_length = possible_grasps_lengthT;
      for( uint8_t i = 0; i < possible_grasps_length; i++){
      offset += this->st_possible_grasps.deserialize(inbuffer + offset);
        memcpy( &(this->possible_grasps[i]), &(this->st_possible_grasps), sizeof(moveit_msgs::Grasp));
      }
      uint32_t length_support_surface_name;
      memcpy(&length_support_surface_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_support_surface_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_support_surface_name-1]=0;
      this->support_surface_name = (char *)(inbuffer + offset-1);
      offset += length_support_surface_name;
      union {
        bool real;
        uint8_t base;
      } u_allow_gripper_support_collision;
      u_allow_gripper_support_collision.base = 0;
      u_allow_gripper_support_collision.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->allow_gripper_support_collision = u_allow_gripper_support_collision.real;
      offset += sizeof(this->allow_gripper_support_collision);
      uint8_t attached_object_touch_links_lengthT = *(inbuffer + offset++);
      if(attached_object_touch_links_lengthT > attached_object_touch_links_length)
        this->attached_object_touch_links = (char**)realloc(this->attached_object_touch_links, attached_object_touch_links_lengthT * sizeof(char*));
      offset += 3;
      attached_object_touch_links_length = attached_object_touch_links_lengthT;
      for( uint8_t i = 0; i < attached_object_touch_links_length; i++){
      uint32_t length_st_attached_object_touch_links;
      memcpy(&length_st_attached_object_touch_links, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_attached_object_touch_links; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_attached_object_touch_links-1]=0;
      this->st_attached_object_touch_links = (char *)(inbuffer + offset-1);
      offset += length_st_attached_object_touch_links;
        memcpy( &(this->attached_object_touch_links[i]), &(this->st_attached_object_touch_links), sizeof(char*));
      }
      union {
        bool real;
        uint8_t base;
      } u_minimize_object_distance;
      u_minimize_object_distance.base = 0;
      u_minimize_object_distance.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->minimize_object_distance = u_minimize_object_distance.real;
      offset += sizeof(this->minimize_object_distance);
      offset += this->path_constraints.deserialize(inbuffer + offset);
      uint32_t length_planner_id;
      memcpy(&length_planner_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_planner_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_planner_id-1]=0;
      this->planner_id = (char *)(inbuffer + offset-1);
      offset += length_planner_id;
      uint8_t allowed_touch_objects_lengthT = *(inbuffer + offset++);
      if(allowed_touch_objects_lengthT > allowed_touch_objects_length)
        this->allowed_touch_objects = (char**)realloc(this->allowed_touch_objects, allowed_touch_objects_lengthT * sizeof(char*));
      offset += 3;
      allowed_touch_objects_length = allowed_touch_objects_lengthT;
      for( uint8_t i = 0; i < allowed_touch_objects_length; i++){
      uint32_t length_st_allowed_touch_objects;
      memcpy(&length_st_allowed_touch_objects, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_allowed_touch_objects; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_allowed_touch_objects-1]=0;
      this->st_allowed_touch_objects = (char *)(inbuffer + offset-1);
      offset += length_st_allowed_touch_objects;
        memcpy( &(this->allowed_touch_objects[i]), &(this->st_allowed_touch_objects), sizeof(char*));
      }
      uint32_t * val_allowed_planning_time = (uint32_t*) &(this->allowed_planning_time);
      offset += 3;
      *val_allowed_planning_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_allowed_planning_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_allowed_planning_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_allowed_planning_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_allowed_planning_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_allowed_planning_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_allowed_planning_time !=0)
        *val_allowed_planning_time |= ((exp_allowed_planning_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->allowed_planning_time = -this->allowed_planning_time;
      offset += this->planning_options.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "moveit_msgs/PickupGoal"; };
    const char * getMD5(){ return "458c6ab3761d73e99b070063f7b74c2a"; };

  };

}
#endif