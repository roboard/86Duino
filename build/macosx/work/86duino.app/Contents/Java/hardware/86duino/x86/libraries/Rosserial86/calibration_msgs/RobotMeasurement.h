#ifndef _ROS_calibration_msgs_RobotMeasurement_h
#define _ROS_calibration_msgs_RobotMeasurement_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "calibration_msgs/CameraMeasurement.h"
#include "calibration_msgs/LaserMeasurement.h"
#include "calibration_msgs/ChainMeasurement.h"

namespace calibration_msgs
{

  class RobotMeasurement : public ros::Msg
  {
    public:
      char * sample_id;
      char * target_id;
      char * chain_id;
      uint8_t M_cam_length;
      calibration_msgs::CameraMeasurement st_M_cam;
      calibration_msgs::CameraMeasurement * M_cam;
      uint8_t M_laser_length;
      calibration_msgs::LaserMeasurement st_M_laser;
      calibration_msgs::LaserMeasurement * M_laser;
      uint8_t M_chain_length;
      calibration_msgs::ChainMeasurement st_M_chain;
      calibration_msgs::ChainMeasurement * M_chain;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_sample_id = strlen( (const char*) this->sample_id);
      memcpy(outbuffer + offset, &length_sample_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->sample_id, length_sample_id);
      offset += length_sample_id;
      uint32_t length_target_id = strlen( (const char*) this->target_id);
      memcpy(outbuffer + offset, &length_target_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->target_id, length_target_id);
      offset += length_target_id;
      uint32_t length_chain_id = strlen( (const char*) this->chain_id);
      memcpy(outbuffer + offset, &length_chain_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->chain_id, length_chain_id);
      offset += length_chain_id;
      *(outbuffer + offset++) = M_cam_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < M_cam_length; i++){
      offset += this->M_cam[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = M_laser_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < M_laser_length; i++){
      offset += this->M_laser[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = M_chain_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < M_chain_length; i++){
      offset += this->M_chain[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_sample_id;
      memcpy(&length_sample_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_sample_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_sample_id-1]=0;
      this->sample_id = (char *)(inbuffer + offset-1);
      offset += length_sample_id;
      uint32_t length_target_id;
      memcpy(&length_target_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_target_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_target_id-1]=0;
      this->target_id = (char *)(inbuffer + offset-1);
      offset += length_target_id;
      uint32_t length_chain_id;
      memcpy(&length_chain_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_chain_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_chain_id-1]=0;
      this->chain_id = (char *)(inbuffer + offset-1);
      offset += length_chain_id;
      uint8_t M_cam_lengthT = *(inbuffer + offset++);
      if(M_cam_lengthT > M_cam_length)
        this->M_cam = (calibration_msgs::CameraMeasurement*)realloc(this->M_cam, M_cam_lengthT * sizeof(calibration_msgs::CameraMeasurement));
      offset += 3;
      M_cam_length = M_cam_lengthT;
      for( uint8_t i = 0; i < M_cam_length; i++){
      offset += this->st_M_cam.deserialize(inbuffer + offset);
        memcpy( &(this->M_cam[i]), &(this->st_M_cam), sizeof(calibration_msgs::CameraMeasurement));
      }
      uint8_t M_laser_lengthT = *(inbuffer + offset++);
      if(M_laser_lengthT > M_laser_length)
        this->M_laser = (calibration_msgs::LaserMeasurement*)realloc(this->M_laser, M_laser_lengthT * sizeof(calibration_msgs::LaserMeasurement));
      offset += 3;
      M_laser_length = M_laser_lengthT;
      for( uint8_t i = 0; i < M_laser_length; i++){
      offset += this->st_M_laser.deserialize(inbuffer + offset);
        memcpy( &(this->M_laser[i]), &(this->st_M_laser), sizeof(calibration_msgs::LaserMeasurement));
      }
      uint8_t M_chain_lengthT = *(inbuffer + offset++);
      if(M_chain_lengthT > M_chain_length)
        this->M_chain = (calibration_msgs::ChainMeasurement*)realloc(this->M_chain, M_chain_lengthT * sizeof(calibration_msgs::ChainMeasurement));
      offset += 3;
      M_chain_length = M_chain_lengthT;
      for( uint8_t i = 0; i < M_chain_length; i++){
      offset += this->st_M_chain.deserialize(inbuffer + offset);
        memcpy( &(this->M_chain[i]), &(this->st_M_chain), sizeof(calibration_msgs::ChainMeasurement));
      }
     return offset;
    }

    const char * getType(){ return "calibration_msgs/RobotMeasurement"; };
    const char * getMD5(){ return "fe22486c078efbf7892430dd0b99305c"; };

  };

}
#endif