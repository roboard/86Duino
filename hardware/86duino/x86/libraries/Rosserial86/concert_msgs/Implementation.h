#ifndef _ROS_concert_msgs_Implementation_h
#define _ROS_concert_msgs_Implementation_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "concert_msgs/LinkGraph.h"

namespace concert_msgs
{

  class Implementation : public ros::Msg
  {
    public:
      char * name;
      concert_msgs::LinkGraph link_graph;
      char * dot_graph;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      offset += this->link_graph.serialize(outbuffer + offset);
      uint32_t length_dot_graph = strlen( (const char*) this->dot_graph);
      memcpy(outbuffer + offset, &length_dot_graph, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->dot_graph, length_dot_graph);
      offset += length_dot_graph;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      offset += this->link_graph.deserialize(inbuffer + offset);
      uint32_t length_dot_graph;
      memcpy(&length_dot_graph, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_dot_graph; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_dot_graph-1]=0;
      this->dot_graph = (char *)(inbuffer + offset-1);
      offset += length_dot_graph;
     return offset;
    }

    const char * getType(){ return "concert_msgs/Implementation"; };
    const char * getMD5(){ return "aeb0655c516d030025a8fe13f0998166"; };

  };

}
#endif