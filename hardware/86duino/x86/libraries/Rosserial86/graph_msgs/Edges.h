#ifndef _ROS_graph_msgs_Edges_h
#define _ROS_graph_msgs_Edges_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace graph_msgs
{

  class Edges : public ros::Msg
  {
    public:
      uint8_t node_ids_length;
      uint32_t st_node_ids;
      uint32_t * node_ids;
      uint8_t weights_length;
      float st_weights;
      float * weights;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = node_ids_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < node_ids_length; i++){
      *(outbuffer + offset + 0) = (this->node_ids[i] >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->node_ids[i] >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->node_ids[i] >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->node_ids[i] >> (8 * 3)) & 0xFF;
      offset += sizeof(this->node_ids[i]);
      }
      *(outbuffer + offset++) = weights_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < weights_length; i++){
      int32_t * val_weightsi = (int32_t *) &(this->weights[i]);
      int32_t exp_weightsi = (((*val_weightsi)>>23)&255);
      if(exp_weightsi != 0)
        exp_weightsi += 1023-127;
      int32_t sig_weightsi = *val_weightsi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_weightsi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_weightsi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_weightsi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_weightsi<<4) & 0xF0) | ((sig_weightsi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_weightsi>>4) & 0x7F;
      if(this->weights[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t node_ids_lengthT = *(inbuffer + offset++);
      if(node_ids_lengthT > node_ids_length)
        this->node_ids = (uint32_t*)realloc(this->node_ids, node_ids_lengthT * sizeof(uint32_t));
      offset += 3;
      node_ids_length = node_ids_lengthT;
      for( uint8_t i = 0; i < node_ids_length; i++){
      this->st_node_ids =  ((uint32_t) (*(inbuffer + offset)));
      this->st_node_ids |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->st_node_ids |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->st_node_ids |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->st_node_ids);
        memcpy( &(this->node_ids[i]), &(this->st_node_ids), sizeof(uint32_t));
      }
      uint8_t weights_lengthT = *(inbuffer + offset++);
      if(weights_lengthT > weights_length)
        this->weights = (float*)realloc(this->weights, weights_lengthT * sizeof(float));
      offset += 3;
      weights_length = weights_lengthT;
      for( uint8_t i = 0; i < weights_length; i++){
      uint32_t * val_st_weights = (uint32_t*) &(this->st_weights);
      offset += 3;
      *val_st_weights = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_weights |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_weights |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_weights |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_weights = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_weights |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_weights !=0)
        *val_st_weights |= ((exp_st_weights)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_weights = -this->st_weights;
        memcpy( &(this->weights[i]), &(this->st_weights), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "graph_msgs/Edges"; };
    const char * getMD5(){ return "1dcd54afd0b0c0fbebeb59dbdda4c026"; };

  };

}
#endif