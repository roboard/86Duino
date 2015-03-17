#ifndef _ROS_sound_play_SoundRequest_h
#define _ROS_sound_play_SoundRequest_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace sound_play
{

  class SoundRequest : public ros::Msg
  {
    public:
      int8_t sound;
      int8_t command;
      char * arg;
      char * arg2;
      enum { BACKINGUP =  1 };
      enum { NEEDS_UNPLUGGING =  2 };
      enum { NEEDS_PLUGGING =  3 };
      enum { NEEDS_UNPLUGGING_BADLY =  4 };
      enum { NEEDS_PLUGGING_BADLY =  5 };
      enum { ALL =  -1  };
      enum { PLAY_FILE =  -2 };
      enum { SAY =  -3 };
      enum { PLAY_STOP =  0  };
      enum { PLAY_ONCE =  1  };
      enum { PLAY_START =  2  };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_sound;
      u_sound.real = this->sound;
      *(outbuffer + offset + 0) = (u_sound.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->sound);
      union {
        int8_t real;
        uint8_t base;
      } u_command;
      u_command.real = this->command;
      *(outbuffer + offset + 0) = (u_command.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->command);
      uint32_t length_arg = strlen( (const char*) this->arg);
      memcpy(outbuffer + offset, &length_arg, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->arg, length_arg);
      offset += length_arg;
      uint32_t length_arg2 = strlen( (const char*) this->arg2);
      memcpy(outbuffer + offset, &length_arg2, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->arg2, length_arg2);
      offset += length_arg2;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int8_t real;
        uint8_t base;
      } u_sound;
      u_sound.base = 0;
      u_sound.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->sound = u_sound.real;
      offset += sizeof(this->sound);
      union {
        int8_t real;
        uint8_t base;
      } u_command;
      u_command.base = 0;
      u_command.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->command = u_command.real;
      offset += sizeof(this->command);
      uint32_t length_arg;
      memcpy(&length_arg, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_arg; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_arg-1]=0;
      this->arg = (char *)(inbuffer + offset-1);
      offset += length_arg;
      uint32_t length_arg2;
      memcpy(&length_arg2, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_arg2; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_arg2-1]=0;
      this->arg2 = (char *)(inbuffer + offset-1);
      offset += length_arg2;
     return offset;
    }

    const char * getType(){ return "sound_play/SoundRequest"; };
    const char * getMD5(){ return "f194e75f1c3bf7a3e1e8b16bc9ef7c2a"; };

  };

}
#endif