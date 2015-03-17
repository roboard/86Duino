#ifndef _ROS_object_recognition_msgs_ObjectType_h
#define _ROS_object_recognition_msgs_ObjectType_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace object_recognition_msgs
{

  class ObjectType : public ros::Msg
  {
    public:
      char * key;
      char * db;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_key = strlen( (const char*) this->key);
      memcpy(outbuffer + offset, &length_key, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->key, length_key);
      offset += length_key;
      uint32_t length_db = strlen( (const char*) this->db);
      memcpy(outbuffer + offset, &length_db, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->db, length_db);
      offset += length_db;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_key;
      memcpy(&length_key, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_key; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_key-1]=0;
      this->key = (char *)(inbuffer + offset-1);
      offset += length_key;
      uint32_t length_db;
      memcpy(&length_db, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_db; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_db-1]=0;
      this->db = (char *)(inbuffer + offset-1);
      offset += length_db;
     return offset;
    }

    const char * getType(){ return "object_recognition_msgs/ObjectType"; };
    const char * getMD5(){ return "ac757ec5be1998b0167e7efcda79e3cf"; };

  };

}
#endif