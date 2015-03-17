#ifndef _ROS_SERVICE_GetPositionFK_h
#define _ROS_SERVICE_GetPositionFK_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/MoveItErrorCodes.h"
#include "geometry_msgs/PoseStamped.h"
#include "std_msgs/Header.h"
#include "moveit_msgs/RobotState.h"

namespace moveit_msgs
{

static const char GETPOSITIONFK[] = "moveit_msgs/GetPositionFK";

  class GetPositionFKRequest : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t fk_link_names_length;
      char* st_fk_link_names;
      char* * fk_link_names;
      moveit_msgs::RobotState robot_state;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = fk_link_names_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < fk_link_names_length; i++){
      uint32_t length_fk_link_namesi = strlen( (const char*) this->fk_link_names[i]);
      memcpy(outbuffer + offset, &length_fk_link_namesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->fk_link_names[i], length_fk_link_namesi);
      offset += length_fk_link_namesi;
      }
      offset += this->robot_state.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t fk_link_names_lengthT = *(inbuffer + offset++);
      if(fk_link_names_lengthT > fk_link_names_length)
        this->fk_link_names = (char**)realloc(this->fk_link_names, fk_link_names_lengthT * sizeof(char*));
      offset += 3;
      fk_link_names_length = fk_link_names_lengthT;
      for( uint8_t i = 0; i < fk_link_names_length; i++){
      uint32_t length_st_fk_link_names;
      memcpy(&length_st_fk_link_names, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_fk_link_names; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_fk_link_names-1]=0;
      this->st_fk_link_names = (char *)(inbuffer + offset-1);
      offset += length_st_fk_link_names;
        memcpy( &(this->fk_link_names[i]), &(this->st_fk_link_names), sizeof(char*));
      }
      offset += this->robot_state.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETPOSITIONFK; };
    const char * getMD5(){ return "1d1ed72044ed56f6246c31b522781797"; };

  };

  class GetPositionFKResponse : public ros::Msg
  {
    public:
      uint8_t pose_stamped_length;
      geometry_msgs::PoseStamped st_pose_stamped;
      geometry_msgs::PoseStamped * pose_stamped;
      uint8_t fk_link_names_length;
      char* st_fk_link_names;
      char* * fk_link_names;
      moveit_msgs::MoveItErrorCodes error_code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = pose_stamped_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < pose_stamped_length; i++){
      offset += this->pose_stamped[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = fk_link_names_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < fk_link_names_length; i++){
      uint32_t length_fk_link_namesi = strlen( (const char*) this->fk_link_names[i]);
      memcpy(outbuffer + offset, &length_fk_link_namesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->fk_link_names[i], length_fk_link_namesi);
      offset += length_fk_link_namesi;
      }
      offset += this->error_code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t pose_stamped_lengthT = *(inbuffer + offset++);
      if(pose_stamped_lengthT > pose_stamped_length)
        this->pose_stamped = (geometry_msgs::PoseStamped*)realloc(this->pose_stamped, pose_stamped_lengthT * sizeof(geometry_msgs::PoseStamped));
      offset += 3;
      pose_stamped_length = pose_stamped_lengthT;
      for( uint8_t i = 0; i < pose_stamped_length; i++){
      offset += this->st_pose_stamped.deserialize(inbuffer + offset);
        memcpy( &(this->pose_stamped[i]), &(this->st_pose_stamped), sizeof(geometry_msgs::PoseStamped));
      }
      uint8_t fk_link_names_lengthT = *(inbuffer + offset++);
      if(fk_link_names_lengthT > fk_link_names_length)
        this->fk_link_names = (char**)realloc(this->fk_link_names, fk_link_names_lengthT * sizeof(char*));
      offset += 3;
      fk_link_names_length = fk_link_names_lengthT;
      for( uint8_t i = 0; i < fk_link_names_length; i++){
      uint32_t length_st_fk_link_names;
      memcpy(&length_st_fk_link_names, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_fk_link_names; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_fk_link_names-1]=0;
      this->st_fk_link_names = (char *)(inbuffer + offset-1);
      offset += length_st_fk_link_names;
        memcpy( &(this->fk_link_names[i]), &(this->st_fk_link_names), sizeof(char*));
      }
      offset += this->error_code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETPOSITIONFK; };
    const char * getMD5(){ return "297215cf4fdfe0008356995ae621dae6"; };

  };

  class GetPositionFK {
    public:
    typedef GetPositionFKRequest Request;
    typedef GetPositionFKResponse Response;
  };

}
#endif
