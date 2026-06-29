#ifndef _ROS_razer_hydra_Hydra_h
#define _ROS_razer_hydra_Hydra_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "razer_hydra/HydraPaddle.h"

namespace razer_hydra
{

  class Hydra : public ros::Msg
  {
    public:
      std_msgs::Header header;
      razer_hydra::HydraPaddle paddles[2];
      enum { LEFT =  0 };
      enum { RIGHT =  1 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      unsigned char * paddles_val = (unsigned char *) this->paddles;
      for( uint8_t i = 0; i < 2; i++){
      offset += this->paddles[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t * paddles_val = (uint8_t*) this->paddles;
      for( uint8_t i = 0; i < 2; i++){
      offset += this->paddles[i].deserialize(inbuffer + offset);
      }
     return offset;
    }

    const char * getType(){ return "razer_hydra/Hydra"; };
    const char * getMD5(){ return "63e785fc661607e9f6f13322700f70b8"; };

  };

}
#endif