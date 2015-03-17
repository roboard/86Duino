#ifndef _ROS_calibration_msgs_LaserMeasurement_h
#define _ROS_calibration_msgs_LaserMeasurement_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "sensor_msgs/JointState.h"
#include "calibration_msgs/DenseLaserSnapshot.h"
#include "sensor_msgs/Image.h"
#include "calibration_msgs/CalibrationPattern.h"
#include "calibration_msgs/JointStateCalibrationPattern.h"

namespace calibration_msgs
{

  class LaserMeasurement : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * laser_id;
      uint8_t joint_points_length;
      sensor_msgs::JointState st_joint_points;
      sensor_msgs::JointState * joint_points;
      bool verbose;
      calibration_msgs::DenseLaserSnapshot snapshot;
      sensor_msgs::Image laser_image;
      calibration_msgs::CalibrationPattern image_features;
      calibration_msgs::JointStateCalibrationPattern joint_features;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_laser_id = strlen( (const char*) this->laser_id);
      memcpy(outbuffer + offset, &length_laser_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->laser_id, length_laser_id);
      offset += length_laser_id;
      *(outbuffer + offset++) = joint_points_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_points_length; i++){
      offset += this->joint_points[i].serialize(outbuffer + offset);
      }
      union {
        bool real;
        uint8_t base;
      } u_verbose;
      u_verbose.real = this->verbose;
      *(outbuffer + offset + 0) = (u_verbose.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->verbose);
      offset += this->snapshot.serialize(outbuffer + offset);
      offset += this->laser_image.serialize(outbuffer + offset);
      offset += this->image_features.serialize(outbuffer + offset);
      offset += this->joint_features.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_laser_id;
      memcpy(&length_laser_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_laser_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_laser_id-1]=0;
      this->laser_id = (char *)(inbuffer + offset-1);
      offset += length_laser_id;
      uint8_t joint_points_lengthT = *(inbuffer + offset++);
      if(joint_points_lengthT > joint_points_length)
        this->joint_points = (sensor_msgs::JointState*)realloc(this->joint_points, joint_points_lengthT * sizeof(sensor_msgs::JointState));
      offset += 3;
      joint_points_length = joint_points_lengthT;
      for( uint8_t i = 0; i < joint_points_length; i++){
      offset += this->st_joint_points.deserialize(inbuffer + offset);
        memcpy( &(this->joint_points[i]), &(this->st_joint_points), sizeof(sensor_msgs::JointState));
      }
      union {
        bool real;
        uint8_t base;
      } u_verbose;
      u_verbose.base = 0;
      u_verbose.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->verbose = u_verbose.real;
      offset += sizeof(this->verbose);
      offset += this->snapshot.deserialize(inbuffer + offset);
      offset += this->laser_image.deserialize(inbuffer + offset);
      offset += this->image_features.deserialize(inbuffer + offset);
      offset += this->joint_features.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "calibration_msgs/LaserMeasurement"; };
    const char * getMD5(){ return "7fa7e818b1234a443aa5d8e315175d17"; };

  };

}
#endif