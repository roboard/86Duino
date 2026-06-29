#ifndef _ROS_object_recognition_msgs_TableArray_h
#define _ROS_object_recognition_msgs_TableArray_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "object_recognition_msgs/Table.h"

namespace object_recognition_msgs
{

  class TableArray : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t tables_length;
      object_recognition_msgs::Table st_tables;
      object_recognition_msgs::Table * tables;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = tables_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tables_length; i++){
      offset += this->tables[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t tables_lengthT = *(inbuffer + offset++);
      if(tables_lengthT > tables_length)
        this->tables = (object_recognition_msgs::Table*)realloc(this->tables, tables_lengthT * sizeof(object_recognition_msgs::Table));
      offset += 3;
      tables_length = tables_lengthT;
      for( uint8_t i = 0; i < tables_length; i++){
      offset += this->st_tables.deserialize(inbuffer + offset);
        memcpy( &(this->tables[i]), &(this->st_tables), sizeof(object_recognition_msgs::Table));
      }
     return offset;
    }

    const char * getType(){ return "object_recognition_msgs/TableArray"; };
    const char * getMD5(){ return "d1c853e5acd0ed273eb6682dc01ab428"; };

  };

}
#endif