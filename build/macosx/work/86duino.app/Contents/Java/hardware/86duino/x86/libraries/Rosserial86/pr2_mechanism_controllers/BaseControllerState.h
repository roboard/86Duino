#ifndef _ROS_pr2_mechanism_controllers_BaseControllerState_h
#define _ROS_pr2_mechanism_controllers_BaseControllerState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Twist.h"

namespace pr2_mechanism_controllers
{

  class BaseControllerState : public ros::Msg
  {
    public:
      geometry_msgs::Twist command;
      uint8_t joint_velocity_measured_length;
      float st_joint_velocity_measured;
      float * joint_velocity_measured;
      uint8_t joint_velocity_commanded_length;
      float st_joint_velocity_commanded;
      float * joint_velocity_commanded;
      uint8_t joint_velocity_error_length;
      float st_joint_velocity_error;
      float * joint_velocity_error;
      uint8_t joint_effort_measured_length;
      float st_joint_effort_measured;
      float * joint_effort_measured;
      uint8_t joint_effort_commanded_length;
      float st_joint_effort_commanded;
      float * joint_effort_commanded;
      uint8_t joint_effort_error_length;
      float st_joint_effort_error;
      float * joint_effort_error;
      uint8_t joint_names_length;
      char* st_joint_names;
      char* * joint_names;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->command.serialize(outbuffer + offset);
      *(outbuffer + offset++) = joint_velocity_measured_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_velocity_measured_length; i++){
      int32_t * val_joint_velocity_measuredi = (int32_t *) &(this->joint_velocity_measured[i]);
      int32_t exp_joint_velocity_measuredi = (((*val_joint_velocity_measuredi)>>23)&255);
      if(exp_joint_velocity_measuredi != 0)
        exp_joint_velocity_measuredi += 1023-127;
      int32_t sig_joint_velocity_measuredi = *val_joint_velocity_measuredi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_velocity_measuredi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_velocity_measuredi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_velocity_measuredi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_velocity_measuredi<<4) & 0xF0) | ((sig_joint_velocity_measuredi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_velocity_measuredi>>4) & 0x7F;
      if(this->joint_velocity_measured[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = joint_velocity_commanded_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_velocity_commanded_length; i++){
      int32_t * val_joint_velocity_commandedi = (int32_t *) &(this->joint_velocity_commanded[i]);
      int32_t exp_joint_velocity_commandedi = (((*val_joint_velocity_commandedi)>>23)&255);
      if(exp_joint_velocity_commandedi != 0)
        exp_joint_velocity_commandedi += 1023-127;
      int32_t sig_joint_velocity_commandedi = *val_joint_velocity_commandedi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_velocity_commandedi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_velocity_commandedi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_velocity_commandedi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_velocity_commandedi<<4) & 0xF0) | ((sig_joint_velocity_commandedi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_velocity_commandedi>>4) & 0x7F;
      if(this->joint_velocity_commanded[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = joint_velocity_error_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_velocity_error_length; i++){
      int32_t * val_joint_velocity_errori = (int32_t *) &(this->joint_velocity_error[i]);
      int32_t exp_joint_velocity_errori = (((*val_joint_velocity_errori)>>23)&255);
      if(exp_joint_velocity_errori != 0)
        exp_joint_velocity_errori += 1023-127;
      int32_t sig_joint_velocity_errori = *val_joint_velocity_errori;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_velocity_errori<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_velocity_errori>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_velocity_errori>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_velocity_errori<<4) & 0xF0) | ((sig_joint_velocity_errori>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_velocity_errori>>4) & 0x7F;
      if(this->joint_velocity_error[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = joint_effort_measured_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_effort_measured_length; i++){
      int32_t * val_joint_effort_measuredi = (int32_t *) &(this->joint_effort_measured[i]);
      int32_t exp_joint_effort_measuredi = (((*val_joint_effort_measuredi)>>23)&255);
      if(exp_joint_effort_measuredi != 0)
        exp_joint_effort_measuredi += 1023-127;
      int32_t sig_joint_effort_measuredi = *val_joint_effort_measuredi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_effort_measuredi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_effort_measuredi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_effort_measuredi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_effort_measuredi<<4) & 0xF0) | ((sig_joint_effort_measuredi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_effort_measuredi>>4) & 0x7F;
      if(this->joint_effort_measured[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = joint_effort_commanded_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_effort_commanded_length; i++){
      int32_t * val_joint_effort_commandedi = (int32_t *) &(this->joint_effort_commanded[i]);
      int32_t exp_joint_effort_commandedi = (((*val_joint_effort_commandedi)>>23)&255);
      if(exp_joint_effort_commandedi != 0)
        exp_joint_effort_commandedi += 1023-127;
      int32_t sig_joint_effort_commandedi = *val_joint_effort_commandedi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_effort_commandedi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_effort_commandedi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_effort_commandedi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_effort_commandedi<<4) & 0xF0) | ((sig_joint_effort_commandedi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_effort_commandedi>>4) & 0x7F;
      if(this->joint_effort_commanded[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = joint_effort_error_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_effort_error_length; i++){
      int32_t * val_joint_effort_errori = (int32_t *) &(this->joint_effort_error[i]);
      int32_t exp_joint_effort_errori = (((*val_joint_effort_errori)>>23)&255);
      if(exp_joint_effort_errori != 0)
        exp_joint_effort_errori += 1023-127;
      int32_t sig_joint_effort_errori = *val_joint_effort_errori;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_effort_errori<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_effort_errori>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_effort_errori>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_effort_errori<<4) & 0xF0) | ((sig_joint_effort_errori>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_effort_errori>>4) & 0x7F;
      if(this->joint_effort_error[i] < 0) *(outbuffer + offset -1) |= 0x80;
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
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->command.deserialize(inbuffer + offset);
      uint8_t joint_velocity_measured_lengthT = *(inbuffer + offset++);
      if(joint_velocity_measured_lengthT > joint_velocity_measured_length)
        this->joint_velocity_measured = (float*)realloc(this->joint_velocity_measured, joint_velocity_measured_lengthT * sizeof(float));
      offset += 3;
      joint_velocity_measured_length = joint_velocity_measured_lengthT;
      for( uint8_t i = 0; i < joint_velocity_measured_length; i++){
      uint32_t * val_st_joint_velocity_measured = (uint32_t*) &(this->st_joint_velocity_measured);
      offset += 3;
      *val_st_joint_velocity_measured = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_velocity_measured |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_velocity_measured |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_velocity_measured |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_velocity_measured = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_velocity_measured |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_velocity_measured !=0)
        *val_st_joint_velocity_measured |= ((exp_st_joint_velocity_measured)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_velocity_measured = -this->st_joint_velocity_measured;
        memcpy( &(this->joint_velocity_measured[i]), &(this->st_joint_velocity_measured), sizeof(float));
      }
      uint8_t joint_velocity_commanded_lengthT = *(inbuffer + offset++);
      if(joint_velocity_commanded_lengthT > joint_velocity_commanded_length)
        this->joint_velocity_commanded = (float*)realloc(this->joint_velocity_commanded, joint_velocity_commanded_lengthT * sizeof(float));
      offset += 3;
      joint_velocity_commanded_length = joint_velocity_commanded_lengthT;
      for( uint8_t i = 0; i < joint_velocity_commanded_length; i++){
      uint32_t * val_st_joint_velocity_commanded = (uint32_t*) &(this->st_joint_velocity_commanded);
      offset += 3;
      *val_st_joint_velocity_commanded = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_velocity_commanded |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_velocity_commanded |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_velocity_commanded |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_velocity_commanded = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_velocity_commanded |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_velocity_commanded !=0)
        *val_st_joint_velocity_commanded |= ((exp_st_joint_velocity_commanded)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_velocity_commanded = -this->st_joint_velocity_commanded;
        memcpy( &(this->joint_velocity_commanded[i]), &(this->st_joint_velocity_commanded), sizeof(float));
      }
      uint8_t joint_velocity_error_lengthT = *(inbuffer + offset++);
      if(joint_velocity_error_lengthT > joint_velocity_error_length)
        this->joint_velocity_error = (float*)realloc(this->joint_velocity_error, joint_velocity_error_lengthT * sizeof(float));
      offset += 3;
      joint_velocity_error_length = joint_velocity_error_lengthT;
      for( uint8_t i = 0; i < joint_velocity_error_length; i++){
      uint32_t * val_st_joint_velocity_error = (uint32_t*) &(this->st_joint_velocity_error);
      offset += 3;
      *val_st_joint_velocity_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_velocity_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_velocity_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_velocity_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_velocity_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_velocity_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_velocity_error !=0)
        *val_st_joint_velocity_error |= ((exp_st_joint_velocity_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_velocity_error = -this->st_joint_velocity_error;
        memcpy( &(this->joint_velocity_error[i]), &(this->st_joint_velocity_error), sizeof(float));
      }
      uint8_t joint_effort_measured_lengthT = *(inbuffer + offset++);
      if(joint_effort_measured_lengthT > joint_effort_measured_length)
        this->joint_effort_measured = (float*)realloc(this->joint_effort_measured, joint_effort_measured_lengthT * sizeof(float));
      offset += 3;
      joint_effort_measured_length = joint_effort_measured_lengthT;
      for( uint8_t i = 0; i < joint_effort_measured_length; i++){
      uint32_t * val_st_joint_effort_measured = (uint32_t*) &(this->st_joint_effort_measured);
      offset += 3;
      *val_st_joint_effort_measured = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_effort_measured |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_effort_measured |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_effort_measured |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_effort_measured = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_effort_measured |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_effort_measured !=0)
        *val_st_joint_effort_measured |= ((exp_st_joint_effort_measured)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_effort_measured = -this->st_joint_effort_measured;
        memcpy( &(this->joint_effort_measured[i]), &(this->st_joint_effort_measured), sizeof(float));
      }
      uint8_t joint_effort_commanded_lengthT = *(inbuffer + offset++);
      if(joint_effort_commanded_lengthT > joint_effort_commanded_length)
        this->joint_effort_commanded = (float*)realloc(this->joint_effort_commanded, joint_effort_commanded_lengthT * sizeof(float));
      offset += 3;
      joint_effort_commanded_length = joint_effort_commanded_lengthT;
      for( uint8_t i = 0; i < joint_effort_commanded_length; i++){
      uint32_t * val_st_joint_effort_commanded = (uint32_t*) &(this->st_joint_effort_commanded);
      offset += 3;
      *val_st_joint_effort_commanded = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_effort_commanded |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_effort_commanded |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_effort_commanded |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_effort_commanded = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_effort_commanded |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_effort_commanded !=0)
        *val_st_joint_effort_commanded |= ((exp_st_joint_effort_commanded)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_effort_commanded = -this->st_joint_effort_commanded;
        memcpy( &(this->joint_effort_commanded[i]), &(this->st_joint_effort_commanded), sizeof(float));
      }
      uint8_t joint_effort_error_lengthT = *(inbuffer + offset++);
      if(joint_effort_error_lengthT > joint_effort_error_length)
        this->joint_effort_error = (float*)realloc(this->joint_effort_error, joint_effort_error_lengthT * sizeof(float));
      offset += 3;
      joint_effort_error_length = joint_effort_error_lengthT;
      for( uint8_t i = 0; i < joint_effort_error_length; i++){
      uint32_t * val_st_joint_effort_error = (uint32_t*) &(this->st_joint_effort_error);
      offset += 3;
      *val_st_joint_effort_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_effort_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_effort_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_effort_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_effort_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_effort_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_effort_error !=0)
        *val_st_joint_effort_error |= ((exp_st_joint_effort_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_effort_error = -this->st_joint_effort_error;
        memcpy( &(this->joint_effort_error[i]), &(this->st_joint_effort_error), sizeof(float));
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
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_controllers/BaseControllerState"; };
    const char * getMD5(){ return "7a488aa492f9175d5fa35e22e56c4b28"; };

  };

}
#endif