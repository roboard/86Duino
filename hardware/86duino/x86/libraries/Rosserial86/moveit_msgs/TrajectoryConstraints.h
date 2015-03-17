#ifndef _ROS_moveit_msgs_TrajectoryConstraints_h
#define _ROS_moveit_msgs_TrajectoryConstraints_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/Constraints.h"

namespace moveit_msgs
{

  class TrajectoryConstraints : public ros::Msg
  {
    public:
      uint8_t constraints_length;
      moveit_msgs::Constraints st_constraints;
      moveit_msgs::Constraints * constraints;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = constraints_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < constraints_length; i++){
      offset += this->constraints[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t constraints_lengthT = *(inbuffer + offset++);
      if(constraints_lengthT > constraints_length)
        this->constraints = (moveit_msgs::Constraints*)realloc(this->constraints, constraints_lengthT * sizeof(moveit_msgs::Constraints));
      offset += 3;
      constraints_length = constraints_lengthT;
      for( uint8_t i = 0; i < constraints_length; i++){
      offset += this->st_constraints.deserialize(inbuffer + offset);
        memcpy( &(this->constraints[i]), &(this->st_constraints), sizeof(moveit_msgs::Constraints));
      }
     return offset;
    }

    const char * getType(){ return "moveit_msgs/TrajectoryConstraints"; };
    const char * getMD5(){ return "461e1a732dfebb01e7d6c75d51a51eac"; };

  };

}
#endif