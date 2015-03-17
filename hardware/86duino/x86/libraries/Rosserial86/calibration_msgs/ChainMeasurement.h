#ifndef _ROS_calibration_msgs_ChainMeasurement_h
#define _ROS_calibration_msgs_ChainMeasurement_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "sensor_msgs/JointState.h"

namespace calibration_msgs
{

  class ChainMeasurement : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * chain_id;
      sensor_msgs::JointState chain_state;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_chain_id = strlen( (const char*) this->chain_id);
      memcpy(outbuffer + offset, &length_chain_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->chain_id, length_chain_id);
      offset += length_chain_id;
      offset += this->chain_state.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_chain_id;
      memcpy(&length_chain_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_chain_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_chain_id-1]=0;
      this->chain_id = (char *)(inbuffer + offset-1);
      offset += length_chain_id;
      offset += this->chain_state.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "calibration_msgs/ChainMeasurement"; };
    const char * getMD5(){ return "a57d957972fc9bc34b14f2a3cac0fbae"; };

  };

}
#endif