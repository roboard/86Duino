#ifndef _ROS_people_msgs_PositionMeasurement_h
#define _ROS_people_msgs_PositionMeasurement_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Point.h"

namespace people_msgs
{

  class PositionMeasurement : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * name;
      char * object_id;
      geometry_msgs::Point pos;
      float reliability;
      float covariance[9];
      int8_t initialization;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      uint32_t length_object_id = strlen( (const char*) this->object_id);
      memcpy(outbuffer + offset, &length_object_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->object_id, length_object_id);
      offset += length_object_id;
      offset += this->pos.serialize(outbuffer + offset);
      int32_t * val_reliability = (int32_t *) &(this->reliability);
      int32_t exp_reliability = (((*val_reliability)>>23)&255);
      if(exp_reliability != 0)
        exp_reliability += 1023-127;
      int32_t sig_reliability = *val_reliability;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_reliability<<5) & 0xff;
      *(outbuffer + offset++) = (sig_reliability>>3) & 0xff;
      *(outbuffer + offset++) = (sig_reliability>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_reliability<<4) & 0xF0) | ((sig_reliability>>19)&0x0F);
      *(outbuffer + offset++) = (exp_reliability>>4) & 0x7F;
      if(this->reliability < 0) *(outbuffer + offset -1) |= 0x80;
      unsigned char * covariance_val = (unsigned char *) this->covariance;
      for( uint8_t i = 0; i < 9; i++){
      int32_t * val_covariancei = (int32_t *) &(this->covariance[i]);
      int32_t exp_covariancei = (((*val_covariancei)>>23)&255);
      if(exp_covariancei != 0)
        exp_covariancei += 1023-127;
      int32_t sig_covariancei = *val_covariancei;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_covariancei<<5) & 0xff;
      *(outbuffer + offset++) = (sig_covariancei>>3) & 0xff;
      *(outbuffer + offset++) = (sig_covariancei>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_covariancei<<4) & 0xF0) | ((sig_covariancei>>19)&0x0F);
      *(outbuffer + offset++) = (exp_covariancei>>4) & 0x7F;
      if(this->covariance[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      union {
        int8_t real;
        uint8_t base;
      } u_initialization;
      u_initialization.real = this->initialization;
      *(outbuffer + offset + 0) = (u_initialization.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->initialization);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      uint32_t length_object_id;
      memcpy(&length_object_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_object_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_object_id-1]=0;
      this->object_id = (char *)(inbuffer + offset-1);
      offset += length_object_id;
      offset += this->pos.deserialize(inbuffer + offset);
      uint32_t * val_reliability = (uint32_t*) &(this->reliability);
      offset += 3;
      *val_reliability = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_reliability |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_reliability |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_reliability |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_reliability = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_reliability |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_reliability !=0)
        *val_reliability |= ((exp_reliability)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->reliability = -this->reliability;
      uint8_t * covariance_val = (uint8_t*) this->covariance;
      for( uint8_t i = 0; i < 9; i++){
      uint32_t * val_covariancei = (uint32_t*) &(this->covariance[i]);
      offset += 3;
      *val_covariancei = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_covariancei |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_covariancei |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_covariancei |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_covariancei = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_covariancei |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_covariancei !=0)
        *val_covariancei |= ((exp_covariancei)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->covariance[i] = -this->covariance[i];
      }
      union {
        int8_t real;
        uint8_t base;
      } u_initialization;
      u_initialization.base = 0;
      u_initialization.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->initialization = u_initialization.real;
      offset += sizeof(this->initialization);
     return offset;
    }

    const char * getType(){ return "people_msgs/PositionMeasurement"; };
    const char * getMD5(){ return "54fa938b4ec28728e01575b79eb0ec7c"; };

  };

}
#endif