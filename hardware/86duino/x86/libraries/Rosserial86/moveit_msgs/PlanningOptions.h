#ifndef _ROS_moveit_msgs_PlanningOptions_h
#define _ROS_moveit_msgs_PlanningOptions_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/PlanningScene.h"

namespace moveit_msgs
{

  class PlanningOptions : public ros::Msg
  {
    public:
      moveit_msgs::PlanningScene planning_scene_diff;
      bool plan_only;
      bool look_around;
      int32_t look_around_attempts;
      float max_safe_execution_cost;
      bool replan;
      int32_t replan_attempts;
      float replan_delay;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->planning_scene_diff.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_plan_only;
      u_plan_only.real = this->plan_only;
      *(outbuffer + offset + 0) = (u_plan_only.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->plan_only);
      union {
        bool real;
        uint8_t base;
      } u_look_around;
      u_look_around.real = this->look_around;
      *(outbuffer + offset + 0) = (u_look_around.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->look_around);
      union {
        int32_t real;
        uint32_t base;
      } u_look_around_attempts;
      u_look_around_attempts.real = this->look_around_attempts;
      *(outbuffer + offset + 0) = (u_look_around_attempts.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_look_around_attempts.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_look_around_attempts.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_look_around_attempts.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->look_around_attempts);
      int32_t * val_max_safe_execution_cost = (int32_t *) &(this->max_safe_execution_cost);
      int32_t exp_max_safe_execution_cost = (((*val_max_safe_execution_cost)>>23)&255);
      if(exp_max_safe_execution_cost != 0)
        exp_max_safe_execution_cost += 1023-127;
      int32_t sig_max_safe_execution_cost = *val_max_safe_execution_cost;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_max_safe_execution_cost<<5) & 0xff;
      *(outbuffer + offset++) = (sig_max_safe_execution_cost>>3) & 0xff;
      *(outbuffer + offset++) = (sig_max_safe_execution_cost>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_max_safe_execution_cost<<4) & 0xF0) | ((sig_max_safe_execution_cost>>19)&0x0F);
      *(outbuffer + offset++) = (exp_max_safe_execution_cost>>4) & 0x7F;
      if(this->max_safe_execution_cost < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        bool real;
        uint8_t base;
      } u_replan;
      u_replan.real = this->replan;
      *(outbuffer + offset + 0) = (u_replan.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->replan);
      union {
        int32_t real;
        uint32_t base;
      } u_replan_attempts;
      u_replan_attempts.real = this->replan_attempts;
      *(outbuffer + offset + 0) = (u_replan_attempts.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_replan_attempts.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_replan_attempts.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_replan_attempts.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->replan_attempts);
      int32_t * val_replan_delay = (int32_t *) &(this->replan_delay);
      int32_t exp_replan_delay = (((*val_replan_delay)>>23)&255);
      if(exp_replan_delay != 0)
        exp_replan_delay += 1023-127;
      int32_t sig_replan_delay = *val_replan_delay;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_replan_delay<<5) & 0xff;
      *(outbuffer + offset++) = (sig_replan_delay>>3) & 0xff;
      *(outbuffer + offset++) = (sig_replan_delay>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_replan_delay<<4) & 0xF0) | ((sig_replan_delay>>19)&0x0F);
      *(outbuffer + offset++) = (exp_replan_delay>>4) & 0x7F;
      if(this->replan_delay < 0) *(outbuffer + offset -1) |= 0x80;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->planning_scene_diff.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_plan_only;
      u_plan_only.base = 0;
      u_plan_only.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->plan_only = u_plan_only.real;
      offset += sizeof(this->plan_only);
      union {
        bool real;
        uint8_t base;
      } u_look_around;
      u_look_around.base = 0;
      u_look_around.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->look_around = u_look_around.real;
      offset += sizeof(this->look_around);
      union {
        int32_t real;
        uint32_t base;
      } u_look_around_attempts;
      u_look_around_attempts.base = 0;
      u_look_around_attempts.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_look_around_attempts.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_look_around_attempts.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_look_around_attempts.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->look_around_attempts = u_look_around_attempts.real;
      offset += sizeof(this->look_around_attempts);
      uint32_t * val_max_safe_execution_cost = (uint32_t*) &(this->max_safe_execution_cost);
      offset += 3;
      *val_max_safe_execution_cost = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_max_safe_execution_cost |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_max_safe_execution_cost |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_max_safe_execution_cost |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_max_safe_execution_cost = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_max_safe_execution_cost |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_max_safe_execution_cost !=0)
        *val_max_safe_execution_cost |= ((exp_max_safe_execution_cost)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->max_safe_execution_cost = -this->max_safe_execution_cost;
      union {
        bool real;
        uint8_t base;
      } u_replan;
      u_replan.base = 0;
      u_replan.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->replan = u_replan.real;
      offset += sizeof(this->replan);
      union {
        int32_t real;
        uint32_t base;
      } u_replan_attempts;
      u_replan_attempts.base = 0;
      u_replan_attempts.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_replan_attempts.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_replan_attempts.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_replan_attempts.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->replan_attempts = u_replan_attempts.real;
      offset += sizeof(this->replan_attempts);
      uint32_t * val_replan_delay = (uint32_t*) &(this->replan_delay);
      offset += 3;
      *val_replan_delay = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_replan_delay |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_replan_delay |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_replan_delay |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_replan_delay = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_replan_delay |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_replan_delay !=0)
        *val_replan_delay |= ((exp_replan_delay)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->replan_delay = -this->replan_delay;
     return offset;
    }

    const char * getType(){ return "moveit_msgs/PlanningOptions"; };
    const char * getMD5(){ return "3934e50ede2ecea03e532aade900ab50"; };

  };

}
#endif