#ifndef _ROS_moveit_msgs_PlaceGoal_h
#define _ROS_moveit_msgs_PlaceGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/PlaceLocation.h"
#include "moveit_msgs/Constraints.h"
#include "moveit_msgs/PlanningOptions.h"

namespace moveit_msgs
{

  class PlaceGoal : public ros::Msg
  {
    public:
      char * group_name;
      char * attached_object_name;
      uint8_t place_locations_length;
      moveit_msgs::PlaceLocation st_place_locations;
      moveit_msgs::PlaceLocation * place_locations;
      bool place_eef;
      char * support_surface_name;
      bool allow_gripper_support_collision;
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
      uint32_t length_group_name = strlen( (const char*) this->group_name);
      memcpy(outbuffer + offset, &length_group_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->group_name, length_group_name);
      offset += length_group_name;
      uint32_t length_attached_object_name = strlen( (const char*) this->attached_object_name);
      memcpy(outbuffer + offset, &length_attached_object_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->attached_object_name, length_attached_object_name);
      offset += length_attached_object_name;
      *(outbuffer + offset++) = place_locations_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < place_locations_length; i++){
      offset += this->place_locations[i].serialize(outbuffer + offset);
      }
      union {
        bool real;
        uint8_t base;
      } u_place_eef;
      u_place_eef.real = this->place_eef;
      *(outbuffer + offset + 0) = (u_place_eef.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->place_eef);
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
      uint32_t length_group_name;
      memcpy(&length_group_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_group_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_group_name-1]=0;
      this->group_name = (char *)(inbuffer + offset-1);
      offset += length_group_name;
      uint32_t length_attached_object_name;
      memcpy(&length_attached_object_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_attached_object_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_attached_object_name-1]=0;
      this->attached_object_name = (char *)(inbuffer + offset-1);
      offset += length_attached_object_name;
      uint8_t place_locations_lengthT = *(inbuffer + offset++);
      if(place_locations_lengthT > place_locations_length)
        this->place_locations = (moveit_msgs::PlaceLocation*)realloc(this->place_locations, place_locations_lengthT * sizeof(moveit_msgs::PlaceLocation));
      offset += 3;
      place_locations_length = place_locations_lengthT;
      for( uint8_t i = 0; i < place_locations_length; i++){
      offset += this->st_place_locations.deserialize(inbuffer + offset);
        memcpy( &(this->place_locations[i]), &(this->st_place_locations), sizeof(moveit_msgs::PlaceLocation));
      }
      union {
        bool real;
        uint8_t base;
      } u_place_eef;
      u_place_eef.base = 0;
      u_place_eef.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->place_eef = u_place_eef.real;
      offset += sizeof(this->place_eef);
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

    const char * getType(){ return "moveit_msgs/PlaceGoal"; };
    const char * getMD5(){ return "e3f3e956e536ccd313fd8f23023f0a94"; };

  };

}
#endif