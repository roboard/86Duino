#ifndef _ROS_calibration_msgs_DenseLaserPoint_h
#define _ROS_calibration_msgs_DenseLaserPoint_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace calibration_msgs
{

  class DenseLaserPoint : public ros::Msg
  {
    public:
      float scan;
      float ray;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_scan = (int32_t *) &(this->scan);
      int32_t exp_scan = (((*val_scan)>>23)&255);
      if(exp_scan != 0)
        exp_scan += 1023-127;
      int32_t sig_scan = *val_scan;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_scan<<5) & 0xff;
      *(outbuffer + offset++) = (sig_scan>>3) & 0xff;
      *(outbuffer + offset++) = (sig_scan>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_scan<<4) & 0xF0) | ((sig_scan>>19)&0x0F);
      *(outbuffer + offset++) = (exp_scan>>4) & 0x7F;
      if(this->scan < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_ray = (int32_t *) &(this->ray);
      int32_t exp_ray = (((*val_ray)>>23)&255);
      if(exp_ray != 0)
        exp_ray += 1023-127;
      int32_t sig_ray = *val_ray;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_ray<<5) & 0xff;
      *(outbuffer + offset++) = (sig_ray>>3) & 0xff;
      *(outbuffer + offset++) = (sig_ray>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_ray<<4) & 0xF0) | ((sig_ray>>19)&0x0F);
      *(outbuffer + offset++) = (exp_ray>>4) & 0x7F;
      if(this->ray < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_scan = (uint32_t*) &(this->scan);
      offset += 3;
      *val_scan = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_scan |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_scan |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_scan |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_scan = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_scan |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_scan !=0)
        *val_scan |= ((exp_scan)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->scan = -this->scan;
      uint32_t * val_ray = (uint32_t*) &(this->ray);
      offset += 3;
      *val_ray = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_ray |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_ray |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_ray |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_ray = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_ray |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_ray !=0)
        *val_ray |= ((exp_ray)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->ray = -this->ray;
     return offset;
    }

    const char * getType(){ return "calibration_msgs/DenseLaserPoint"; };
    const char * getMD5(){ return "12821677bc3daf8fabbb485d5b0cc027"; };

  };

}
#endif