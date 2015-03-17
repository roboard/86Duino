#ifndef _ROS_turtlebot_calibration_ScanAngle_h
#define _ROS_turtlebot_calibration_ScanAngle_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace turtlebot_calibration
{

  class ScanAngle : public ros::Msg
  {
    public:
      std_msgs::Header header;
      float scan_angle;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      int32_t * val_scan_angle = (int32_t *) &(this->scan_angle);
      int32_t exp_scan_angle = (((*val_scan_angle)>>23)&255);
      if(exp_scan_angle != 0)
        exp_scan_angle += 1023-127;
      int32_t sig_scan_angle = *val_scan_angle;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_scan_angle<<5) & 0xff;
      *(outbuffer + offset++) = (sig_scan_angle>>3) & 0xff;
      *(outbuffer + offset++) = (sig_scan_angle>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_scan_angle<<4) & 0xF0) | ((sig_scan_angle>>19)&0x0F);
      *(outbuffer + offset++) = (exp_scan_angle>>4) & 0x7F;
      if(this->scan_angle < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t * val_scan_angle = (uint32_t*) &(this->scan_angle);
      offset += 3;
      *val_scan_angle = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_scan_angle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_scan_angle |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_scan_angle |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_scan_angle = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_scan_angle |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_scan_angle !=0)
        *val_scan_angle |= ((exp_scan_angle)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->scan_angle = -this->scan_angle;
     return offset;
    }

    const char * getType(){ return "turtlebot_calibration/ScanAngle"; };
    const char * getMD5(){ return "06e2c839dc5c7f5c13ac09a8b4ff0a6a"; };

  };

}
#endif