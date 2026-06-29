#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperSensorRawData_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperSensorRawData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/time.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperSensorRawData : public ros::Msg
  {
    public:
      ros::Time stamp;
      float left_finger_pad_force;
      float right_finger_pad_force;
      float left_finger_pad_force_filtered;
      float right_finger_pad_force_filtered;
      float left_finger_pad_forces[22];
      float right_finger_pad_forces[22];
      float left_finger_pad_forces_filtered[22];
      float right_finger_pad_forces_filtered[22];
      float acc_x_raw;
      float acc_y_raw;
      float acc_z_raw;
      float acc_x_filtered;
      float acc_y_filtered;
      float acc_z_filtered;
      bool left_contact;
      bool right_contact;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->stamp.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->stamp.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->stamp.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->stamp.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->stamp.sec);
      *(outbuffer + offset + 0) = (this->stamp.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->stamp.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->stamp.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->stamp.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->stamp.nsec);
      int32_t * val_left_finger_pad_force = (int32_t *) &(this->left_finger_pad_force);
      int32_t exp_left_finger_pad_force = (((*val_left_finger_pad_force)>>23)&255);
      if(exp_left_finger_pad_force != 0)
        exp_left_finger_pad_force += 1023-127;
      int32_t sig_left_finger_pad_force = *val_left_finger_pad_force;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_left_finger_pad_force<<5) & 0xff;
      *(outbuffer + offset++) = (sig_left_finger_pad_force>>3) & 0xff;
      *(outbuffer + offset++) = (sig_left_finger_pad_force>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_left_finger_pad_force<<4) & 0xF0) | ((sig_left_finger_pad_force>>19)&0x0F);
      *(outbuffer + offset++) = (exp_left_finger_pad_force>>4) & 0x7F;
      if(this->left_finger_pad_force < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_right_finger_pad_force = (int32_t *) &(this->right_finger_pad_force);
      int32_t exp_right_finger_pad_force = (((*val_right_finger_pad_force)>>23)&255);
      if(exp_right_finger_pad_force != 0)
        exp_right_finger_pad_force += 1023-127;
      int32_t sig_right_finger_pad_force = *val_right_finger_pad_force;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_right_finger_pad_force<<5) & 0xff;
      *(outbuffer + offset++) = (sig_right_finger_pad_force>>3) & 0xff;
      *(outbuffer + offset++) = (sig_right_finger_pad_force>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_right_finger_pad_force<<4) & 0xF0) | ((sig_right_finger_pad_force>>19)&0x0F);
      *(outbuffer + offset++) = (exp_right_finger_pad_force>>4) & 0x7F;
      if(this->right_finger_pad_force < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_left_finger_pad_force_filtered = (int32_t *) &(this->left_finger_pad_force_filtered);
      int32_t exp_left_finger_pad_force_filtered = (((*val_left_finger_pad_force_filtered)>>23)&255);
      if(exp_left_finger_pad_force_filtered != 0)
        exp_left_finger_pad_force_filtered += 1023-127;
      int32_t sig_left_finger_pad_force_filtered = *val_left_finger_pad_force_filtered;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_left_finger_pad_force_filtered<<5) & 0xff;
      *(outbuffer + offset++) = (sig_left_finger_pad_force_filtered>>3) & 0xff;
      *(outbuffer + offset++) = (sig_left_finger_pad_force_filtered>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_left_finger_pad_force_filtered<<4) & 0xF0) | ((sig_left_finger_pad_force_filtered>>19)&0x0F);
      *(outbuffer + offset++) = (exp_left_finger_pad_force_filtered>>4) & 0x7F;
      if(this->left_finger_pad_force_filtered < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_right_finger_pad_force_filtered = (int32_t *) &(this->right_finger_pad_force_filtered);
      int32_t exp_right_finger_pad_force_filtered = (((*val_right_finger_pad_force_filtered)>>23)&255);
      if(exp_right_finger_pad_force_filtered != 0)
        exp_right_finger_pad_force_filtered += 1023-127;
      int32_t sig_right_finger_pad_force_filtered = *val_right_finger_pad_force_filtered;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_right_finger_pad_force_filtered<<5) & 0xff;
      *(outbuffer + offset++) = (sig_right_finger_pad_force_filtered>>3) & 0xff;
      *(outbuffer + offset++) = (sig_right_finger_pad_force_filtered>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_right_finger_pad_force_filtered<<4) & 0xF0) | ((sig_right_finger_pad_force_filtered>>19)&0x0F);
      *(outbuffer + offset++) = (exp_right_finger_pad_force_filtered>>4) & 0x7F;
      if(this->right_finger_pad_force_filtered < 0) *(outbuffer + offset -1) |= 0x80;
      unsigned char * left_finger_pad_forces_val = (unsigned char *) this->left_finger_pad_forces;
      for( uint8_t i = 0; i < 22; i++){
      int32_t * val_left_finger_pad_forcesi = (int32_t *) &(this->left_finger_pad_forces[i]);
      int32_t exp_left_finger_pad_forcesi = (((*val_left_finger_pad_forcesi)>>23)&255);
      if(exp_left_finger_pad_forcesi != 0)
        exp_left_finger_pad_forcesi += 1023-127;
      int32_t sig_left_finger_pad_forcesi = *val_left_finger_pad_forcesi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_left_finger_pad_forcesi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_left_finger_pad_forcesi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_left_finger_pad_forcesi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_left_finger_pad_forcesi<<4) & 0xF0) | ((sig_left_finger_pad_forcesi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_left_finger_pad_forcesi>>4) & 0x7F;
      if(this->left_finger_pad_forces[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      unsigned char * right_finger_pad_forces_val = (unsigned char *) this->right_finger_pad_forces;
      for( uint8_t i = 0; i < 22; i++){
      int32_t * val_right_finger_pad_forcesi = (int32_t *) &(this->right_finger_pad_forces[i]);
      int32_t exp_right_finger_pad_forcesi = (((*val_right_finger_pad_forcesi)>>23)&255);
      if(exp_right_finger_pad_forcesi != 0)
        exp_right_finger_pad_forcesi += 1023-127;
      int32_t sig_right_finger_pad_forcesi = *val_right_finger_pad_forcesi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_right_finger_pad_forcesi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_right_finger_pad_forcesi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_right_finger_pad_forcesi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_right_finger_pad_forcesi<<4) & 0xF0) | ((sig_right_finger_pad_forcesi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_right_finger_pad_forcesi>>4) & 0x7F;
      if(this->right_finger_pad_forces[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      unsigned char * left_finger_pad_forces_filtered_val = (unsigned char *) this->left_finger_pad_forces_filtered;
      for( uint8_t i = 0; i < 22; i++){
      int32_t * val_left_finger_pad_forces_filteredi = (int32_t *) &(this->left_finger_pad_forces_filtered[i]);
      int32_t exp_left_finger_pad_forces_filteredi = (((*val_left_finger_pad_forces_filteredi)>>23)&255);
      if(exp_left_finger_pad_forces_filteredi != 0)
        exp_left_finger_pad_forces_filteredi += 1023-127;
      int32_t sig_left_finger_pad_forces_filteredi = *val_left_finger_pad_forces_filteredi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_left_finger_pad_forces_filteredi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_left_finger_pad_forces_filteredi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_left_finger_pad_forces_filteredi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_left_finger_pad_forces_filteredi<<4) & 0xF0) | ((sig_left_finger_pad_forces_filteredi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_left_finger_pad_forces_filteredi>>4) & 0x7F;
      if(this->left_finger_pad_forces_filtered[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      unsigned char * right_finger_pad_forces_filtered_val = (unsigned char *) this->right_finger_pad_forces_filtered;
      for( uint8_t i = 0; i < 22; i++){
      int32_t * val_right_finger_pad_forces_filteredi = (int32_t *) &(this->right_finger_pad_forces_filtered[i]);
      int32_t exp_right_finger_pad_forces_filteredi = (((*val_right_finger_pad_forces_filteredi)>>23)&255);
      if(exp_right_finger_pad_forces_filteredi != 0)
        exp_right_finger_pad_forces_filteredi += 1023-127;
      int32_t sig_right_finger_pad_forces_filteredi = *val_right_finger_pad_forces_filteredi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_right_finger_pad_forces_filteredi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_right_finger_pad_forces_filteredi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_right_finger_pad_forces_filteredi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_right_finger_pad_forces_filteredi<<4) & 0xF0) | ((sig_right_finger_pad_forces_filteredi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_right_finger_pad_forces_filteredi>>4) & 0x7F;
      if(this->right_finger_pad_forces_filtered[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      int32_t * val_acc_x_raw = (int32_t *) &(this->acc_x_raw);
      int32_t exp_acc_x_raw = (((*val_acc_x_raw)>>23)&255);
      if(exp_acc_x_raw != 0)
        exp_acc_x_raw += 1023-127;
      int32_t sig_acc_x_raw = *val_acc_x_raw;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_acc_x_raw<<5) & 0xff;
      *(outbuffer + offset++) = (sig_acc_x_raw>>3) & 0xff;
      *(outbuffer + offset++) = (sig_acc_x_raw>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_acc_x_raw<<4) & 0xF0) | ((sig_acc_x_raw>>19)&0x0F);
      *(outbuffer + offset++) = (exp_acc_x_raw>>4) & 0x7F;
      if(this->acc_x_raw < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_acc_y_raw = (int32_t *) &(this->acc_y_raw);
      int32_t exp_acc_y_raw = (((*val_acc_y_raw)>>23)&255);
      if(exp_acc_y_raw != 0)
        exp_acc_y_raw += 1023-127;
      int32_t sig_acc_y_raw = *val_acc_y_raw;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_acc_y_raw<<5) & 0xff;
      *(outbuffer + offset++) = (sig_acc_y_raw>>3) & 0xff;
      *(outbuffer + offset++) = (sig_acc_y_raw>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_acc_y_raw<<4) & 0xF0) | ((sig_acc_y_raw>>19)&0x0F);
      *(outbuffer + offset++) = (exp_acc_y_raw>>4) & 0x7F;
      if(this->acc_y_raw < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_acc_z_raw = (int32_t *) &(this->acc_z_raw);
      int32_t exp_acc_z_raw = (((*val_acc_z_raw)>>23)&255);
      if(exp_acc_z_raw != 0)
        exp_acc_z_raw += 1023-127;
      int32_t sig_acc_z_raw = *val_acc_z_raw;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_acc_z_raw<<5) & 0xff;
      *(outbuffer + offset++) = (sig_acc_z_raw>>3) & 0xff;
      *(outbuffer + offset++) = (sig_acc_z_raw>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_acc_z_raw<<4) & 0xF0) | ((sig_acc_z_raw>>19)&0x0F);
      *(outbuffer + offset++) = (exp_acc_z_raw>>4) & 0x7F;
      if(this->acc_z_raw < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_acc_x_filtered = (int32_t *) &(this->acc_x_filtered);
      int32_t exp_acc_x_filtered = (((*val_acc_x_filtered)>>23)&255);
      if(exp_acc_x_filtered != 0)
        exp_acc_x_filtered += 1023-127;
      int32_t sig_acc_x_filtered = *val_acc_x_filtered;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_acc_x_filtered<<5) & 0xff;
      *(outbuffer + offset++) = (sig_acc_x_filtered>>3) & 0xff;
      *(outbuffer + offset++) = (sig_acc_x_filtered>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_acc_x_filtered<<4) & 0xF0) | ((sig_acc_x_filtered>>19)&0x0F);
      *(outbuffer + offset++) = (exp_acc_x_filtered>>4) & 0x7F;
      if(this->acc_x_filtered < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_acc_y_filtered = (int32_t *) &(this->acc_y_filtered);
      int32_t exp_acc_y_filtered = (((*val_acc_y_filtered)>>23)&255);
      if(exp_acc_y_filtered != 0)
        exp_acc_y_filtered += 1023-127;
      int32_t sig_acc_y_filtered = *val_acc_y_filtered;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_acc_y_filtered<<5) & 0xff;
      *(outbuffer + offset++) = (sig_acc_y_filtered>>3) & 0xff;
      *(outbuffer + offset++) = (sig_acc_y_filtered>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_acc_y_filtered<<4) & 0xF0) | ((sig_acc_y_filtered>>19)&0x0F);
      *(outbuffer + offset++) = (exp_acc_y_filtered>>4) & 0x7F;
      if(this->acc_y_filtered < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_acc_z_filtered = (int32_t *) &(this->acc_z_filtered);
      int32_t exp_acc_z_filtered = (((*val_acc_z_filtered)>>23)&255);
      if(exp_acc_z_filtered != 0)
        exp_acc_z_filtered += 1023-127;
      int32_t sig_acc_z_filtered = *val_acc_z_filtered;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_acc_z_filtered<<5) & 0xff;
      *(outbuffer + offset++) = (sig_acc_z_filtered>>3) & 0xff;
      *(outbuffer + offset++) = (sig_acc_z_filtered>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_acc_z_filtered<<4) & 0xF0) | ((sig_acc_z_filtered>>19)&0x0F);
      *(outbuffer + offset++) = (exp_acc_z_filtered>>4) & 0x7F;
      if(this->acc_z_filtered < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_left_contact;
      u_left_contact.real = this->left_contact;
      *(outbuffer + offset + 0) = (u_left_contact.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->left_contact);
      union {
        bool real;
        uint8_t base;
      } u_right_contact;
      u_right_contact.real = this->right_contact;
      *(outbuffer + offset + 0) = (u_right_contact.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->right_contact);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->stamp.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->stamp.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->stamp.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->stamp.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->stamp.sec);
      this->stamp.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->stamp.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->stamp.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->stamp.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->stamp.nsec);
      uint32_t * val_left_finger_pad_force = (uint32_t*) &(this->left_finger_pad_force);
      offset += 3;
      *val_left_finger_pad_force = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_left_finger_pad_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_left_finger_pad_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_left_finger_pad_force |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_left_finger_pad_force = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_left_finger_pad_force |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_left_finger_pad_force !=0)
        *val_left_finger_pad_force |= ((exp_left_finger_pad_force)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->left_finger_pad_force = -this->left_finger_pad_force;
      uint32_t * val_right_finger_pad_force = (uint32_t*) &(this->right_finger_pad_force);
      offset += 3;
      *val_right_finger_pad_force = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_right_finger_pad_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_right_finger_pad_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_right_finger_pad_force |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_right_finger_pad_force = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_right_finger_pad_force |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_right_finger_pad_force !=0)
        *val_right_finger_pad_force |= ((exp_right_finger_pad_force)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->right_finger_pad_force = -this->right_finger_pad_force;
      uint32_t * val_left_finger_pad_force_filtered = (uint32_t*) &(this->left_finger_pad_force_filtered);
      offset += 3;
      *val_left_finger_pad_force_filtered = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_left_finger_pad_force_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_left_finger_pad_force_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_left_finger_pad_force_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_left_finger_pad_force_filtered = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_left_finger_pad_force_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_left_finger_pad_force_filtered !=0)
        *val_left_finger_pad_force_filtered |= ((exp_left_finger_pad_force_filtered)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->left_finger_pad_force_filtered = -this->left_finger_pad_force_filtered;
      uint32_t * val_right_finger_pad_force_filtered = (uint32_t*) &(this->right_finger_pad_force_filtered);
      offset += 3;
      *val_right_finger_pad_force_filtered = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_right_finger_pad_force_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_right_finger_pad_force_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_right_finger_pad_force_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_right_finger_pad_force_filtered = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_right_finger_pad_force_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_right_finger_pad_force_filtered !=0)
        *val_right_finger_pad_force_filtered |= ((exp_right_finger_pad_force_filtered)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->right_finger_pad_force_filtered = -this->right_finger_pad_force_filtered;
      uint8_t * left_finger_pad_forces_val = (uint8_t*) this->left_finger_pad_forces;
      for( uint8_t i = 0; i < 22; i++){
      uint32_t * val_left_finger_pad_forcesi = (uint32_t*) &(this->left_finger_pad_forces[i]);
      offset += 3;
      *val_left_finger_pad_forcesi = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_left_finger_pad_forcesi |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_left_finger_pad_forcesi |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_left_finger_pad_forcesi |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_left_finger_pad_forcesi = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_left_finger_pad_forcesi |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_left_finger_pad_forcesi !=0)
        *val_left_finger_pad_forcesi |= ((exp_left_finger_pad_forcesi)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->left_finger_pad_forces[i] = -this->left_finger_pad_forces[i];
      }
      uint8_t * right_finger_pad_forces_val = (uint8_t*) this->right_finger_pad_forces;
      for( uint8_t i = 0; i < 22; i++){
      uint32_t * val_right_finger_pad_forcesi = (uint32_t*) &(this->right_finger_pad_forces[i]);
      offset += 3;
      *val_right_finger_pad_forcesi = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_right_finger_pad_forcesi |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_right_finger_pad_forcesi |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_right_finger_pad_forcesi |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_right_finger_pad_forcesi = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_right_finger_pad_forcesi |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_right_finger_pad_forcesi !=0)
        *val_right_finger_pad_forcesi |= ((exp_right_finger_pad_forcesi)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->right_finger_pad_forces[i] = -this->right_finger_pad_forces[i];
      }
      uint8_t * left_finger_pad_forces_filtered_val = (uint8_t*) this->left_finger_pad_forces_filtered;
      for( uint8_t i = 0; i < 22; i++){
      uint32_t * val_left_finger_pad_forces_filteredi = (uint32_t*) &(this->left_finger_pad_forces_filtered[i]);
      offset += 3;
      *val_left_finger_pad_forces_filteredi = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_left_finger_pad_forces_filteredi |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_left_finger_pad_forces_filteredi |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_left_finger_pad_forces_filteredi |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_left_finger_pad_forces_filteredi = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_left_finger_pad_forces_filteredi |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_left_finger_pad_forces_filteredi !=0)
        *val_left_finger_pad_forces_filteredi |= ((exp_left_finger_pad_forces_filteredi)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->left_finger_pad_forces_filtered[i] = -this->left_finger_pad_forces_filtered[i];
      }
      uint8_t * right_finger_pad_forces_filtered_val = (uint8_t*) this->right_finger_pad_forces_filtered;
      for( uint8_t i = 0; i < 22; i++){
      uint32_t * val_right_finger_pad_forces_filteredi = (uint32_t*) &(this->right_finger_pad_forces_filtered[i]);
      offset += 3;
      *val_right_finger_pad_forces_filteredi = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_right_finger_pad_forces_filteredi |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_right_finger_pad_forces_filteredi |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_right_finger_pad_forces_filteredi |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_right_finger_pad_forces_filteredi = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_right_finger_pad_forces_filteredi |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_right_finger_pad_forces_filteredi !=0)
        *val_right_finger_pad_forces_filteredi |= ((exp_right_finger_pad_forces_filteredi)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->right_finger_pad_forces_filtered[i] = -this->right_finger_pad_forces_filtered[i];
      }
      uint32_t * val_acc_x_raw = (uint32_t*) &(this->acc_x_raw);
      offset += 3;
      *val_acc_x_raw = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_acc_x_raw |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_acc_x_raw |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_acc_x_raw |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_acc_x_raw = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_acc_x_raw |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_acc_x_raw !=0)
        *val_acc_x_raw |= ((exp_acc_x_raw)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->acc_x_raw = -this->acc_x_raw;
      uint32_t * val_acc_y_raw = (uint32_t*) &(this->acc_y_raw);
      offset += 3;
      *val_acc_y_raw = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_acc_y_raw |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_acc_y_raw |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_acc_y_raw |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_acc_y_raw = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_acc_y_raw |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_acc_y_raw !=0)
        *val_acc_y_raw |= ((exp_acc_y_raw)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->acc_y_raw = -this->acc_y_raw;
      uint32_t * val_acc_z_raw = (uint32_t*) &(this->acc_z_raw);
      offset += 3;
      *val_acc_z_raw = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_acc_z_raw |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_acc_z_raw |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_acc_z_raw |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_acc_z_raw = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_acc_z_raw |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_acc_z_raw !=0)
        *val_acc_z_raw |= ((exp_acc_z_raw)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->acc_z_raw = -this->acc_z_raw;
      uint32_t * val_acc_x_filtered = (uint32_t*) &(this->acc_x_filtered);
      offset += 3;
      *val_acc_x_filtered = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_acc_x_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_acc_x_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_acc_x_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_acc_x_filtered = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_acc_x_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_acc_x_filtered !=0)
        *val_acc_x_filtered |= ((exp_acc_x_filtered)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->acc_x_filtered = -this->acc_x_filtered;
      uint32_t * val_acc_y_filtered = (uint32_t*) &(this->acc_y_filtered);
      offset += 3;
      *val_acc_y_filtered = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_acc_y_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_acc_y_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_acc_y_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_acc_y_filtered = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_acc_y_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_acc_y_filtered !=0)
        *val_acc_y_filtered |= ((exp_acc_y_filtered)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->acc_y_filtered = -this->acc_y_filtered;
      uint32_t * val_acc_z_filtered = (uint32_t*) &(this->acc_z_filtered);
      offset += 3;
      *val_acc_z_filtered = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_acc_z_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_acc_z_filtered |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_acc_z_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_acc_z_filtered = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_acc_z_filtered |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_acc_z_filtered !=0)
        *val_acc_z_filtered |= ((exp_acc_z_filtered)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->acc_z_filtered = -this->acc_z_filtered;
      union {
        bool real;
        uint8_t base;
      } u_left_contact;
      u_left_contact.base = 0;
      u_left_contact.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->left_contact = u_left_contact.real;
      offset += sizeof(this->left_contact);
      union {
        bool real;
        uint8_t base;
      } u_right_contact;
      u_right_contact.base = 0;
      u_right_contact.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->right_contact = u_right_contact.real;
      offset += sizeof(this->right_contact);
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperSensorRawData"; };
    const char * getMD5(){ return "696a1f2e6969deb0bc6998636ae1b17e"; };

  };

}
#endif