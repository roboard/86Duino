#ifndef _ROS_pr2_msgs_BatteryServer_h
#define _ROS_pr2_msgs_BatteryServer_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "pr2_msgs/BatteryState.h"

namespace pr2_msgs
{

  class BatteryServer : public ros::Msg
  {
    public:
      std_msgs::Header header;
      int32_t id;
      int32_t lastTimeSystem;
      uint16_t timeLeft;
      uint16_t averageCharge;
      char * message;
      int32_t lastTimeController;
      uint16_t present;
      uint16_t charging;
      uint16_t discharging;
      uint16_t reserved;
      uint16_t powerPresent;
      uint16_t powerNG;
      uint16_t inhibited;
      uint8_t battery_length;
      pr2_msgs::BatteryState st_battery;
      pr2_msgs::BatteryState * battery;
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
      union {
        int32_t real;
        uint32_t base;
      } u_lastTimeSystem;
      u_lastTimeSystem.real = this->lastTimeSystem;
      *(outbuffer + offset + 0) = (u_lastTimeSystem.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_lastTimeSystem.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_lastTimeSystem.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_lastTimeSystem.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->lastTimeSystem);
      *(outbuffer + offset + 0) = (this->timeLeft >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->timeLeft >> (8 * 1)) & 0xFF;
      offset += sizeof(this->timeLeft);
      *(outbuffer + offset + 0) = (this->averageCharge >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->averageCharge >> (8 * 1)) & 0xFF;
      offset += sizeof(this->averageCharge);
      uint32_t length_message = strlen( (const char*) this->message);
      memcpy(outbuffer + offset, &length_message, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->message, length_message);
      offset += length_message;
      union {
        int32_t real;
        uint32_t base;
      } u_lastTimeController;
      u_lastTimeController.real = this->lastTimeController;
      *(outbuffer + offset + 0) = (u_lastTimeController.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_lastTimeController.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_lastTimeController.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_lastTimeController.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->lastTimeController);
      *(outbuffer + offset + 0) = (this->present >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->present >> (8 * 1)) & 0xFF;
      offset += sizeof(this->present);
      *(outbuffer + offset + 0) = (this->charging >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->charging >> (8 * 1)) & 0xFF;
      offset += sizeof(this->charging);
      *(outbuffer + offset + 0) = (this->discharging >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->discharging >> (8 * 1)) & 0xFF;
      offset += sizeof(this->discharging);
      *(outbuffer + offset + 0) = (this->reserved >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->reserved >> (8 * 1)) & 0xFF;
      offset += sizeof(this->reserved);
      *(outbuffer + offset + 0) = (this->powerPresent >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->powerPresent >> (8 * 1)) & 0xFF;
      offset += sizeof(this->powerPresent);
      *(outbuffer + offset + 0) = (this->powerNG >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->powerNG >> (8 * 1)) & 0xFF;
      offset += sizeof(this->powerNG);
      *(outbuffer + offset + 0) = (this->inhibited >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->inhibited >> (8 * 1)) & 0xFF;
      offset += sizeof(this->inhibited);
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
      union {
        int32_t real;
        uint32_t base;
      } u_lastTimeSystem;
      u_lastTimeSystem.base = 0;
      u_lastTimeSystem.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_lastTimeSystem.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_lastTimeSystem.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_lastTimeSystem.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->lastTimeSystem = u_lastTimeSystem.real;
      offset += sizeof(this->lastTimeSystem);
      this->timeLeft =  ((uint16_t) (*(inbuffer + offset)));
      this->timeLeft |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->timeLeft);
      this->averageCharge =  ((uint16_t) (*(inbuffer + offset)));
      this->averageCharge |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->averageCharge);
      uint32_t length_message;
      memcpy(&length_message, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_message; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_message-1]=0;
      this->message = (char *)(inbuffer + offset-1);
      offset += length_message;
      union {
        int32_t real;
        uint32_t base;
      } u_lastTimeController;
      u_lastTimeController.base = 0;
      u_lastTimeController.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_lastTimeController.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_lastTimeController.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_lastTimeController.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->lastTimeController = u_lastTimeController.real;
      offset += sizeof(this->lastTimeController);
      this->present =  ((uint16_t) (*(inbuffer + offset)));
      this->present |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->present);
      this->charging =  ((uint16_t) (*(inbuffer + offset)));
      this->charging |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->charging);
      this->discharging =  ((uint16_t) (*(inbuffer + offset)));
      this->discharging |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->discharging);
      this->reserved =  ((uint16_t) (*(inbuffer + offset)));
      this->reserved |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->reserved);
      this->powerPresent =  ((uint16_t) (*(inbuffer + offset)));
      this->powerPresent |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->powerPresent);
      this->powerNG =  ((uint16_t) (*(inbuffer + offset)));
      this->powerNG |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->powerNG);
      this->inhibited =  ((uint16_t) (*(inbuffer + offset)));
      this->inhibited |= ((uint16_t) (*(inbuffer + offset + 1))) << (8 * 1);
      offset += sizeof(this->inhibited);
      uint8_t battery_lengthT = *(inbuffer + offset++);
      if(battery_lengthT > battery_length)
        this->battery = (pr2_msgs::BatteryState*)realloc(this->battery, battery_lengthT * sizeof(pr2_msgs::BatteryState));
      offset += 3;
      battery_length = battery_lengthT;
      for( uint8_t i = 0; i < battery_length; i++){
      offset += this->st_battery.deserialize(inbuffer + offset);
        memcpy( &(this->battery[i]), &(this->st_battery), sizeof(pr2_msgs::BatteryState));
      }
     return offset;
    }

    const char * getType(){ return "pr2_msgs/BatteryServer"; };
    const char * getMD5(){ return "4f6d6e54c9581beb1df7ea408c0727be"; };

  };

}
#endif