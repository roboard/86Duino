#ifndef _ROS_manipulation_msgs_GraspableObject_h
#define _ROS_manipulation_msgs_GraspableObject_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "household_objects_database_msgs/DatabaseModelPose.h"
#include "sensor_msgs/PointCloud.h"
#include "manipulation_msgs/SceneRegion.h"

namespace manipulation_msgs
{

  class GraspableObject : public ros::Msg
  {
    public:
      char * reference_frame_id;
      uint8_t potential_models_length;
      household_objects_database_msgs::DatabaseModelPose st_potential_models;
      household_objects_database_msgs::DatabaseModelPose * potential_models;
      sensor_msgs::PointCloud cluster;
      manipulation_msgs::SceneRegion region;
      char * collision_name;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_reference_frame_id = strlen( (const char*) this->reference_frame_id);
      memcpy(outbuffer + offset, &length_reference_frame_id, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->reference_frame_id, length_reference_frame_id);
      offset += length_reference_frame_id;
      *(outbuffer + offset++) = potential_models_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < potential_models_length; i++){
      offset += this->potential_models[i].serialize(outbuffer + offset);
      }
      offset += this->cluster.serialize(outbuffer + offset);
      offset += this->region.serialize(outbuffer + offset);
      uint32_t length_collision_name = strlen( (const char*) this->collision_name);
      memcpy(outbuffer + offset, &length_collision_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->collision_name, length_collision_name);
      offset += length_collision_name;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_reference_frame_id;
      memcpy(&length_reference_frame_id, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_reference_frame_id; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_reference_frame_id-1]=0;
      this->reference_frame_id = (char *)(inbuffer + offset-1);
      offset += length_reference_frame_id;
      uint8_t potential_models_lengthT = *(inbuffer + offset++);
      if(potential_models_lengthT > potential_models_length)
        this->potential_models = (household_objects_database_msgs::DatabaseModelPose*)realloc(this->potential_models, potential_models_lengthT * sizeof(household_objects_database_msgs::DatabaseModelPose));
      offset += 3;
      potential_models_length = potential_models_lengthT;
      for( uint8_t i = 0; i < potential_models_length; i++){
      offset += this->st_potential_models.deserialize(inbuffer + offset);
        memcpy( &(this->potential_models[i]), &(this->st_potential_models), sizeof(household_objects_database_msgs::DatabaseModelPose));
      }
      offset += this->cluster.deserialize(inbuffer + offset);
      offset += this->region.deserialize(inbuffer + offset);
      uint32_t length_collision_name;
      memcpy(&length_collision_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_collision_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_collision_name-1]=0;
      this->collision_name = (char *)(inbuffer + offset-1);
      offset += length_collision_name;
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/GraspableObject"; };
    const char * getMD5(){ return "e2efd13d8e2bbb4697a5d71f167bceaa"; };

  };

}
#endif