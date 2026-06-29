#ifndef _ROS_tf2_web_republisher_TFSubscriptionGoal_h
#define _ROS_tf2_web_republisher_TFSubscriptionGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace tf2_web_republisher
{

  class TFSubscriptionGoal : public ros::Msg
  {
    public:
      uint8_t source_frames_length;
      char* st_source_frames;
      char* * source_frames;
      char * target_frame;
      float angular_thres;
      float trans_thres;
      float rate;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = source_frames_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < source_frames_length; i++){
      uint32_t length_source_framesi = strlen( (const char*) this->source_frames[i]);
      memcpy(outbuffer + offset, &length_source_framesi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->source_frames[i], length_source_framesi);
      offset += length_source_framesi;
      }
      uint32_t length_target_frame = strlen( (const char*) this->target_frame);
      memcpy(outbuffer + offset, &length_target_frame, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->target_frame, length_target_frame);
      offset += length_target_frame;
      union {
        float real;
        uint32_t base;
      } u_angular_thres;
      u_angular_thres.real = this->angular_thres;
      *(outbuffer + offset + 0) = (u_angular_thres.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_angular_thres.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_angular_thres.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_angular_thres.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->angular_thres);
      union {
        float real;
        uint32_t base;
      } u_trans_thres;
      u_trans_thres.real = this->trans_thres;
      *(outbuffer + offset + 0) = (u_trans_thres.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_trans_thres.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_trans_thres.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_trans_thres.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->trans_thres);
      union {
        float real;
        uint32_t base;
      } u_rate;
      u_rate.real = this->rate;
      *(outbuffer + offset + 0) = (u_rate.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_rate.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_rate.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_rate.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->rate);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t source_frames_lengthT = *(inbuffer + offset++);
      if(source_frames_lengthT > source_frames_length)
        this->source_frames = (char**)realloc(this->source_frames, source_frames_lengthT * sizeof(char*));
      offset += 3;
      source_frames_length = source_frames_lengthT;
      for( uint8_t i = 0; i < source_frames_length; i++){
      uint32_t length_st_source_frames;
      memcpy(&length_st_source_frames, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_source_frames; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_source_frames-1]=0;
      this->st_source_frames = (char *)(inbuffer + offset-1);
      offset += length_st_source_frames;
        memcpy( &(this->source_frames[i]), &(this->st_source_frames), sizeof(char*));
      }
      uint32_t length_target_frame;
      memcpy(&length_target_frame, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_target_frame; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_target_frame-1]=0;
      this->target_frame = (char *)(inbuffer + offset-1);
      offset += length_target_frame;
      union {
        float real;
        uint32_t base;
      } u_angular_thres;
      u_angular_thres.base = 0;
      u_angular_thres.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_angular_thres.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_angular_thres.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_angular_thres.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->angular_thres = u_angular_thres.real;
      offset += sizeof(this->angular_thres);
      union {
        float real;
        uint32_t base;
      } u_trans_thres;
      u_trans_thres.base = 0;
      u_trans_thres.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_trans_thres.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_trans_thres.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_trans_thres.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->trans_thres = u_trans_thres.real;
      offset += sizeof(this->trans_thres);
      union {
        float real;
        uint32_t base;
      } u_rate;
      u_rate.base = 0;
      u_rate.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_rate.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_rate.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_rate.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->rate = u_rate.real;
      offset += sizeof(this->rate);
     return offset;
    }

    const char * getType(){ return "tf2_web_republisher/TFSubscriptionGoal"; };
    const char * getMD5(){ return "b2dae39608227a5c1c4a91ad77023a27"; };

  };

}
#endif