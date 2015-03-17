#ifndef _ROS_SERVICE_GraspPlanning_h
#define _ROS_SERVICE_GraspPlanning_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "manipulation_msgs/GraspPlanningErrorCode.h"
#include "manipulation_msgs/GraspableObject.h"
#include "manipulation_msgs/Grasp.h"

namespace manipulation_msgs
{

static const char GRASPPLANNING[] = "manipulation_msgs/GraspPlanning";

  class GraspPlanningRequest : public ros::Msg
  {
    public:
      char * arm_name;
      manipulation_msgs::GraspableObject target;
      char * collision_object_name;
      char * collision_support_surface_name;
      uint8_t grasps_to_evaluate_length;
      manipulation_msgs::Grasp st_grasps_to_evaluate;
      manipulation_msgs::Grasp * grasps_to_evaluate;
      uint8_t movable_obstacles_length;
      manipulation_msgs::GraspableObject st_movable_obstacles;
      manipulation_msgs::GraspableObject * movable_obstacles;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_arm_name = strlen( (const char*) this->arm_name);
      memcpy(outbuffer + offset, &length_arm_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->arm_name, length_arm_name);
      offset += length_arm_name;
      offset += this->target.serialize(outbuffer + offset);
      uint32_t length_collision_object_name = strlen( (const char*) this->collision_object_name);
      memcpy(outbuffer + offset, &length_collision_object_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->collision_object_name, length_collision_object_name);
      offset += length_collision_object_name;
      uint32_t length_collision_support_surface_name = strlen( (const char*) this->collision_support_surface_name);
      memcpy(outbuffer + offset, &length_collision_support_surface_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->collision_support_surface_name, length_collision_support_surface_name);
      offset += length_collision_support_surface_name;
      *(outbuffer + offset++) = grasps_to_evaluate_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < grasps_to_evaluate_length; i++){
      offset += this->grasps_to_evaluate[i].serialize(outbuffer + offset);
      }
      *(outbuffer + offset++) = movable_obstacles_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < movable_obstacles_length; i++){
      offset += this->movable_obstacles[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_arm_name;
      memcpy(&length_arm_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_arm_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_arm_name-1]=0;
      this->arm_name = (char *)(inbuffer + offset-1);
      offset += length_arm_name;
      offset += this->target.deserialize(inbuffer + offset);
      uint32_t length_collision_object_name;
      memcpy(&length_collision_object_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_collision_object_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_collision_object_name-1]=0;
      this->collision_object_name = (char *)(inbuffer + offset-1);
      offset += length_collision_object_name;
      uint32_t length_collision_support_surface_name;
      memcpy(&length_collision_support_surface_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_collision_support_surface_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_collision_support_surface_name-1]=0;
      this->collision_support_surface_name = (char *)(inbuffer + offset-1);
      offset += length_collision_support_surface_name;
      uint8_t grasps_to_evaluate_lengthT = *(inbuffer + offset++);
      if(grasps_to_evaluate_lengthT > grasps_to_evaluate_length)
        this->grasps_to_evaluate = (manipulation_msgs::Grasp*)realloc(this->grasps_to_evaluate, grasps_to_evaluate_lengthT * sizeof(manipulation_msgs::Grasp));
      offset += 3;
      grasps_to_evaluate_length = grasps_to_evaluate_lengthT;
      for( uint8_t i = 0; i < grasps_to_evaluate_length; i++){
      offset += this->st_grasps_to_evaluate.deserialize(inbuffer + offset);
        memcpy( &(this->grasps_to_evaluate[i]), &(this->st_grasps_to_evaluate), sizeof(manipulation_msgs::Grasp));
      }
      uint8_t movable_obstacles_lengthT = *(inbuffer + offset++);
      if(movable_obstacles_lengthT > movable_obstacles_length)
        this->movable_obstacles = (manipulation_msgs::GraspableObject*)realloc(this->movable_obstacles, movable_obstacles_lengthT * sizeof(manipulation_msgs::GraspableObject));
      offset += 3;
      movable_obstacles_length = movable_obstacles_lengthT;
      for( uint8_t i = 0; i < movable_obstacles_length; i++){
      offset += this->st_movable_obstacles.deserialize(inbuffer + offset);
        memcpy( &(this->movable_obstacles[i]), &(this->st_movable_obstacles), sizeof(manipulation_msgs::GraspableObject));
      }
     return offset;
    }

    const char * getType(){ return GRASPPLANNING; };
    const char * getMD5(){ return "077dca08a07415d82d6ab047890161f4"; };

  };

  class GraspPlanningResponse : public ros::Msg
  {
    public:
      uint8_t grasps_length;
      manipulation_msgs::Grasp st_grasps;
      manipulation_msgs::Grasp * grasps;
      manipulation_msgs::GraspPlanningErrorCode error_code;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = grasps_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < grasps_length; i++){
      offset += this->grasps[i].serialize(outbuffer + offset);
      }
      offset += this->error_code.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t grasps_lengthT = *(inbuffer + offset++);
      if(grasps_lengthT > grasps_length)
        this->grasps = (manipulation_msgs::Grasp*)realloc(this->grasps, grasps_lengthT * sizeof(manipulation_msgs::Grasp));
      offset += 3;
      grasps_length = grasps_lengthT;
      for( uint8_t i = 0; i < grasps_length; i++){
      offset += this->st_grasps.deserialize(inbuffer + offset);
        memcpy( &(this->grasps[i]), &(this->st_grasps), sizeof(manipulation_msgs::Grasp));
      }
      offset += this->error_code.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GRASPPLANNING; };
    const char * getMD5(){ return "ff7a88c4aec40207164535a24dc9c440"; };

  };

  class GraspPlanning {
    public:
    typedef GraspPlanningRequest Request;
    typedef GraspPlanningResponse Response;
  };

}
#endif
