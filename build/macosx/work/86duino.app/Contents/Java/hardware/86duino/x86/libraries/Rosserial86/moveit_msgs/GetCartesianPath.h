#ifndef _ROS_SERVICE_GetCartesianPath_h
#define _ROS_SERVICE_GetCartesianPath_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "moveit_msgs/RobotTrajectory.h"
#include "moveit_msgs/RobotState.h"
#include "moveit_msgs/MoveItErrorCodes.h"
#include "geometry_msgs/Pose.h"
#include "moveit_msgs/Constraints.h"

namespace moveit_msgs
{

static const char GETCARTESIANPATH[] = "moveit_msgs/GetCartesianPath";

  class GetCartesianPathRequest : public ros::Msg
  {
    public:
      std_msgs::Header header;
      moveit_msgs::RobotState start_state;
      char * group_name;
      char * link_name;
      uint8_t waypoints_length;
      geometry_msgs::Pose st_waypoints;
      geometry_msgs::Pose * waypoints;
      float max_step;
      float jump_threshold;
      bool avoid_collisions;
      moveit_msgs::Constraints path_constraints;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->start_state.serialize(outbuffer + offset);
      uint32_t length_group_name = strlen( (const char*) this->group_name);
      memcpy(outbuffer + offset, &length_group_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->group_name, length_group_name);
      offset += length_group_name;
      uint32_t length_link_name = strlen( (const char*) this->link_name);
      memcpy(outbuffer + offset, &length_link_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->link_name, length_link_name);
      offset += length_link_name;
      *(outbuffer + offset++) = waypoints_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < waypoints_length; i++){
      offset += this->waypoints[i].serialize(outbuffer + offset);
      }
      int32_t * val_max_step = (int32_t *) &(this->max_step);
      int32_t exp_max_step = (((*val_max_step)>>23)&255);
      if(exp_max_step != 0)
        exp_max_step += 1023-127;
      int32_t sig_max_step = *val_max_step;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_step<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_step>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_step>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_step<<4) & 0xF0) | ((sig_max_step>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_step>>4) & 0x7F;
      if(this->max_step < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_jump_threshold = (int32_t *) &(this->jump_threshold);
      int32_t exp_jump_threshold = (((*val_jump_threshold)>>23)&255);
      if(exp_jump_threshold != 0)
        exp_jump_threshold += 1023-127;
      int32_t sig_jump_threshold = *val_jump_threshold;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_jump_threshold<<5) & 0xff;
      *(outbuffer + offset++) = (sig_jump_threshold>>3) & 0xff;
      *(outbuffer + offset++) = (sig_jump_threshold>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_jump_threshold<<4) & 0xF0) | ((sig_jump_threshold>>19)&0x0F);
      *(outbuffer + offset++) = (exp_jump_threshold>>4) & 0x7F;
      if(this->jump_threshold < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_avoid_collisions;
      u_avoid_collisions.real = this->avoid_collisions;
      *(outbuffer + offset + 0) = (u_avoid_collisions.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->avoid_collisions);
      offset += this->path_constraints.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->start_state.deserialize(inbuffer + offset);
      uint32_t length_group_name;
      memcpy(&length_group_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_group_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_group_name-1]=0;
      this->group_name = (char *)(inbuffer + offset-1);
      offset += length_group_name;
      uint32_t length_link_name;
      memcpy(&length_link_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_link_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_link_name-1]=0;
      this->link_name = (char *)(inbuffer + offset-1);
      offset += length_link_name;
      uint8_t waypoints_lengthT = *(inbuffer + offset++);
      if(waypoints_lengthT > waypoints_length)
        this->waypoints = (geometry_msgs::Pose*)realloc(this->waypoints, waypoints_lengthT * sizeof(geometry_msgs::Pose));
      offset += 3;
      waypoints_length = waypoints_lengthT;
      for( uint8_t i = 0; i < waypoints_length; i++){
      offset += this->st_waypoints.deserialize(inbuffer + offset);
        memcpy( &(this->waypoints[i]), &(this->st_waypoints), sizeof(geometry_msgs::Pose));
      }
      uint32_t * val_max_step = (uint32_t*) &(this->max_step);
      offset += 3;
      *val_max_step = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_step |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_step |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_step |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_step = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_step |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_step !=0)
        *val_max_step |= ((exp_max_step)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_step = -this->max_step;
      uint32_t * val_jump_threshold = (uint32_t*) &(this->jump_threshold);
      offset += 3;
      *val_jump_threshold = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_jump_threshold |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_jump_threshold |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_jump_threshold |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_jump_threshold = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_jump_threshold |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_jump_threshold !=0)
        *val_jump_threshold |= ((exp_jump_threshold)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->jump_threshold = -this->jump_threshold;
      union {
        bool real;
        uint8_t base;
      } u_avoid_collisions;
      u_avoid_collisions.base = 0;
      u_avoid_collisions.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->avoid_collisions = u_avoid_collisions.real;
      offset += sizeof(this->avoid_collisions);
      offset += this->path_constraints.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETCARTESIANPATH; };
    const char * getMD5(){ return "b37c16ad7ed838d811a270a8054276b6"; };

  };

  class GetCartesianPathResponse : public ros::Msg
  {
    public:
      moveit_msgs::RobotState start_state;
      moveit_msgs::RobotTrajectory solution;
      float fraction;
      moveit_msgs::MoveItErrorCodes error_code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->start_state.serialize(outbuffer + offset);
      offset += this->solution.serialize(outbuffer + offset);
      int32_t * val_fraction = (int32_t *) &(this->fraction);
      int32_t exp_fraction = (((*val_fraction)>>23)&255);
      if(exp_fraction != 0)
        exp_fraction += 1023-127;
      int32_t sig_fraction = *val_fraction;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_fraction<<5) & 0xff;
      *(outbuffer + offset++) = (sig_fraction>>3) & 0xff;
      *(outbuffer + offset++) = (sig_fraction>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_fraction<<4) & 0xF0) | ((sig_fraction>>19)&0x0F);
      *(outbuffer + offset++) = (exp_fraction>>4) & 0x7F;
      if(this->fraction < 0) *(outbuffer + offset -1) |= 0x80;
      offset += this->error_code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->start_state.deserialize(inbuffer + offset);
      offset += this->solution.deserialize(inbuffer + offset);
      uint32_t * val_fraction = (uint32_t*) &(this->fraction);
      offset += 3;
      *val_fraction = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_fraction |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_fraction |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_fraction |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_fraction = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_fraction |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_fraction !=0)
        *val_fraction |= ((exp_fraction)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->fraction = -this->fraction;
      offset += this->error_code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETCARTESIANPATH; };
    const char * getMD5(){ return "45414110461a45eb0e273e013924ce59"; };

  };

  class GetCartesianPath {
    public:
    typedef GetCartesianPathRequest Request;
    typedef GetCartesianPathResponse Response;
  };

}
#endif
