#ifndef _ROS_robot_mechanism_controllers_JTCartesianControllerState_h
#define _ROS_robot_mechanism_controllers_JTCartesianControllerState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Twist.h"
#include "geometry_msgs/Wrench.h"
#include "std_msgs/Float64MultiArray.h"

namespace robot_mechanism_controllers
{

  class JTCartesianControllerState : public ros::Msg
  {
    public:
      std_msgs::Header header;
      geometry_msgs::PoseStamped x;
      geometry_msgs::PoseStamped x_desi;
      geometry_msgs::PoseStamped x_desi_filtered;
      geometry_msgs::Twist x_err;
      geometry_msgs::Twist xd;
      geometry_msgs::Twist xd_desi;
      geometry_msgs::Wrench F;
      uint8_t tau_pose_length;
      float st_tau_pose;
      float * tau_pose;
      uint8_t tau_posture_length;
      float st_tau_posture;
      float * tau_posture;
      uint8_t tau_length;
      float st_tau;
      float * tau;
      std_msgs::Float64MultiArray J;
      std_msgs::Float64MultiArray N;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->x.serialize(outbuffer + offset);
      offset += this->x_desi.serialize(outbuffer + offset);
      offset += this->x_desi_filtered.serialize(outbuffer + offset);
      offset += this->x_err.serialize(outbuffer + offset);
      offset += this->xd.serialize(outbuffer + offset);
      offset += this->xd_desi.serialize(outbuffer + offset);
      offset += this->F.serialize(outbuffer + offset);
      *(outbuffer + offset++) = tau_pose_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tau_pose_length; i++){
      int32_t * val_tau_posei = (int32_t *) &(this->tau_pose[i]);
      int32_t exp_tau_posei = (((*val_tau_posei)>>23)&255);
      if(exp_tau_posei != 0)
        exp_tau_posei += 1023-127;
      int32_t sig_tau_posei = *val_tau_posei;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tau_posei<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tau_posei>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tau_posei>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tau_posei<<4) & 0xF0) | ((sig_tau_posei>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tau_posei>>4) & 0x7F;
      if(this->tau_pose[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = tau_posture_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tau_posture_length; i++){
      int32_t * val_tau_posturei = (int32_t *) &(this->tau_posture[i]);
      int32_t exp_tau_posturei = (((*val_tau_posturei)>>23)&255);
      if(exp_tau_posturei != 0)
        exp_tau_posturei += 1023-127;
      int32_t sig_tau_posturei = *val_tau_posturei;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tau_posturei<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tau_posturei>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tau_posturei>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tau_posturei<<4) & 0xF0) | ((sig_tau_posturei>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tau_posturei>>4) & 0x7F;
      if(this->tau_posture[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = tau_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tau_length; i++){
      int32_t * val_taui = (int32_t *) &(this->tau[i]);
      int32_t exp_taui = (((*val_taui)>>23)&255);
      if(exp_taui != 0)
        exp_taui += 1023-127;
      int32_t sig_taui = *val_taui;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_taui<<5) & 0xff;
      *(outbuffer + offset++) = (sig_taui>>3) & 0xff;
      *(outbuffer + offset++) = (sig_taui>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_taui<<4) & 0xF0) | ((sig_taui>>19)&0x0F);
      *(outbuffer + offset++) = (exp_taui>>4) & 0x7F;
      if(this->tau[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      offset += this->J.serialize(outbuffer + offset);
      offset += this->N.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->x.deserialize(inbuffer + offset);
      offset += this->x_desi.deserialize(inbuffer + offset);
      offset += this->x_desi_filtered.deserialize(inbuffer + offset);
      offset += this->x_err.deserialize(inbuffer + offset);
      offset += this->xd.deserialize(inbuffer + offset);
      offset += this->xd_desi.deserialize(inbuffer + offset);
      offset += this->F.deserialize(inbuffer + offset);
      uint8_t tau_pose_lengthT = *(inbuffer + offset++);
      if(tau_pose_lengthT > tau_pose_length)
        this->tau_pose = (float*)realloc(this->tau_pose, tau_pose_lengthT * sizeof(float));
      offset += 3;
      tau_pose_length = tau_pose_lengthT;
      for( uint8_t i = 0; i < tau_pose_length; i++){
      uint32_t * val_st_tau_pose = (uint32_t*) &(this->st_tau_pose);
      offset += 3;
      *val_st_tau_pose = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_tau_pose |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_tau_pose |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_tau_pose |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_tau_pose = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_tau_pose |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_tau_pose !=0)
        *val_st_tau_pose |= ((exp_st_tau_pose)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_tau_pose = -this->st_tau_pose;
        memcpy( &(this->tau_pose[i]), &(this->st_tau_pose), sizeof(float));
      }
      uint8_t tau_posture_lengthT = *(inbuffer + offset++);
      if(tau_posture_lengthT > tau_posture_length)
        this->tau_posture = (float*)realloc(this->tau_posture, tau_posture_lengthT * sizeof(float));
      offset += 3;
      tau_posture_length = tau_posture_lengthT;
      for( uint8_t i = 0; i < tau_posture_length; i++){
      uint32_t * val_st_tau_posture = (uint32_t*) &(this->st_tau_posture);
      offset += 3;
      *val_st_tau_posture = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_tau_posture |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_tau_posture |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_tau_posture |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_tau_posture = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_tau_posture |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_tau_posture !=0)
        *val_st_tau_posture |= ((exp_st_tau_posture)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_tau_posture = -this->st_tau_posture;
        memcpy( &(this->tau_posture[i]), &(this->st_tau_posture), sizeof(float));
      }
      uint8_t tau_lengthT = *(inbuffer + offset++);
      if(tau_lengthT > tau_length)
        this->tau = (float*)realloc(this->tau, tau_lengthT * sizeof(float));
      offset += 3;
      tau_length = tau_lengthT;
      for( uint8_t i = 0; i < tau_length; i++){
      uint32_t * val_st_tau = (uint32_t*) &(this->st_tau);
      offset += 3;
      *val_st_tau = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_tau |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_tau |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_tau |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_tau = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_tau |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_tau !=0)
        *val_st_tau |= ((exp_st_tau)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_tau = -this->st_tau;
        memcpy( &(this->tau[i]), &(this->st_tau), sizeof(float));
      }
      offset += this->J.deserialize(inbuffer + offset);
      offset += this->N.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "robot_mechanism_controllers/JTCartesianControllerState"; };
    const char * getMD5(){ return "6ecdaa599ea0d27643819ae4cd4c43d0"; };

  };

}
#endif