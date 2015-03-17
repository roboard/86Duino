#ifndef _ROS_ethercat_hardware_MotorTrace_h
#define _ROS_ethercat_hardware_MotorTrace_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "ethercat_hardware/BoardInfo.h"
#include "ethercat_hardware/ActuatorInfo.h"
#include "ethercat_hardware/MotorTraceSample.h"

namespace ethercat_hardware
{

  class MotorTrace : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * reason;
      ethercat_hardware::BoardInfo board_info;
      ethercat_hardware::ActuatorInfo actuator_info;
      uint8_t samples_length;
      ethercat_hardware::MotorTraceSample st_samples;
      ethercat_hardware::MotorTraceSample * samples;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_reason = strlen( (const char*) this->reason);
      memcpy(outbuffer + offset, &length_reason, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->reason, length_reason);
      offset += length_reason;
      offset += this->board_info.serialize(outbuffer + offset);
      offset += this->actuator_info.serialize(outbuffer + offset);
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
      uint32_t length_reason;
      memcpy(&length_reason, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_reason; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_reason-1]=0;
      this->reason = (char *)(inbuffer + offset-1);
      offset += length_reason;
      offset += this->board_info.deserialize(inbuffer + offset);
      offset += this->actuator_info.deserialize(inbuffer + offset);
      uint8_t samples_lengthT = *(inbuffer + offset++);
      if(samples_lengthT > samples_length)
        this->samples = (ethercat_hardware::MotorTraceSample*)realloc(this->samples, samples_lengthT * sizeof(ethercat_hardware::MotorTraceSample));
      offset += 3;
      samples_length = samples_lengthT;
      for( uint8_t i = 0; i < samples_length; i++){
      offset += this->st_samples.deserialize(inbuffer + offset);
        memcpy( &(this->samples[i]), &(this->st_samples), sizeof(ethercat_hardware::MotorTraceSample));
      }
     return offset;
    }

    const char * getType(){ return "ethercat_hardware/MotorTrace"; };
    const char * getMD5(){ return "ada0b8b7f00967d292bd5bb4f59d4bd8"; };

  };

}
#endif