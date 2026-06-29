#ifndef _ROS_pr2_tilt_laser_interface_GetSnapshotGoal_h
#define _ROS_pr2_tilt_laser_interface_GetSnapshotGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_tilt_laser_interface
{

  class GetSnapshotGoal : public ros::Msg
  {
    public:
      float start_angle;
      float end_angle;
      float speed;
      bool hi_fidelity;
      bool continuous;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_start_angle;
      u_start_angle.real = this->start_angle;
      *(outbuffer + offset + 0) = (u_start_angle.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_start_angle.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_start_angle.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_start_angle.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->start_angle);
      union {
        float real;
        uint32_t base;
      } u_end_angle;
      u_end_angle.real = this->end_angle;
      *(outbuffer + offset + 0) = (u_end_angle.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_end_angle.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_end_angle.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_end_angle.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->end_angle);
      union {
        float real;
        uint32_t base;
      } u_speed;
      u_speed.real = this->speed;
      *(outbuffer + offset + 0) = (u_speed.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_speed.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_speed.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_speed.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->speed);
      union {
        bool real;
        uint8_t base;
      } u_hi_fidelity;
      u_hi_fidelity.real = this->hi_fidelity;
      *(outbuffer + offset + 0) = (u_hi_fidelity.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->hi_fidelity);
      union {
        bool real;
        uint8_t base;
      } u_continuous;
      u_continuous.real = this->continuous;
      *(outbuffer + offset + 0) = (u_continuous.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->continuous);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_start_angle;
      u_start_angle.base = 0;
      u_start_angle.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_start_angle.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_start_angle.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_start_angle.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->start_angle = u_start_angle.real;
      offset += sizeof(this->start_angle);
      union {
        float real;
        uint32_t base;
      } u_end_angle;
      u_end_angle.base = 0;
      u_end_angle.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_end_angle.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_end_angle.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_end_angle.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->end_angle = u_end_angle.real;
      offset += sizeof(this->end_angle);
      union {
        float real;
        uint32_t base;
      } u_speed;
      u_speed.base = 0;
      u_speed.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_speed.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_speed.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_speed.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->speed = u_speed.real;
      offset += sizeof(this->speed);
      union {
        bool real;
        uint8_t base;
      } u_hi_fidelity;
      u_hi_fidelity.base = 0;
      u_hi_fidelity.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->hi_fidelity = u_hi_fidelity.real;
      offset += sizeof(this->hi_fidelity);
      union {
        bool real;
        uint8_t base;
      } u_continuous;
      u_continuous.base = 0;
      u_continuous.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->continuous = u_continuous.real;
      offset += sizeof(this->continuous);
     return offset;
    }

    const char * getType(){ return "pr2_tilt_laser_interface/GetSnapshotGoal"; };
    const char * getMD5(){ return "464673531a0e6172d0d965e354c952f0"; };

  };

}
#endif