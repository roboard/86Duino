#ifndef _ROS_SERVICE_GetModelMesh_h
#define _ROS_SERVICE_GetModelMesh_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "household_objects_database_msgs/DatabaseReturnCode.h"
#include "shape_msgs/Mesh.h"

namespace household_objects_database_msgs
{

static const char GETMODELMESH[] = "household_objects_database_msgs/GetModelMesh";

  class GetModelMeshRequest : public ros::Msg
  {
    public:
      int32_t model_id;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_model_id;
      u_model_id.real = this->model_id;
      *(outbuffer + offset + 0) = (u_model_id.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_model_id.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_model_id.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_model_id.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->model_id);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      union {
        int32_t real;
        uint32_t base;
      } u_model_id;
      u_model_id.base = 0;
      u_model_id.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_model_id.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_model_id.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_model_id.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->model_id = u_model_id.real;
      offset += sizeof(this->model_id);
     return offset;
    }

    const char * getType(){ return GETMODELMESH; };
    const char * getMD5(){ return "28cb0598daf3b969068a38cd07aaa9f6"; };

  };

  class GetModelMeshResponse : public ros::Msg
  {
    public:
      household_objects_database_msgs::DatabaseReturnCode return_code;
      shape_msgs::Mesh mesh;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->return_code.serialize(outbuffer + offset);
      offset += this->mesh.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->return_code.deserialize(inbuffer + offset);
      offset += this->mesh.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETMODELMESH; };
    const char * getMD5(){ return "350330c487efb062c09ffe1ab80683de"; };

  };

  class GetModelMesh {
    public:
    typedef GetModelMeshRequest Request;
    typedef GetModelMeshResponse Response;
  };

}
#endif
