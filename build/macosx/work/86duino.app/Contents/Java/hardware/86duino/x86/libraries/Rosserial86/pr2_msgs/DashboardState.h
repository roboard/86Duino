#ifndef _ROS_pr2_msgs_DashboardState_h
#define _ROS_pr2_msgs_DashboardState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Bool.h"
#include "pr2_msgs/PowerBoardState.h"
#include "pr2_msgs/PowerState.h"
#include "pr2_msgs/AccessPoint.h"

namespace pr2_msgs
{

  class DashboardState : public ros::Msg
  {
    public:
      std_msgs::Bool motors_halted;
      bool motors_halted_valid;
      pr2_msgs::PowerBoardState power_board_state;
      bool power_board_state_valid;
      pr2_msgs::PowerState power_state;
      bool power_state_valid;
      pr2_msgs::AccessPoint access_point;
      bool access_point_valid;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->motors_halted.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_motors_halted_valid;
      u_motors_halted_valid.real = this->motors_halted_valid;
      *(outbuffer + offset + 0) = (u_motors_halted_valid.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->motors_halted_valid);
      offset += this->power_board_state.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_power_board_state_valid;
      u_power_board_state_valid.real = this->power_board_state_valid;
      *(outbuffer + offset + 0) = (u_power_board_state_valid.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->power_board_state_valid);
      offset += this->power_state.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_power_state_valid;
      u_power_state_valid.real = this->power_state_valid;
      *(outbuffer + offset + 0) = (u_power_state_valid.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->power_state_valid);
      offset += this->access_point.serialize(outbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_access_point_valid;
      u_access_point_valid.real = this->access_point_valid;
      *(outbuffer + offset + 0) = (u_access_point_valid.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->access_point_valid);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->motors_halted.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_motors_halted_valid;
      u_motors_halted_valid.base = 0;
      u_motors_halted_valid.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->motors_halted_valid = u_motors_halted_valid.real;
      offset += sizeof(this->motors_halted_valid);
      offset += this->power_board_state.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_power_board_state_valid;
      u_power_board_state_valid.base = 0;
      u_power_board_state_valid.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->power_board_state_valid = u_power_board_state_valid.real;
      offset += sizeof(this->power_board_state_valid);
      offset += this->power_state.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_power_state_valid;
      u_power_state_valid.base = 0;
      u_power_state_valid.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->power_state_valid = u_power_state_valid.real;
      offset += sizeof(this->power_state_valid);
      offset += this->access_point.deserialize(inbuffer + offset);
      union {
        bool real;
        uint8_t base;
      } u_access_point_valid;
      u_access_point_valid.base = 0;
      u_access_point_valid.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->access_point_valid = u_access_point_valid.real;
      offset += sizeof(this->access_point_valid);
     return offset;
    }

    const char * getType(){ return "pr2_msgs/DashboardState"; };
    const char * getMD5(){ return "db0cd0d535d75e0f6257b20c403e87f5"; };

  };

}
#endif