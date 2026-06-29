#ifndef _ROS_SERVICE_FkTest_h
#define _ROS_SERVICE_FkTest_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_calibration_launch
{

static const char FKTEST[] = "pr2_calibration_launch/FkTest";

  class FkTestRequest : public ros::Msg
  {
    public:
      char * root;
      char * tip;
      uint8_t joint_positions_length;
      float st_joint_positions;
      float * joint_positions;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_root = strlen( (const char*) this->root);
      memcpy(outbuffer + offset, &length_root, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->root, length_root);
      offset += length_root;
      uint32_t length_tip = strlen( (const char*) this->tip);
      memcpy(outbuffer + offset, &length_tip, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->tip, length_tip);
      offset += length_tip;
      *(outbuffer + offset++) = joint_positions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_positions_length; i++){
      int32_t * val_joint_positionsi = (int32_t *) &(this->joint_positions[i]);
      int32_t exp_joint_positionsi = (((*val_joint_positionsi)>>23)&255);
      if(exp_joint_positionsi != 0)
        exp_joint_positionsi += 1023-127;
      int32_t sig_joint_positionsi = *val_joint_positionsi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_positionsi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_positionsi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_positionsi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_positionsi<<4) & 0xF0) | ((sig_joint_positionsi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_positionsi>>4) & 0x7F;
      if(this->joint_positions[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_root;
      memcpy(&length_root, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_root; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_root-1]=0;
      this->root = (char *)(inbuffer + offset-1);
      offset += length_root;
      uint32_t length_tip;
      memcpy(&length_tip, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_tip; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_tip-1]=0;
      this->tip = (char *)(inbuffer + offset-1);
      offset += length_tip;
      uint8_t joint_positions_lengthT = *(inbuffer + offset++);
      if(joint_positions_lengthT > joint_positions_length)
        this->joint_positions = (float*)realloc(this->joint_positions, joint_positions_lengthT * sizeof(float));
      offset += 3;
      joint_positions_length = joint_positions_lengthT;
      for( uint8_t i = 0; i < joint_positions_length; i++){
      uint32_t * val_st_joint_positions = (uint32_t*) &(this->st_joint_positions);
      offset += 3;
      *val_st_joint_positions = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_positions |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_positions |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_positions |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_positions = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_positions |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_positions !=0)
        *val_st_joint_positions |= ((exp_st_joint_positions)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_positions = -this->st_joint_positions;
        memcpy( &(this->joint_positions[i]), &(this->st_joint_positions), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return FKTEST; };
    const char * getMD5(){ return "708e14f98ff72822d3442bcaef9c218d"; };

  };

  class FkTestResponse : public ros::Msg
  {
    public:
      uint8_t pos_length;
      float st_pos;
      float * pos;
      uint8_t rot_length;
      float st_rot;
      float * rot;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = pos_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < pos_length; i++){
      int32_t * val_posi = (int32_t *) &(this->pos[i]);
      int32_t exp_posi = (((*val_posi)>>23)&255);
      if(exp_posi != 0)
        exp_posi += 1023-127;
      int32_t sig_posi = *val_posi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_posi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_posi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_posi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_posi<<4) & 0xF0) | ((sig_posi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_posi>>4) & 0x7F;
      if(this->pos[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = rot_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < rot_length; i++){
      int32_t * val_roti = (int32_t *) &(this->rot[i]);
      int32_t exp_roti = (((*val_roti)>>23)&255);
      if(exp_roti != 0)
        exp_roti += 1023-127;
      int32_t sig_roti = *val_roti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_roti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_roti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_roti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_roti<<4) & 0xF0) | ((sig_roti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_roti>>4) & 0x7F;
      if(this->rot[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t pos_lengthT = *(inbuffer + offset++);
      if(pos_lengthT > pos_length)
        this->pos = (float*)realloc(this->pos, pos_lengthT * sizeof(float));
      offset += 3;
      pos_length = pos_lengthT;
      for( uint8_t i = 0; i < pos_length; i++){
      uint32_t * val_st_pos = (uint32_t*) &(this->st_pos);
      offset += 3;
      *val_st_pos = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_pos |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_pos |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_pos |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_pos = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_pos |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_pos !=0)
        *val_st_pos |= ((exp_st_pos)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_pos = -this->st_pos;
        memcpy( &(this->pos[i]), &(this->st_pos), sizeof(float));
      }
      uint8_t rot_lengthT = *(inbuffer + offset++);
      if(rot_lengthT > rot_length)
        this->rot = (float*)realloc(this->rot, rot_lengthT * sizeof(float));
      offset += 3;
      rot_length = rot_lengthT;
      for( uint8_t i = 0; i < rot_length; i++){
      uint32_t * val_st_rot = (uint32_t*) &(this->st_rot);
      offset += 3;
      *val_st_rot = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_rot |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_rot |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_rot |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_rot = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_rot |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_rot !=0)
        *val_st_rot |= ((exp_st_rot)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_rot = -this->st_rot;
        memcpy( &(this->rot[i]), &(this->st_rot), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return FKTEST; };
    const char * getMD5(){ return "e2248c2f30c5f3e010ed2e9434015c6e"; };

  };

  class FkTest {
    public:
    typedef FkTestRequest Request;
    typedef FkTestResponse Response;
  };

}
#endif
