#ifndef _ROS_concert_msgs_LinkGraph_h
#define _ROS_concert_msgs_LinkGraph_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "concert_msgs/LinkNode.h"
#include "concert_msgs/LinkConnection.h"
#include "concert_msgs/LinkEdge.h"

namespace concert_msgs
{

  class LinkGraph : public ros::Msg
  {
    public:
      uint8_t nodes_length;
      concert_msgs::LinkNode st_nodes;
      concert_msgs::LinkNode * nodes;
      uint8_t topics_length;
      concert_msgs::LinkConnection st_topics;
      concert_msgs::LinkConnection * topics;
      uint8_t actions_length;
      concert_msgs::LinkConnection st_actions;
      concert_msgs::LinkConnection * actions;
      uint8_t edges_length;
      concert_msgs::LinkEdge st_edges;
      concert_msgs::LinkEdge * edges;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = nodes_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < nodes_length; i++){
      offset += this->nodes[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = topics_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < topics_length; i++){
      offset += this->topics[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = actions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < actions_length; i++){
      offset += this->actions[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = edges_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < edges_length; i++){
      offset += this->edges[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t nodes_lengthT = *(inbuffer + offset++);
      if(nodes_lengthT > nodes_length)
        this->nodes = (concert_msgs::LinkNode*)realloc(this->nodes, nodes_lengthT * sizeof(concert_msgs::LinkNode));
      offset += 3;
      nodes_length = nodes_lengthT;
      for( uint8_t i = 0; i < nodes_length; i++){
      offset += this->st_nodes.deserialize(inbuffer + offset);
        memcpy( &(this->nodes[i]), &(this->st_nodes), sizeof(concert_msgs::LinkNode));
      }
      uint8_t topics_lengthT = *(inbuffer + offset++);
      if(topics_lengthT > topics_length)
        this->topics = (concert_msgs::LinkConnection*)realloc(this->topics, topics_lengthT * sizeof(concert_msgs::LinkConnection));
      offset += 3;
      topics_length = topics_lengthT;
      for( uint8_t i = 0; i < topics_length; i++){
      offset += this->st_topics.deserialize(inbuffer + offset);
        memcpy( &(this->topics[i]), &(this->st_topics), sizeof(concert_msgs::LinkConnection));
      }
      uint8_t actions_lengthT = *(inbuffer + offset++);
      if(actions_lengthT > actions_length)
        this->actions = (concert_msgs::LinkConnection*)realloc(this->actions, actions_lengthT * sizeof(concert_msgs::LinkConnection));
      offset += 3;
      actions_length = actions_lengthT;
      for( uint8_t i = 0; i < actions_length; i++){
      offset += this->st_actions.deserialize(inbuffer + offset);
        memcpy( &(this->actions[i]), &(this->st_actions), sizeof(concert_msgs::LinkConnection));
      }
      uint8_t edges_lengthT = *(inbuffer + offset++);
      if(edges_lengthT > edges_length)
        this->edges = (concert_msgs::LinkEdge*)realloc(this->edges, edges_lengthT * sizeof(concert_msgs::LinkEdge));
      offset += 3;
      edges_length = edges_lengthT;
      for( uint8_t i = 0; i < edges_length; i++){
      offset += this->st_edges.deserialize(inbuffer + offset);
        memcpy( &(this->edges[i]), &(this->st_edges), sizeof(concert_msgs::LinkEdge));
      }
     return offset;
    }

    const char * getType(){ return "concert_msgs/LinkGraph"; };
    const char * getMD5(){ return "6b8d2cdce844002beff4e2b35b1567e2"; };

  };

}
#endif