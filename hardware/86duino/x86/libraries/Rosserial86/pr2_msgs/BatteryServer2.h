#ifndef _ROS_pr2_msgs_BatteryServer2_h
#define _ROS_pr2_msgs_BatteryServer2_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "ros/time.h"
#include "ros/duration.h"
#include "pr2_msgs/BatteryState2.h"

namespace pr2_msgs
{

  class BatteryServer2 : public ros::Msg
  {
    public:
      std_msgs::Header header;
      int32_t id;
      ros::Time last_system_update;
      ros::Duration time_left;
      int32_t average_charge;
      char * message;
      ros::Time last_controller_update;
      uint8_t battery_length;
      pr2_msgs::BatteryState2 st_battery;
      pr2_msgs::BatteryState2 * battery;
      enum { MAX_BAT_COUNT = 4 };
      enum { MAX_BAT_REG = 48 };

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      union {
        int32_t real;
        uint32_t base;
      } u_id;
      u_id.real = this->id;
      *(outbuffer + offset + 0) = (u_id.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_id.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_id.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_id.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->id);
      *(outbuffer + offset + 0) = (this->last_system_update.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->last_system_update.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->last_system_update.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->last_system_update.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->last_system_update.sec);
      *(outbuffer + offset + 0) = (this->last_system_update.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->last_system_update.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->last_system_update.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->last_system_update.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->last_system_update.nsec);
      *(outbuffer + offset + 0) = (this->time_left.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->time_left.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->time_left.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->time_left.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->time_left.sec);
      *(outbuffer + offset + 0) = (this->time_left.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->time_left.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->time_left.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->time_left.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->time_left.nsec);
      union {
        int32_t real;
        uint32_t base;
      } u_average_charge;
      u_average_charge.real = this->average_charge;
      *(outbuffer + offset + 0) = (u_average_charge.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_average_charge.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_average_charge.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_average_charge.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->average_charge);
      uint32_t length_message = strlen( (const char*) this->message);
      memcpy(outbuffer + offset, &length_message, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->message, length_message);
      offset += length_message;
      *(outbuffer + offset + 0) = (this->last_controller_update.sec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->last_controller_update.sec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->last_controller_update.sec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->last_controller_update.sec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->last_controller_update.sec);
      *(outbuffer + offset + 0) = (this->last_controller_update.nsec >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->last_controller_update.nsec >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->last_controller_update.nsec >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->last_controller_update.nsec >> (8 * 3)) & 0xFF;
      offset += sizeof(this->last_controller_update.nsec);
      *(outbuffer + offset++) = battery_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < battery_length; i++){
      offset += this->battery[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      union {
        int32_t real;
        uint32_t base;
      } u_id;
      u_id.base = 0;
      u_id.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_id.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_id.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_id.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->id = u_id.real;
      offset += sizeof(this->id);
      this->last_system_update.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->last_system_update.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->last_system_update.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->last_system_update.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->last_system_update.sec);
      this->last_system_update.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->last_system_update.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->last_system_update.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->last_system_update.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->last_system_update.nsec);
      this->time_left.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->time_left.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->time_left.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->time_left.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->time_left.sec);
      this->time_left.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->time_left.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->time_left.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->time_left.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->time_left.nsec);
      union {
        int32_t real;
        uint32_t base;
      } u_average_charge;
      u_average_charge.base = 0;
      u_average_charge.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_average_charge.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_average_charge.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_average_charge.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->average_charge = u_average_charge.real;
      offset += sizeof(this->average_charge);
      uint32_t length_message;
      memcpy(&length_message, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_message; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_message-1]=0;
      this->message = (char *)(inbuffer + offset-1);
      offset += length_message;
      this->last_controller_update.sec =  ((uint32_t) (*(inbuffer + offset)));
      this->last_controller_update.sec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->last_controller_update.sec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->last_controller_update.sec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->last_controller_update.sec);
      this->last_controller_update.nsec =  ((uint32_t) (*(inbuffer + offset)));
      this->last_controller_update.nsec |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->last_controller_update.nsec |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->last_controller_update.nsec |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->last_controller_update.nsec);
      uint8_t battery_lengthT = *(inbuffer + offset++);
      if(battery_lengthT > battery_length)
        this->battery = (pr2_msgs::BatteryState2*)realloc(this->battery, battery_lengthT * sizeof(pr2_msgs::BatteryState2));
      offset += 3;
      battery_length = battery_lengthT;
      for( uint8_t i = 0; i < battery_length; i++){
      offset += this->st_battery.deserialize(inbuffer + offset);
        memcpy( &(this->battery[i]), &(this->st_battery), sizeof(pr2_msgs::BatteryState2));
      }
     return offset;
    }

    const char * getType(){ return "pr2_msgs/BatteryServer2"; };
    const char * getMD5(){ return "5f2cec7d06c312d756189db96c1f3819"; };

  };

}
#endif