#ifndef _ROS_program_queue_Output_h
#define _ROS_program_queue_Output_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace program_queue
{

  class Output : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * output;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_output = strlen( (const char*) this->output);
      memcpy(outbuffer + offset, &length_output, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->output, length_output);
      offset += length_output;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_output;
      memcpy(&length_output, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_output; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_output-1]=0;
      this->output = (char *)(inbuffer + offset-1);
      offset += length_output;
     return offset;
    }

    const char * getType(){ return "program_queue/Output"; };
    const char * getMD5(){ return "c8d7785b7436e847c9dd7124367e2134"; };

  };

}
#endif