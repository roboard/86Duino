#ifndef _ROS_pr2_plugs_msgs_RechargeCommand_h
#define _ROS_pr2_plugs_msgs_RechargeCommand_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_plugs_msgs
{

  class RechargeCommand : public ros::Msg
  {
    public:
      int32_t command;
      char * plug_id;
      enum { UNPLUG =  1 };
      enum { PLUG_IN =  2 };
      enum { CALIBRATE =  3 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_command;
      u_command.real = this->command;
      *(outbuffer + offset + 0) = (u_command.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_command.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_command.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_command.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->command);
      uint32_t length_plug_id = strlen( (const char*) this->plug_id);
      memcpy(outbuffer + offset, &length_plug_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->plug_id, length_plug_id);
      offset += length_plug_id;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_command;
      u_command.base = 0;
      u_command.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_command.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_command.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_command.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->command = u_command.real;
      offset += sizeof(this->command);
      uint32_t length_plug_id;
      memcpy(&length_plug_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_plug_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_plug_id-1]=0;
      this->plug_id = (char *)(inbuffer + offset-1);
      offset += length_plug_id;
     return offset;
    }

    const char * getType(){ return "pr2_plugs_msgs/RechargeCommand"; };
    const char * getMD5(){ return "63c92705bbfd8357ff1fffddb3457530"; };

  };

}
#endif