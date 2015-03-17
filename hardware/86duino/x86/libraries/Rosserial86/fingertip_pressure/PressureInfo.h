#ifndef _ROS_fingertip_pressure_PressureInfo_h
#define _ROS_fingertip_pressure_PressureInfo_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "fingertip_pressure/PressureInfoElement.h"

namespace fingertip_pressure
{

  class PressureInfo : public ros::Msg
  {
    public:
      uint8_t sensor_length;
      fingertip_pressure::PressureInfoElement st_sensor;
      fingertip_pressure::PressureInfoElement * sensor;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = sensor_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < sensor_length; i++){
      offset += this->sensor[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t sensor_lengthT = *(inbuffer + offset++);
      if(sensor_lengthT > sensor_length)
        this->sensor = (fingertip_pressure::PressureInfoElement*)realloc(this->sensor, sensor_lengthT * sizeof(fingertip_pressure::PressureInfoElement));
      offset += 3;
      sensor_length = sensor_lengthT;
      for( uint8_t i = 0; i < sensor_length; i++){
      offset += this->st_sensor.deserialize(inbuffer + offset);
        memcpy( &(this->sensor[i]), &(this->st_sensor), sizeof(fingertip_pressure::PressureInfoElement));
      }
     return offset;
    }

    const char * getType(){ return "fingertip_pressure/PressureInfo"; };
    const char * getMD5(){ return "a11fc5bae3534aa023741e378743af5b"; };

  };

}
#endif