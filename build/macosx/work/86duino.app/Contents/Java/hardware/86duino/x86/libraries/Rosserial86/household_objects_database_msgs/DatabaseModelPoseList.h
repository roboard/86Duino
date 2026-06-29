#ifndef _ROS_household_objects_database_msgs_DatabaseModelPoseList_h
#define _ROS_household_objects_database_msgs_DatabaseModelPoseList_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "household_objects_database_msgs/DatabaseModelPose.h"

namespace household_objects_database_msgs
{

  class DatabaseModelPoseList : public ros::Msg
  {
    public:
      uint8_t model_list_length;
      household_objects_database_msgs::DatabaseModelPose st_model_list;
      household_objects_database_msgs::DatabaseModelPose * model_list;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = model_list_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < model_list_length; i++){
      offset += this->model_list[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t model_list_lengthT = *(inbuffer + offset++);
      if(model_list_lengthT > model_list_length)
        this->model_list = (household_objects_database_msgs::DatabaseModelPose*)realloc(this->model_list, model_list_lengthT * sizeof(household_objects_database_msgs::DatabaseModelPose));
      offset += 3;
      model_list_length = model_list_lengthT;
      for( uint8_t i = 0; i < model_list_length; i++){
      offset += this->st_model_list.deserialize(inbuffer + offset);
        memcpy( &(this->model_list[i]), &(this->st_model_list), sizeof(household_objects_database_msgs::DatabaseModelPose));
      }
     return offset;
    }

    const char * getType(){ return "household_objects_database_msgs/DatabaseModelPoseList"; };
    const char * getMD5(){ return "f0bb2aa8d2ededee0ffe0f5d2107e099"; };

  };

}
#endif