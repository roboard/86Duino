#ifndef _ROS_ethercat_hardware_BoardInfo_h
#define _ROS_ethercat_hardware_BoardInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ethercat_hardware
{

  class BoardInfo : public ros::Msg
  {
    public:
      char * description;
      uint32_t product_code;
      uint32_t pcb;
      uint32_t pca;
      uint32_t serial;
      uint32_t firmware_major;
      uint32_t firmware_minor;
      float board_resistance;
      float max_pwm_ratio;
      float hw_max_current;
      bool poor_measured_motor_voltage;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_description = strlen( (const char*) this->description);
      memcpy(outbuffer + offset, &length_description, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->description, length_description);
      offset += length_description;
      *(outbuffer + offset + 0) = (this->product_code >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->product_code >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->product_code >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->product_code >> (8 * 3)) & 0xFF;
      offset += sizeof(this->product_code);
      *(outbuffer + offset + 0) = (this->pcb >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->pcb >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->pcb >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->pcb >> (8 * 3)) & 0xFF;
      offset += sizeof(this->pcb);
      *(outbuffer + offset + 0) = (this->pca >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->pca >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->pca >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->pca >> (8 * 3)) & 0xFF;
      offset += sizeof(this->pca);
      *(outbuffer + offset + 0) = (this->serial >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->serial >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->serial >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->serial >> (8 * 3)) & 0xFF;
      offset += sizeof(this->serial);
      *(outbuffer + offset + 0) = (this->firmware_major >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->firmware_major >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->firmware_major >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->firmware_major >> (8 * 3)) & 0xFF;
      offset += sizeof(this->firmware_major);
      *(outbuffer + offset + 0) = (this->firmware_minor >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->firmware_minor >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->firmware_minor >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->firmware_minor >> (8 * 3)) & 0xFF;
      offset += sizeof(this->firmware_minor);
      int32_t * val_board_resistance = (int32_t *) &(this->board_resistance);
      int32_t exp_board_resistance = (((*val_board_resistance)>>23)&255);
      if(exp_board_resistance != 0)
        exp_board_resistance += 1023-127;
      int32_t sig_board_resistance = *val_board_resistance;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_board_resistance<<5) & 0xff;
      *(outbuffer + offset++) = (sig_board_resistance>>3) & 0xff;
      *(outbuffer + offset++) = (sig_board_resistance>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_board_resistance<<4) & 0xF0) | ((sig_board_resistance>>19)&0x0F);
      *(outbuffer + offset++) = (exp_board_resistance>>4) & 0x7F;
      if(this->board_resistance < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_max_pwm_ratio = (int32_t *) &(this->max_pwm_ratio);
      int32_t exp_max_pwm_ratio = (((*val_max_pwm_ratio)>>23)&255);
      if(exp_max_pwm_ratio != 0)
        exp_max_pwm_ratio += 1023-127;
      int32_t sig_max_pwm_ratio = *val_max_pwm_ratio;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_pwm_ratio<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_pwm_ratio>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_pwm_ratio>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_pwm_ratio<<4) & 0xF0) | ((sig_max_pwm_ratio>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_pwm_ratio>>4) & 0x7F;
      if(this->max_pwm_ratio < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_hw_max_current = (int32_t *) &(this->hw_max_current);
      int32_t exp_hw_max_current = (((*val_hw_max_current)>>23)&255);
      if(exp_hw_max_current != 0)
        exp_hw_max_current += 1023-127;
      int32_t sig_hw_max_current = *val_hw_max_current;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_hw_max_current<<5) & 0xff;
      *(outbuffer + offset++) = (sig_hw_max_current>>3) & 0xff;
      *(outbuffer + offset++) = (sig_hw_max_current>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_hw_max_current<<4) & 0xF0) | ((sig_hw_max_current>>19)&0x0F);
      *(outbuffer + offset++) = (exp_hw_max_current>>4) & 0x7F;
      if(this->hw_max_current < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_poor_measured_motor_voltage;
      u_poor_measured_motor_voltage.real = this->poor_measured_motor_voltage;
      *(outbuffer + offset + 0) = (u_poor_measured_motor_voltage.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->poor_measured_motor_voltage);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_description;
      memcpy(&length_description, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_description; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_description-1]=0;
      this->description = (char *)(inbuffer + offset-1);
      offset += length_description;
      this->product_code =  ((uint32_t) (*(inbuffer + offset)));
      this->product_code |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->product_code |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->product_code |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->product_code);
      this->pcb =  ((uint32_t) (*(inbuffer + offset)));
      this->pcb |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->pcb |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->pcb |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->pcb);
      this->pca =  ((uint32_t) (*(inbuffer + offset)));
      this->pca |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->pca |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->pca |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->pca);
      this->serial =  ((uint32_t) (*(inbuffer + offset)));
      this->serial |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->serial |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->serial |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->serial);
      this->firmware_major =  ((uint32_t) (*(inbuffer + offset)));
      this->firmware_major |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->firmware_major |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->firmware_major |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->firmware_major);
      this->firmware_minor =  ((uint32_t) (*(inbuffer + offset)));
      this->firmware_minor |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->firmware_minor |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->firmware_minor |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->firmware_minor);
      uint32_t * val_board_resistance = (uint32_t*) &(this->board_resistance);
      offset += 3;
      *val_board_resistance = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_board_resistance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_board_resistance |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_board_resistance |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_board_resistance = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_board_resistance |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_board_resistance !=0)
        *val_board_resistance |= ((exp_board_resistance)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->board_resistance = -this->board_resistance;
      uint32_t * val_max_pwm_ratio = (uint32_t*) &(this->max_pwm_ratio);
      offset += 3;
      *val_max_pwm_ratio = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_pwm_ratio |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_pwm_ratio |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_pwm_ratio |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_pwm_ratio = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_pwm_ratio |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_pwm_ratio !=0)
        *val_max_pwm_ratio |= ((exp_max_pwm_ratio)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_pwm_ratio = -this->max_pwm_ratio;
      uint32_t * val_hw_max_current = (uint32_t*) &(this->hw_max_current);
      offset += 3;
      *val_hw_max_current = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_hw_max_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_hw_max_current |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_hw_max_current |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_hw_max_current = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_hw_max_current |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_hw_max_current !=0)
        *val_hw_max_current |= ((exp_hw_max_current)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->hw_max_current = -this->hw_max_current;
      union {
        bool real;
        uint8_t base;
      } u_poor_measured_motor_voltage;
      u_poor_measured_motor_voltage.base = 0;
      u_poor_measured_motor_voltage.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->poor_measured_motor_voltage = u_poor_measured_motor_voltage.real;
      offset += sizeof(this->poor_measured_motor_voltage);
     return offset;
    }

    const char * getType(){ return "ethercat_hardware/BoardInfo"; };
    const char * getMD5(){ return "ffcb87ef2725c5fab7d0d8fcd4c7e7bc"; };

  };

}
#endif