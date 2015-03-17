#ifndef _ROS_people_msgs_People_h
#define _ROS_people_msgs_People_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "people_msgs/Person.h"

namespace people_msgs
{

  class People : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t people_length;
      people_msgs::Person st_people;
      people_msgs::Person * people;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = people_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < people_length; i++){
      offset += this->people[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t people_lengthT = *(inbuffer + offset++);
      if(people_lengthT > people_length)
        this->people = (people_msgs::Person*)realloc(this->people, people_lengthT * sizeof(people_msgs::Person));
      offset += 3;
      people_length = people_lengthT;
      for( uint8_t i = 0; i < people_length; i++){
      offset += this->st_people.deserialize(inbuffer + offset);
        memcpy( &(this->people[i]), &(this->st_people), sizeof(people_msgs::Person));
      }
     return offset;
    }

    const char * getType(){ return "people_msgs/People"; };
    const char * getMD5(){ return "18722f4b8db20cc2369740732357847b"; };

  };

}
#endif