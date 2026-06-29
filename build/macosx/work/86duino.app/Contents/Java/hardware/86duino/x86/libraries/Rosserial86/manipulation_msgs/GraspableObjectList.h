#ifndef _ROS_manipulation_msgs_GraspableObjectList_h
#define _ROS_manipulation_msgs_GraspableObjectList_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "manipulation_msgs/GraspableObject.h"
#include "sensor_msgs/Image.h"
#include "sensor_msgs/CameraInfo.h"
#include "shape_msgs/Mesh.h"
#include "geometry_msgs/Pose.h"

namespace manipulation_msgs
{

  class GraspableObjectList : public ros::Msg
  {
    public:
      uint8_t graspable_objects_length;
      manipulation_msgs::GraspableObject st_graspable_objects;
      manipulation_msgs::GraspableObject * graspable_objects;
      sensor_msgs::Image image;
      sensor_msgs::CameraInfo camera_info;
      uint8_t meshes_length;
      shape_msgs::Mesh st_meshes;
      shape_msgs::Mesh * meshes;
      geometry_msgs::Pose reference_to_camera;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = graspable_objects_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < graspable_objects_length; i++){
      offset += this->graspable_objects[i].serialize(outbuffer + offset);
      }
      offset += this->image.serialize(outbuffer + offset);
      offset += this->camera_info.serialize(outbuffer + offset);
      *(outbuffer + offset++) = meshes_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < meshes_length; i++){
      offset += this->meshes[i].serialize(outbuffer + offset);
      }
      offset += this->reference_to_camera.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t graspable_objects_lengthT = *(inbuffer + offset++);
      if(graspable_objects_lengthT > graspable_objects_length)
        this->graspable_objects = (manipulation_msgs::GraspableObject*)realloc(this->graspable_objects, graspable_objects_lengthT * sizeof(manipulation_msgs::GraspableObject));
      offset += 3;
      graspable_objects_length = graspable_objects_lengthT;
      for( uint8_t i = 0; i < graspable_objects_length; i++){
      offset += this->st_graspable_objects.deserialize(inbuffer + offset);
        memcpy( &(this->graspable_objects[i]), &(this->st_graspable_objects), sizeof(manipulation_msgs::GraspableObject));
      }
      offset += this->image.deserialize(inbuffer + offset);
      offset += this->camera_info.deserialize(inbuffer + offset);
      uint8_t meshes_lengthT = *(inbuffer + offset++);
      if(meshes_lengthT > meshes_length)
        this->meshes = (shape_msgs::Mesh*)realloc(this->meshes, meshes_lengthT * sizeof(shape_msgs::Mesh));
      offset += 3;
      meshes_length = meshes_lengthT;
      for( uint8_t i = 0; i < meshes_length; i++){
      offset += this->st_meshes.deserialize(inbuffer + offset);
        memcpy( &(this->meshes[i]), &(this->st_meshes), sizeof(shape_msgs::Mesh));
      }
      offset += this->reference_to_camera.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "manipulation_msgs/GraspableObjectList"; };
    const char * getMD5(){ return "d67571f2982f1b7115de1e0027319109"; };

  };

}
#endif