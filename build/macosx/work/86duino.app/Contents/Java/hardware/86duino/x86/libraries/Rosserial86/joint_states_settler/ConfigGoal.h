#ifndef _ROS_joint_states_settler_ConfigGoal_h
#define _ROS_joint_states_settler_ConfigGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/duration.h"

namespace joint_states_settler
{

  class ConfigGoal : public ros::Msg
  {
    public:
      uint8_t joint_names_length;
      char* st_joint_names;
      char* * joint_names;
      uint8_t tolerances_length;
      float st_tolerances;
      float * tolerances;
      ros::Duration max_step;
      uint32_t cache_size;

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
      *(outbuffer + offset++) = tolerances_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tolerances_length; i++){
      int32_t * val_tolerancesi = (int32_t *) &(this->tolerances[i]);
      int32_t exp_tolerancesi = (((*val_tolerancesi)>>23)&255);
      if(exp_tolerancesi != 0)
        exp_tolerancesi += 1023-127;
      int32_t sig_tolerancesi = *val_tolerancesi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tolerancesi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tolerancesi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tolerancesi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tolerancesi<<4) & 0xF0) | ((sig_tolerancesi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tolerancesi>>4) & 0x7F;
      if(this->tolerances[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset + 0) = (this->max_step.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->max_step.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->max_step.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->max_step.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->max_step.sec);
      *(outbuffer + offset + 0) = (this->max_step.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->max_step.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->max_step.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->max_step.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->max_step.nsec);
      *(outbuffer + offset + 0) = (this->cache_size >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->cache_size >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->cache_size >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->cache_size >> (8 * 3)) & 0xFF;
      offset += sizeof(this->cache_size);
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
      uint8_t tolerances_lengthT = *(inbuffer + offset++);
      if(tolerances_lengthT > tolerances_length)
        this->tolerances = (float*)realloc(this->tolerances, tolerances_lengthT * sizeof(float));
      offset += 3;
      tolerances_length = tolerances_lengthT;
      for( uint8_t i = 0; i < tolerances_length; i++){
      uint32_t * val_st_tolerances = (uint32_t*) &(this->st_tolerances);
      offset += 3;
      *val_st_tolerances = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_tolerances |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_tolerances |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_tolerances |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_tolerances = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_tolerances |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_tolerances !=0)
        *val_st_tolerances |= ((exp_st_tolerances)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_tolerances = -this->st_tolerances;
        memcpy( &(this->tolerances[i]), &(this->st_tolerances), sizeof(float));
      }
      this->max_step.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->max_step.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->max_step.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->max_step.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->max_step.sec);
      this->max_step.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->max_step.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->max_step.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->max_step.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->max_step.nsec);
      this->cache_size =  ((uint32_t) (*(inbuffer + offset)));
      this->cache_size |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->cache_size |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->cache_size |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->cache_size);
     return offset;
    }

    const char * getType(){ return "joint_states_settler/ConfigGoal"; };
    const char * getMD5(){ return "863b25359077f9aa231257b3d1612b63"; };

  };

}
#endif