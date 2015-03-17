#ifndef _ROS_SERVICE_ClientList_h
#define _ROS_SERVICE_ClientList_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "concert_msgs/ConcertClient.h"

namespace concert_msgs
{

static const char CLIENTLIST[] = "concert_msgs/ClientList";

  class ClientListRequest : public ros::Msg
  {
    public:

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
     return offset;
    }

    const char * getType(){ return CLIENTLIST; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class ClientListResponse : public ros::Msg
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

    const char * getType(){ return CLIENTLIST; };
    const char * getMD5(){ return "9572afe4f30b1567d33f03cf448620a8"; };

  };

  class ClientList {
    public:
    typedef ClientListRequest Request;
    typedef ClientListResponse Response;
  };

}
#endif
