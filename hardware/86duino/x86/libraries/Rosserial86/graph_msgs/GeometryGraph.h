#ifndef _ROS_graph_msgs_GeometryGraph_h
#define _ROS_graph_msgs_GeometryGraph_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/Point.h"
#include "graph_msgs/Edges.h"

namespace graph_msgs
{

  class GeometryGraph : public ros::Msg
  {
    public:
      uint8_t nodes_length;
      geometry_msgs::Point st_nodes;
      geometry_msgs::Point * nodes;
      uint8_t edges_length;
      graph_msgs::Edges st_edges;
      graph_msgs::Edges * edges;

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
        this->nodes = (geometry_msgs::Point*)realloc(this->nodes, nodes_lengthT * sizeof(geometry_msgs::Point));
      offset += 3;
      nodes_length = nodes_lengthT;
      for( uint8_t i = 0; i < nodes_length; i++){
      offset += this->st_nodes.deserialize(inbuffer + offset);
        memcpy( &(this->nodes[i]), &(this->st_nodes), sizeof(geometry_msgs::Point));
      }
      uint8_t edges_lengthT = *(inbuffer + offset++);
      if(edges_lengthT > edges_length)
        this->edges = (graph_msgs::Edges*)realloc(this->edges, edges_lengthT * sizeof(graph_msgs::Edges));
      offset += 3;
      edges_length = edges_lengthT;
      for( uint8_t i = 0; i < edges_length; i++){
      offset += this->st_edges.deserialize(inbuffer + offset);
        memcpy( &(this->edges[i]), &(this->st_edges), sizeof(graph_msgs::Edges));
      }
     return offset;
    }

    const char * getType(){ return "graph_msgs/GeometryGraph"; };
    const char * getMD5(){ return "6f18eb8b18a1c0df93480c68feb2fd89"; };

  };

}
#endif