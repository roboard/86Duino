#ifndef _ROS_pr2_precise_trajectory_MoveSequenceGoal_h
#define _ROS_pr2_precise_trajectory_MoveSequenceGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Pose.h"

namespace pr2_precise_trajectory
{

  class MoveSequenceGoal : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t poses_length;
      geometry_msgs::Pose st_poses;
      geometry_msgs::Pose * poses;
      uint8_t times_length;
      float st_times;
      float * times;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = poses_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < poses_length; i++){
      offset += this->poses[i].serialize(outbuffer + offset);
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
      uint8_t poses_lengthT = *(inbuffer + offset++);
      if(poses_lengthT > poses_length)
        this->poses = (geometry_msgs::Pose*)realloc(this->poses, poses_lengthT * sizeof(geometry_msgs::Pose));
      offset += 3;
      poses_length = poses_lengthT;
      for( uint8_t i = 0; i < poses_length; i++){
      offset += this->st_poses.deserialize(inbuffer + offset);
        memcpy( &(this->poses[i]), &(this->st_poses), sizeof(geometry_msgs::Pose));
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

    const char * getType(){ return "pr2_precise_trajectory/MoveSequenceGoal"; };
    const char * getMD5(){ return "973bb148ba385e78971e34ca777c5ab7"; };

  };

}
#endif