#ifndef _ROS_pr2_gripper_sensor_msgs_PR2GripperFindContactData_h
#define _ROS_pr2_gripper_sensor_msgs_PR2GripperFindContactData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/time.h"
#include "pr2_gripper_sensor_msgs/PR2GripperSensorRTState.h"

namespace pr2_gripper_sensor_msgs
{

  class PR2GripperFindContactData : public ros::Msg
  {
    public:
      ros::Time stamp;
      bool contact_conditions_met;
      bool left_fingertip_pad_contact;
      bool right_fingertip_pad_contact;
      float left_fingertip_pad_force;
      float right_fingertip_pad_force;
      float joint_position;
      float joint_effort;
      pr2_gripper_sensor_msgs::PR2GripperSensorRTState rtstate;

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
      union {
        bool real;
        uint8_t base;
      } u_contact_conditions_met;
      u_contact_conditions_met.real = this->contact_conditions_met;
      *(outbuffer + offset + 0) = (u_contact_conditions_met.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->contact_conditions_met);
      union {
        bool real;
        uint8_t base;
      } u_left_fingertip_pad_contact;
      u_left_fingertip_pad_contact.real = this->left_fingertip_pad_contact;
      *(outbuffer + offset + 0) = (u_left_fingertip_pad_contact.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->left_fingertip_pad_contact);
      union {
        bool real;
        uint8_t base;
      } u_right_fingertip_pad_contact;
      u_right_fingertip_pad_contact.real = this->right_fingertip_pad_contact;
      *(outbuffer + offset + 0) = (u_right_fingertip_pad_contact.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->right_fingertip_pad_contact);
      int32_t * val_left_fingertip_pad_force = (int32_t *) &(this->left_fingertip_pad_force);
      int32_t exp_left_fingertip_pad_force = (((*val_left_fingertip_pad_force)>>23)&255);
      if(exp_left_fingertip_pad_force != 0)
        exp_left_fingertip_pad_force += 1023-127;
      int32_t sig_left_fingertip_pad_force = *val_left_fingertip_pad_force;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_left_fingertip_pad_force<<5) & 0xff;
      *(outbuffer + offset++) = (sig_left_fingertip_pad_force>>3) & 0xff;
      *(outbuffer + offset++) = (sig_left_fingertip_pad_force>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_left_fingertip_pad_force<<4) & 0xF0) | ((sig_left_fingertip_pad_force>>19)&0x0F);
      *(outbuffer + offset++) = (exp_left_fingertip_pad_force>>4) & 0x7F;
      if(this->left_fingertip_pad_force < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_right_fingertip_pad_force = (int32_t *) &(this->right_fingertip_pad_force);
      int32_t exp_right_fingertip_pad_force = (((*val_right_fingertip_pad_force)>>23)&255);
      if(exp_right_fingertip_pad_force != 0)
        exp_right_fingertip_pad_force += 1023-127;
      int32_t sig_right_fingertip_pad_force = *val_right_fingertip_pad_force;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_right_fingertip_pad_force<<5) & 0xff;
      *(outbuffer + offset++) = (sig_right_fingertip_pad_force>>3) & 0xff;
      *(outbuffer + offset++) = (sig_right_fingertip_pad_force>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_right_fingertip_pad_force<<4) & 0xF0) | ((sig_right_fingertip_pad_force>>19)&0x0F);
      *(outbuffer + offset++) = (exp_right_fingertip_pad_force>>4) & 0x7F;
      if(this->right_fingertip_pad_force < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_joint_position = (int32_t *) &(this->joint_position);
      int32_t exp_joint_position = (((*val_joint_position)>>23)&255);
      if(exp_joint_position != 0)
        exp_joint_position += 1023-127;
      int32_t sig_joint_position = *val_joint_position;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_position<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_position>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_position>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_position<<4) & 0xF0) | ((sig_joint_position>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_position>>4) & 0x7F;
      if(this->joint_position < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_joint_effort = (int32_t *) &(this->joint_effort);
      int32_t exp_joint_effort = (((*val_joint_effort)>>23)&255);
      if(exp_joint_effort != 0)
        exp_joint_effort += 1023-127;
      int32_t sig_joint_effort = *val_joint_effort;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_effort<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_effort>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_effort>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_effort<<4) & 0xF0) | ((sig_joint_effort>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_effort>>4) & 0x7F;
      if(this->joint_effort < 0) *(outbuffer + offset -1) |= 0x80;
      offset += this->rtstate.serialize(outbuffer + offset);
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
      union {
        bool real;
        uint8_t base;
      } u_contact_conditions_met;
      u_contact_conditions_met.base = 0;
      u_contact_conditions_met.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->contact_conditions_met = u_contact_conditions_met.real;
      offset += sizeof(this->contact_conditions_met);
      union {
        bool real;
        uint8_t base;
      } u_left_fingertip_pad_contact;
      u_left_fingertip_pad_contact.base = 0;
      u_left_fingertip_pad_contact.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->left_fingertip_pad_contact = u_left_fingertip_pad_contact.real;
      offset += sizeof(this->left_fingertip_pad_contact);
      union {
        bool real;
        uint8_t base;
      } u_right_fingertip_pad_contact;
      u_right_fingertip_pad_contact.base = 0;
      u_right_fingertip_pad_contact.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->right_fingertip_pad_contact = u_right_fingertip_pad_contact.real;
      offset += sizeof(this->right_fingertip_pad_contact);
      uint32_t * val_left_fingertip_pad_force = (uint32_t*) &(this->left_fingertip_pad_force);
      offset += 3;
      *val_left_fingertip_pad_force = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_left_fingertip_pad_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_left_fingertip_pad_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_left_fingertip_pad_force |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_left_fingertip_pad_force = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_left_fingertip_pad_force |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_left_fingertip_pad_force !=0)
        *val_left_fingertip_pad_force |= ((exp_left_fingertip_pad_force)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->left_fingertip_pad_force = -this->left_fingertip_pad_force;
      uint32_t * val_right_fingertip_pad_force = (uint32_t*) &(this->right_fingertip_pad_force);
      offset += 3;
      *val_right_fingertip_pad_force = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_right_fingertip_pad_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_right_fingertip_pad_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_right_fingertip_pad_force |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_right_fingertip_pad_force = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_right_fingertip_pad_force |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_right_fingertip_pad_force !=0)
        *val_right_fingertip_pad_force |= ((exp_right_fingertip_pad_force)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->right_fingertip_pad_force = -this->right_fingertip_pad_force;
      uint32_t * val_joint_position = (uint32_t*) &(this->joint_position);
      offset += 3;
      *val_joint_position = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_joint_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_joint_position |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_joint_position |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_joint_position = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_joint_position |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_joint_position !=0)
        *val_joint_position |= ((exp_joint_position)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->joint_position = -this->joint_position;
      uint32_t * val_joint_effort = (uint32_t*) &(this->joint_effort);
      offset += 3;
      *val_joint_effort = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_joint_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_joint_effort |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_joint_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_joint_effort = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_joint_effort |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_joint_effort !=0)
        *val_joint_effort |= ((exp_joint_effort)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->joint_effort = -this->joint_effort;
      offset += this->rtstate.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "pr2_gripper_sensor_msgs/PR2GripperFindContactData"; };
    const char * getMD5(){ return "bc53e3dc7d19b896ca9b5ea205d54b91"; };

  };

}
#endif