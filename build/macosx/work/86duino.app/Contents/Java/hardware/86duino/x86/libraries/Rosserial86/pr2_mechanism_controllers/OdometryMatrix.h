#ifndef _ROS_pr2_mechanism_controllers_OdometryMatrix_h
#define _ROS_pr2_mechanism_controllers_OdometryMatrix_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_mechanism_controllers
{

  class OdometryMatrix : public ros::Msg
  {
    public:
      uint8_t m_length;
      float st_m;
      float * m;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = m_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < m_length; i++){
      int32_t * val_mi = (int32_t *) &(this->m[i]);
      int32_t exp_mi = (((*val_mi)>>23)&255);
      if(exp_mi != 0)
        exp_mi += 1023-127;
      int32_t sig_mi = *val_mi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_mi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_mi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_mi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_mi<<4) & 0xF0) | ((sig_mi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_mi>>4) & 0x7F;
      if(this->m[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t m_lengthT = *(inbuffer + offset++);
      if(m_lengthT > m_length)
        this->m = (float*)realloc(this->m, m_lengthT * sizeof(float));
      offset += 3;
      m_length = m_lengthT;
      for( uint8_t i = 0; i < m_length; i++){
      uint32_t * val_st_m = (uint32_t*) &(this->st_m);
      offset += 3;
      *val_st_m = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_m |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_m |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_m |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_m = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_m |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_m !=0)
        *val_st_m |= ((exp_st_m)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_m = -this->st_m;
        memcpy( &(this->m[i]), &(this->st_m), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_controllers/OdometryMatrix"; };
    const char * getMD5(){ return "1f7818e7ce16454badf1bee936b0ff16"; };

  };

}
#endif