#ifndef _ROS_moveit_msgs_AttachedCollisionObject_h
#define _ROS_moveit_msgs_AttachedCollisionObject_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/CollisionObject.h"
#include "trajectory_msgs/JointTrajectory.h"

namespace moveit_msgs
{

  class AttachedCollisionObject : public ros::Msg
  {
    public:
      char * link_name;
      moveit_msgs::CollisionObject object;
      uint8_t touch_links_length;
      char* st_touch_links;
      char* * touch_links;
      trajectory_msgs::JointTrajectory detach_posture;
      float weight;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_link_name = strlen( (const char*) this->link_name);
      memcpy(outbuffer + offset, &length_link_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->link_name, length_link_name);
      offset += length_link_name;
      offset += this->object.serialize(outbuffer + offset);
      *(outbuffer + offset++) = touch_links_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < touch_links_length; i++){
      uint32_t length_touch_linksi = strlen( (const char*) this->touch_links[i]);
      memcpy(outbuffer + offset, &length_touch_linksi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->touch_links[i], length_touch_linksi);
      offset += length_touch_linksi;
      }
      offset += this->detach_posture.serialize(outbuffer + offset);
      int32_t * val_weight = (int32_t *) &(this->weight);
      int32_t exp_weight = (((*val_weight)>>23)&255);
      if(exp_weight != 0)
        exp_weight += 1023-127;
      int32_t sig_weight = *val_weight;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_weight<<5) & 0xff;
      *(outbuffer + offset++) = (sig_weight>>3) & 0xff;
      *(outbuffer + offset++) = (sig_weight>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_weight<<4) & 0xF0) | ((sig_weight>>19)&0x0F);
      *(outbuffer + offset++) = (exp_weight>>4) & 0x7F;
      if(this->weight < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_link_name;
      memcpy(&length_link_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_link_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_link_name-1]=0;
      this->link_name = (char *)(inbuffer + offset-1);
      offset += length_link_name;
      offset += this->object.deserialize(inbuffer + offset);
      uint8_t touch_links_lengthT = *(inbuffer + offset++);
      if(touch_links_lengthT > touch_links_length)
        this->touch_links = (char**)realloc(this->touch_links, touch_links_lengthT * sizeof(char*));
      offset += 3;
      touch_links_length = touch_links_lengthT;
      for( uint8_t i = 0; i < touch_links_length; i++){
      uint32_t length_st_touch_links;
      memcpy(&length_st_touch_links, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_touch_links; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_touch_links-1]=0;
      this->st_touch_links = (char *)(inbuffer + offset-1);
      offset += length_st_touch_links;
        memcpy( &(this->touch_links[i]), &(this->st_touch_links), sizeof(char*));
      }
      offset += this->detach_posture.deserialize(inbuffer + offset);
      uint32_t * val_weight = (uint32_t*) &(this->weight);
      offset += 3;
      *val_weight = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_weight |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_weight |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_weight |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_weight = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_weight |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_weight !=0)
        *val_weight |= ((exp_weight)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->weight = -this->weight;
     return offset;
    }

    const char * getType(){ return "moveit_msgs/AttachedCollisionObject"; };
    const char * getMD5(){ return "3ceac60b21e85bbd6c5b0ab9d476b752"; };

  };

}
#endif