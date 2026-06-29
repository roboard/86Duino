#ifndef _ROS_pr2_mechanism_controllers_BaseOdometryState_h
#define _ROS_pr2_mechanism_controllers_BaseOdometryState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Twist.h"

namespace pr2_mechanism_controllers
{

  class BaseOdometryState : public ros::Msg
  {
    public:
      geometry_msgs::Twist velocity;
      uint8_t wheel_link_names_length;
      char* st_wheel_link_names;
      char* * wheel_link_names;
      uint8_t drive_constraint_errors_length;
      float st_drive_constraint_errors;
      float * drive_constraint_errors;
      uint8_t longitudinal_slip_constraint_errors_length;
      float st_longitudinal_slip_constraint_errors;
      float * longitudinal_slip_constraint_errors;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->velocity.serialize(outbuffer + offset);
      *(outbuffer + offset++) = wheel_link_names_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < wheel_link_names_length; i++){
      uint32_t length_wheel_link_namesi = strlen( (const char*) this->wheel_link_names[i]);
      memcpy(outbuffer + offset, &length_wheel_link_namesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->wheel_link_names[i], length_wheel_link_namesi);
      offset += length_wheel_link_namesi;
      }
      *(outbuffer + offset++) = drive_constraint_errors_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < drive_constraint_errors_length; i++){
      int32_t * val_drive_constraint_errorsi = (int32_t *) &(this->drive_constraint_errors[i]);
      int32_t exp_drive_constraint_errorsi = (((*val_drive_constraint_errorsi)>>23)&255);
      if(exp_drive_constraint_errorsi != 0)
        exp_drive_constraint_errorsi += 1023-127;
      int32_t sig_drive_constraint_errorsi = *val_drive_constraint_errorsi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_drive_constraint_errorsi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_drive_constraint_errorsi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_drive_constraint_errorsi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_drive_constraint_errorsi<<4) & 0xF0) | ((sig_drive_constraint_errorsi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_drive_constraint_errorsi>>4) & 0x7F;
      if(this->drive_constraint_errors[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = longitudinal_slip_constraint_errors_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < longitudinal_slip_constraint_errors_length; i++){
      int32_t * val_longitudinal_slip_constraint_errorsi = (int32_t *) &(this->longitudinal_slip_constraint_errors[i]);
      int32_t exp_longitudinal_slip_constraint_errorsi = (((*val_longitudinal_slip_constraint_errorsi)>>23)&255);
      if(exp_longitudinal_slip_constraint_errorsi != 0)
        exp_longitudinal_slip_constraint_errorsi += 1023-127;
      int32_t sig_longitudinal_slip_constraint_errorsi = *val_longitudinal_slip_constraint_errorsi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_longitudinal_slip_constraint_errorsi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_longitudinal_slip_constraint_errorsi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_longitudinal_slip_constraint_errorsi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_longitudinal_slip_constraint_errorsi<<4) & 0xF0) | ((sig_longitudinal_slip_constraint_errorsi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_longitudinal_slip_constraint_errorsi>>4) & 0x7F;
      if(this->longitudinal_slip_constraint_errors[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->velocity.deserialize(inbuffer + offset);
      uint8_t wheel_link_names_lengthT = *(inbuffer + offset++);
      if(wheel_link_names_lengthT > wheel_link_names_length)
        this->wheel_link_names = (char**)realloc(this->wheel_link_names, wheel_link_names_lengthT * sizeof(char*));
      offset += 3;
      wheel_link_names_length = wheel_link_names_lengthT;
      for( uint8_t i = 0; i < wheel_link_names_length; i++){
      uint32_t length_st_wheel_link_names;
      memcpy(&length_st_wheel_link_names, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_wheel_link_names; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_wheel_link_names-1]=0;
      this->st_wheel_link_names = (char *)(inbuffer + offset-1);
      offset += length_st_wheel_link_names;
        memcpy( &(this->wheel_link_names[i]), &(this->st_wheel_link_names), sizeof(char*));
      }
      uint8_t drive_constraint_errors_lengthT = *(inbuffer + offset++);
      if(drive_constraint_errors_lengthT > drive_constraint_errors_length)
        this->drive_constraint_errors = (float*)realloc(this->drive_constraint_errors, drive_constraint_errors_lengthT * sizeof(float));
      offset += 3;
      drive_constraint_errors_length = drive_constraint_errors_lengthT;
      for( uint8_t i = 0; i < drive_constraint_errors_length; i++){
      uint32_t * val_st_drive_constraint_errors = (uint32_t*) &(this->st_drive_constraint_errors);
      offset += 3;
      *val_st_drive_constraint_errors = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_drive_constraint_errors |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_drive_constraint_errors |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_drive_constraint_errors |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_drive_constraint_errors = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_drive_constraint_errors |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_drive_constraint_errors !=0)
        *val_st_drive_constraint_errors |= ((exp_st_drive_constraint_errors)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_drive_constraint_errors = -this->st_drive_constraint_errors;
        memcpy( &(this->drive_constraint_errors[i]), &(this->st_drive_constraint_errors), sizeof(float));
      }
      uint8_t longitudinal_slip_constraint_errors_lengthT = *(inbuffer + offset++);
      if(longitudinal_slip_constraint_errors_lengthT > longitudinal_slip_constraint_errors_length)
        this->longitudinal_slip_constraint_errors = (float*)realloc(this->longitudinal_slip_constraint_errors, longitudinal_slip_constraint_errors_lengthT * sizeof(float));
      offset += 3;
      longitudinal_slip_constraint_errors_length = longitudinal_slip_constraint_errors_lengthT;
      for( uint8_t i = 0; i < longitudinal_slip_constraint_errors_length; i++){
      uint32_t * val_st_longitudinal_slip_constraint_errors = (uint32_t*) &(this->st_longitudinal_slip_constraint_errors);
      offset += 3;
      *val_st_longitudinal_slip_constraint_errors = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_longitudinal_slip_constraint_errors |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_longitudinal_slip_constraint_errors |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_longitudinal_slip_constraint_errors |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_longitudinal_slip_constraint_errors = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_longitudinal_slip_constraint_errors |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_longitudinal_slip_constraint_errors !=0)
        *val_st_longitudinal_slip_constraint_errors |= ((exp_st_longitudinal_slip_constraint_errors)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_longitudinal_slip_constraint_errors = -this->st_longitudinal_slip_constraint_errors;
        memcpy( &(this->longitudinal_slip_constraint_errors[i]), &(this->st_longitudinal_slip_constraint_errors), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_controllers/BaseOdometryState"; };
    const char * getMD5(){ return "8a483e137ebc37abafa4c26549091dd6"; };

  };

}
#endif