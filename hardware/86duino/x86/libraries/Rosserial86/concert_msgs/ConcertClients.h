#ifndef _ROS_concert_msgs_ConcertClients_h
#define _ROS_concert_msgs_ConcertClients_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "concert_msgs/ConcertClient.h"

namespace concert_msgs
{

  class ConcertClients : public ros::Msg
  {
    public:
      uint8_t clients_length;
      concert_msgs::ConcertClient st_clients;
      concert_msgs::ConcertClient * clients;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = clients_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < clients_length; i++){
      offset += this->clients[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t clients_lengthT = *(inbuffer + offset++);
      if(clients_lengthT > clients_length)
        this->clients = (concert_msgs::ConcertClient*)realloc(this->clients, clients_lengthT * sizeof(concert_msgs::ConcertClient));
      offset += 3;
      clients_length = clients_lengthT;
      for( uint8_t i = 0; i < clients_length; i++){
      offset += this->st_clients.deserialize(inbuffer + offset);
        memcpy( &(this->clients[i]), &(this->st_clients), sizeof(concert_msgs::ConcertClient));
      }
     return offset;
    }

    const char * getType(){ return "concert_msgs/ConcertClients"; };
    const char * getMD5(){ return "9572afe4f30b1567d33f03cf448620a8"; };

  };

}
#endif