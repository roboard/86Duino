#ifndef _ROS_moveit_msgs_PlanningSceneWorld_h
#define _ROS_moveit_msgs_PlanningSceneWorld_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/CollisionObject.h"
#include "octomap_msgs/OctomapWithPose.h"

namespace moveit_msgs
{

  class PlanningSceneWorld : public ros::Msg
  {
    public:
      uint8_t collision_objects_length;
      moveit_msgs::CollisionObject st_collision_objects;
      moveit_msgs::CollisionObject * collision_objects;
      octomap_msgs::OctomapWithPose octomap;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = collision_objects_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < collision_objects_length; i++){
      offset += this->collision_objects[i].serialize(outbuffer + offset);
      }
      offset += this->octomap.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t collision_objects_lengthT = *(inbuffer + offset++);
      if(collision_objects_lengthT > collision_objects_length)
        this->collision_objects = (moveit_msgs::CollisionObject*)realloc(this->collision_objects, collision_objects_lengthT * sizeof(moveit_msgs::CollisionObject));
      offset += 3;
      collision_objects_length = collision_objects_lengthT;
      for( uint8_t i = 0; i < collision_objects_length; i++){
      offset += this->st_collision_objects.deserialize(inbuffer + offset);
        memcpy( &(this->collision_objects[i]), &(this->st_collision_objects), sizeof(moveit_msgs::CollisionObject));
      }
      offset += this->octomap.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "moveit_msgs/PlanningSceneWorld"; };
    const char * getMD5(){ return "373d88390d1db385335639f687723ee6"; };

  };

}
#endif