#ifndef _ROS_joint_qualification_controllers_RobotData_h
#define _ROS_joint_qualification_controllers_RobotData_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "joint_qualification_controllers/JointData.h"
#include "joint_qualification_controllers/ActuatorData.h"

namespace joint_qualification_controllers
{

  class RobotData : public ros::Msg
  {
    public:
      float test_time;
      bool timeout;
      int32_t num_joints;
      int32_t num_actuators;
      uint8_t joint_data_length;
      joint_qualification_controllers::JointData st_joint_data;
      joint_qualification_controllers::JointData * joint_data;
      uint8_t actuator_data_length;
      joint_qualification_controllers::ActuatorData st_actuator_data;
      joint_qualification_controllers::ActuatorData * actuator_data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_test_time;
      u_test_time.real = this->test_time;
      *(outbuffer + offset + 0) = (u_test_time.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_test_time.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_test_time.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_test_time.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->test_time);
      union {
        bool real;
        uint8_t base;
      } u_timeout;
      u_timeout.real = this->timeout;
      *(outbuffer + offset + 0) = (u_timeout.base >> (8 * 0)) & 0xFF;
      offset += sizeof(this->timeout);
      union {
        int32_t real;
        uint32_t base;
      } u_num_joints;
      u_num_joints.real = this->num_joints;
      *(outbuffer + offset + 0) = (u_num_joints.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_num_joints.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_num_joints.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_num_joints.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->num_joints);
      union {
        int32_t real;
        uint32_t base;
      } u_num_actuators;
      u_num_actuators.real = this->num_actuators;
      *(outbuffer + offset + 0) = (u_num_actuators.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_num_actuators.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_num_actuators.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_num_actuators.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->num_actuators);
      *(outbuffer + offset++) = joint_data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_data_length; i++){
      offset += this->joint_data[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = actuator_data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < actuator_data_length; i++){
      offset += this->actuator_data[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        float real;
        uint32_t base;
      } u_test_time;
      u_test_time.base = 0;
      u_test_time.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_test_time.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_test_time.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_test_time.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->test_time = u_test_time.real;
      offset += sizeof(this->test_time);
      union {
        bool real;
        uint8_t base;
      } u_timeout;
      u_timeout.base = 0;
      u_timeout.base |= ((uint8_t) (*(inbuffer + offset + 0))) << (8 * 0);
      this->timeout = u_timeout.real;
      offset += sizeof(this->timeout);
      union {
        int32_t real;
        uint32_t base;
      } u_num_joints;
      u_num_joints.base = 0;
      u_num_joints.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_num_joints.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_num_joints.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_num_joints.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->num_joints = u_num_joints.real;
      offset += sizeof(this->num_joints);
      union {
        int32_t real;
        uint32_t base;
      } u_num_actuators;
      u_num_actuators.base = 0;
      u_num_actuators.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_num_actuators.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_num_actuators.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_num_actuators.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->num_actuators = u_num_actuators.real;
      offset += sizeof(this->num_actuators);
      uint8_t joint_data_lengthT = *(inbuffer + offset++);
      if(joint_data_lengthT > joint_data_length)
        this->joint_data = (joint_qualification_controllers::JointData*)realloc(this->joint_data, joint_data_lengthT * sizeof(joint_qualification_controllers::JointData));
      offset += 3;
      joint_data_length = joint_data_lengthT;
      for( uint8_t i = 0; i < joint_data_length; i++){
      offset += this->st_joint_data.deserialize(inbuffer + offset);
        memcpy( &(this->joint_data[i]), &(this->st_joint_data), sizeof(joint_qualification_controllers::JointData));
      }
      uint8_t actuator_data_lengthT = *(inbuffer + offset++);
      if(actuator_data_lengthT > actuator_data_length)
        this->actuator_data = (joint_qualification_controllers::ActuatorData*)realloc(this->actuator_data, actuator_data_lengthT * sizeof(joint_qualification_controllers::ActuatorData));
      offset += 3;
      actuator_data_length = actuator_data_lengthT;
      for( uint8_t i = 0; i < actuator_data_length; i++){
      offset += this->st_actuator_data.deserialize(inbuffer + offset);
        memcpy( &(this->actuator_data[i]), &(this->st_actuator_data), sizeof(joint_qualification_controllers::ActuatorData));
      }
     return offset;
    }

    const char * getType(){ return "joint_qualification_controllers/RobotData"; };
    const char * getMD5(){ return "025931e8023fe22bd8228a980121730a"; };

  };

}
#endif