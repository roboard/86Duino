#ifndef _ROS_moveit_msgs_Grasp_h
#define _ROS_moveit_msgs_Grasp_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "trajectory_msgs/JointTrajectory.h"
#include "geometry_msgs/PoseStamped.h"
#include "moveit_msgs/GripperTranslation.h"

namespace moveit_msgs
{

  class Grasp : public ros::Msg
  {
    public:
      char * id;
      trajectory_msgs::JointTrajectory pre_grasp_posture;
      trajectory_msgs::JointTrajectory grasp_posture;
      geometry_msgs::PoseStamped grasp_pose;
      float grasp_quality;
      moveit_msgs::GripperTranslation pre_grasp_approach;
      moveit_msgs::GripperTranslation post_grasp_retreat;
      moveit_msgs::GripperTranslation post_place_retreat;
      float max_contact_force;
      uint8_t allowed_touch_objects_length;
      char* st_allowed_touch_objects;
      char* * allowed_touch_objects;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_id = strlen( (const char*) this->id);
      memcpy(outbuffer + offset, &length_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->id, length_id);
      offset += length_id;
      offset += this->pre_grasp_posture.serialize(outbuffer + offset);
      offset += this->grasp_posture.serialize(outbuffer + offset);
      offset += this->grasp_pose.serialize(outbuffer + offset);
      int32_t * val_grasp_quality = (int32_t *) &(this->grasp_quality);
      int32_t exp_grasp_quality = (((*val_grasp_quality)>>23)&255);
      if(exp_grasp_quality != 0)
        exp_grasp_quality += 1023-127;
      int32_t sig_grasp_quality = *val_grasp_quality;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_grasp_quality<<5) & 0xff;
      *(outbuffer + offset++) = (sig_grasp_quality>>3) & 0xff;
      *(outbuffer + offset++) = (sig_grasp_quality>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_grasp_quality<<4) & 0xF0) | ((sig_grasp_quality>>19)&0x0F);
      *(outbuffer + offset++) = (exp_grasp_quality>>4) & 0x7F;
      if(this->grasp_quality < 0) *(outbuffer + offset -1) |= 0x80;
      offset += this->pre_grasp_approach.serialize(outbuffer + offset);
      offset += this->post_grasp_retreat.serialize(outbuffer + offset);
      offset += this->post_place_retreat.serialize(outbuffer + offset);
      union {
        float real;
        uint32_t base;
      } u_max_contact_force;
      u_max_contact_force.real = this->max_contact_force;
      *(outbuffer + offset + 0) = (u_max_contact_force.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_max_contact_force.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_max_contact_force.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_max_contact_force.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->max_contact_force);
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
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_id;
      memcpy(&length_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_id-1]=0;
      this->id = (char *)(inbuffer + offset-1);
      offset += length_id;
      offset += this->pre_grasp_posture.deserialize(inbuffer + offset);
      offset += this->grasp_posture.deserialize(inbuffer + offset);
      offset += this->grasp_pose.deserialize(inbuffer + offset);
      uint32_t * val_grasp_quality = (uint32_t*) &(this->grasp_quality);
      offset += 3;
      *val_grasp_quality = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_grasp_quality |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_grasp_quality |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_grasp_quality |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_grasp_quality = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_grasp_quality |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_grasp_quality !=0)
        *val_grasp_quality |= ((exp_grasp_quality)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->grasp_quality = -this->grasp_quality;
      offset += this->pre_grasp_approach.deserialize(inbuffer + offset);
      offset += this->post_grasp_retreat.deserialize(inbuffer + offset);
      offset += this->post_place_retreat.deserialize(inbuffer + offset);
      union {
        float real;
        uint32_t base;
      } u_max_contact_force;
      u_max_contact_force.base = 0;
      u_max_contact_force.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_max_contact_force.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_max_contact_force.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_max_contact_force.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->max_contact_force = u_max_contact_force.real;
      offset += sizeof(this->max_contact_force);
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
     return offset;
    }

    const char * getType(){ return "moveit_msgs/Grasp"; };
    const char * getMD5(){ return "e26c8fb64f589c33c5d5e54bd7b5e4cb"; };

  };

}
#endif