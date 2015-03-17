#ifndef _ROS_pr2_msgs_AccelerometerState_h
#define _ROS_pr2_msgs_AccelerometerState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Vector3.h"

namespace pr2_msgs
{

  class AccelerometerState : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t samples_length;
      geometry_msgs::Vector3 st_samples;
      geometry_msgs::Vector3 * samples;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = samples_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < samples_length; i++){
      offset += this->samples[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t samples_lengthT = *(inbuffer + offset++);
      if(samples_lengthT > samples_length)
        this->samples = (geometry_msgs::Vector3*)realloc(this->samples, samples_lengthT * sizeof(geometry_msgs::Vector3));
      offset += 3;
      samples_length = samples_lengthT;
      for( uint8_t i = 0; i < samples_length; i++){
      offset += this->st_samples.deserialize(inbuffer + offset);
        memcpy( &(this->samples[i]), &(this->st_samples), sizeof(geometry_msgs::Vector3));
      }
     return offset;
    }

    const char * getType(){ return "pr2_msgs/AccelerometerState"; };
    const char * getMD5(){ return "26492e97ed8c13252c4a85592d3e93fd"; };

  };

}
#endif