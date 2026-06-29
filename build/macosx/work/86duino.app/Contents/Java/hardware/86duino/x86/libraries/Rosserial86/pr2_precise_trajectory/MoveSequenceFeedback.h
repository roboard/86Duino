#ifndef _ROS_pr2_precise_trajectory_MoveSequenceFeedback_h
#define _ROS_pr2_precise_trajectory_MoveSequenceFeedback_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace pr2_precise_trajectory
{

  class MoveSequenceFeedback : public ros::Msg
  {
    public:
      uint32_t pose_index;
      float percent_complete;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->pose_index >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->pose_index >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->pose_index >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->pose_index >> (8 * 3)) & 0xFF;
      offset += sizeof(this->pose_index);
      union {
        float real;
        uint32_t base;
      } u_percent_complete;
      u_percent_complete.real = this->percent_complete;
      *(outbuffer + offset + 0) = (u_percent_complete.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_percent_complete.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_percent_complete.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_percent_complete.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->percent_complete);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->pose_index =  ((uint32_t) (*(inbuffer + offset)));
      this->pose_index |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->pose_index |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->pose_index |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->pose_index);
      union {
        float real;
        uint32_t base;
      } u_percent_complete;
      u_percent_complete.base = 0;
      u_percent_complete.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_percent_complete.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_percent_complete.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_percent_complete.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->percent_complete = u_percent_complete.real;
      offset += sizeof(this->percent_complete);
     return offset;
    }

    const char * getType(){ return "pr2_precise_trajectory/MoveSequenceFeedback"; };
    const char * getMD5(){ return "2b10bc69468e02fa60aa221b2bf40fb8"; };

  };

}
#endif