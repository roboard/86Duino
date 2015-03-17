#ifndef _ROS_SERVICE_TestBond_h
#define _ROS_SERVICE_TestBond_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "ros/duration.h"

namespace test_bond
{

static const char TESTBOND[] = "test_bond/TestBond";

  class TestBondRequest : public ros::Msg
  {
    public:
      char * topic;
      char * id;
      ros::Duration delay_connect;
      ros::Duration delay_death;
      bool inhibit_death;
      bool inhibit_death_message;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_topic = strlen( (const char*) this->topic);
      memcpy(outbuffer + offset, &length_topic, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->topic, length_topic);
      offset += length_topic;
      uint32_t length_id = strlen( (const char*) this->id);
      memcpy(outbuffer + offset, &length_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->id, length_id);
      offset += length_id;
      *(outbuffer + offset + 0) = (this->delay_connect.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->delay_connect.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->delay_connect.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->delay_connect.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->delay_connect.sec);
      *(outbuffer + offset + 0) = (this->delay_connect.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->delay_connect.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->delay_connect.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->delay_connect.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->delay_connect.nsec);
      *(outbuffer + offset + 0) = (this->delay_death.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->delay_death.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->delay_death.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->delay_death.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->delay_death.sec);
      *(outbuffer + offset + 0) = (this->delay_death.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->delay_death.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->delay_death.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->delay_death.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->delay_death.nsec);
      union {
        bool real;
        uint8_t base;
      } u_inhibit_death;
      u_inhibit_death.real = this->inhibit_death;
      *(outbuffer + offset + 0) = (u_inhibit_death.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->inhibit_death);
      union {
        bool real;
        uint8_t base;
      } u_inhibit_death_message;
      u_inhibit_death_message.real = this->inhibit_death_message;
      *(outbuffer + offset + 0) = (u_inhibit_death_message.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->inhibit_death_message);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_topic;
      memcpy(&length_topic, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_topic; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_topic-1]=0;
      this->topic = (char *)(inbuffer + offset-1);
      offset += length_topic;
      uint32_t length_id;
      memcpy(&length_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_id-1]=0;
      this->id = (char *)(inbuffer + offset-1);
      offset += length_id;
      this->delay_connect.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->delay_connect.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->delay_connect.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->delay_connect.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->delay_connect.sec);
      this->delay_connect.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->delay_connect.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->delay_connect.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->delay_connect.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->delay_connect.nsec);
      this->delay_death.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->delay_death.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->delay_death.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->delay_death.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->delay_death.sec);
      this->delay_death.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->delay_death.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->delay_death.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->delay_death.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->delay_death.nsec);
      union {
        bool real;
        uint8_t base;
      } u_inhibit_death;
      u_inhibit_death.base = 0;
      u_inhibit_death.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->inhibit_death = u_inhibit_death.real;
      offset += sizeof(this->inhibit_death);
      union {
        bool real;
        uint8_t base;
      } u_inhibit_death_message;
      u_inhibit_death_message.base = 0;
      u_inhibit_death_message.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->inhibit_death_message = u_inhibit_death_message.real;
      offset += sizeof(this->inhibit_death_message);
     return offset;
    }

    const char * getType(){ return TESTBOND; };
    const char * getMD5(){ return "1c7d43df2e371719140975f9c404a8bb"; };

  };

  class TestBondResponse : public ros::Msg
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

    const char * getType(){ return TESTBOND; };
    const char * getMD5(){ return "d41d8cd98f00b204e9800998ecf8427e"; };

  };

  class TestBond {
    public:
    typedef TestBondRequest Request;
    typedef TestBondResponse Response;
  };

}
#endif
