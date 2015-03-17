#ifndef _ROS_pr2_mechanism_msgs_MechanismStatistics_h
#define _ROS_pr2_mechanism_msgs_MechanismStatistics_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "pr2_mechanism_msgs/ActuatorStatistics.h"
#include "pr2_mechanism_msgs/JointStatistics.h"
#include "pr2_mechanism_msgs/ControllerStatistics.h"

namespace pr2_mechanism_msgs
{

  class MechanismStatistics : public ros::Msg
  {
    public:
      std_msgs::Header header;
      uint8_t actuator_statistics_length;
      pr2_mechanism_msgs::ActuatorStatistics st_actuator_statistics;
      pr2_mechanism_msgs::ActuatorStatistics * actuator_statistics;
      uint8_t joint_statistics_length;
      pr2_mechanism_msgs::JointStatistics st_joint_statistics;
      pr2_mechanism_msgs::JointStatistics * joint_statistics;
      uint8_t controller_statistics_length;
      pr2_mechanism_msgs::ControllerStatistics st_controller_statistics;
      pr2_mechanism_msgs::ControllerStatistics * controller_statistics;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      *(outbuffer + offset++) = actuator_statistics_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < actuator_statistics_length; i++){
      offset += this->actuator_statistics[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = joint_statistics_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_statistics_length; i++){
      offset += this->joint_statistics[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = controller_statistics_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < controller_statistics_length; i++){
      offset += this->controller_statistics[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint8_t actuator_statistics_lengthT = *(inbuffer + offset++);
      if(actuator_statistics_lengthT > actuator_statistics_length)
        this->actuator_statistics = (pr2_mechanism_msgs::ActuatorStatistics*)realloc(this->actuator_statistics, actuator_statistics_lengthT * sizeof(pr2_mechanism_msgs::ActuatorStatistics));
      offset += 3;
      actuator_statistics_length = actuator_statistics_lengthT;
      for( uint8_t i = 0; i < actuator_statistics_length; i++){
      offset += this->st_actuator_statistics.deserialize(inbuffer + offset);
        memcpy( &(this->actuator_statistics[i]), &(this->st_actuator_statistics), sizeof(pr2_mechanism_msgs::ActuatorStatistics));
      }
      uint8_t joint_statistics_lengthT = *(inbuffer + offset++);
      if(joint_statistics_lengthT > joint_statistics_length)
        this->joint_statistics = (pr2_mechanism_msgs::JointStatistics*)realloc(this->joint_statistics, joint_statistics_lengthT * sizeof(pr2_mechanism_msgs::JointStatistics));
      offset += 3;
      joint_statistics_length = joint_statistics_lengthT;
      for( uint8_t i = 0; i < joint_statistics_length; i++){
      offset += this->st_joint_statistics.deserialize(inbuffer + offset);
        memcpy( &(this->joint_statistics[i]), &(this->st_joint_statistics), sizeof(pr2_mechanism_msgs::JointStatistics));
      }
      uint8_t controller_statistics_lengthT = *(inbuffer + offset++);
      if(controller_statistics_lengthT > controller_statistics_length)
        this->controller_statistics = (pr2_mechanism_msgs::ControllerStatistics*)realloc(this->controller_statistics, controller_statistics_lengthT * sizeof(pr2_mechanism_msgs::ControllerStatistics));
      offset += 3;
      controller_statistics_length = controller_statistics_lengthT;
      for( uint8_t i = 0; i < controller_statistics_length; i++){
      offset += this->st_controller_statistics.deserialize(inbuffer + offset);
        memcpy( &(this->controller_statistics[i]), &(this->st_controller_statistics), sizeof(pr2_mechanism_msgs::ControllerStatistics));
      }
     return offset;
    }

    const char * getType(){ return "pr2_mechanism_msgs/MechanismStatistics"; };
    const char * getMD5(){ return "b4a99069393681672c01f8c823458e04"; };

  };

}
#endif