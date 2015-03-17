#ifndef _ROS_pr2_precise_trajectory_GripperSequenceGoal_h
#define _ROS_pr2_precise_trajectory_GripperSequenceGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace pr2_precise_trajectory
{

  class GripperSequenceGoal : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t positions_length;
      float st_positions;
      float * positions;
      uint8_t times_length;
      float st_times;
      float * times;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = positions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < positions_length; i++){
      union {
        float real;
        uint32_t base;
      } u_positionsi;
      u_positionsi.real = this->positions[i];
      *(outbuffer + offset + 0) = (u_positionsi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_positionsi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_positionsi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_positionsi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->positions[i]);
      }
      *(outbuffer + offset++) = times_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < times_length; i++){
      union {
        float real;
        uint32_t base;
      } u_timesi;
      u_timesi.real = this->times[i];
      *(outbuffer + offset + 0) = (u_timesi.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_timesi.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_timesi.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_timesi.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->times[i]);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t positions_lengthT = *(inbuffer + offset++);
      if(positions_lengthT > positions_length)
        this->positions = (float*)realloc(this->positions, positions_lengthT * sizeof(float));
      offset += 3;
      positions_length = positions_lengthT;
      for( uint8_t i = 0; i < positions_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_positions;
      u_st_positions.base = 0;
      u_st_positions.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_positions.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_positions.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_positions.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_positions = u_st_positions.real;
      offset += sizeof(this->st_positions);
        memcpy( &(this->positions[i]), &(this->st_positions), sizeof(float));
      }
      uint8_t times_lengthT = *(inbuffer + offset++);
      if(times_lengthT > times_length)
        this->times = (float*)realloc(this->times, times_lengthT * sizeof(float));
      offset += 3;
      times_length = times_lengthT;
      for( uint8_t i = 0; i < times_length; i++){
      union {
        float real;
        uint32_t base;
      } u_st_times;
      u_st_times.base = 0;
      u_st_times.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_st_times.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_st_times.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_st_times.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->st_times = u_st_times.real;
      offset += sizeof(this->st_times);
        memcpy( &(this->times[i]), &(this->st_times), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "pr2_precise_trajectory/GripperSequenceGoal"; };
    const char * getMD5(){ return "93bb675f6baf2d979ce58b3c08100656"; };

  };

}
#endif