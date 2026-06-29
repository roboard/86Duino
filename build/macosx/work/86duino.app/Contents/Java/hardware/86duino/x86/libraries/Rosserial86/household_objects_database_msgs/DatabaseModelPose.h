#ifndef _ROS_household_objects_database_msgs_DatabaseModelPose_h
#define _ROS_household_objects_database_msgs_DatabaseModelPose_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "object_recognition_msgs/ObjectType.h"
#include "geometry_msgs/PoseStamped.h"

namespace household_objects_database_msgs
{

  class DatabaseModelPose : public ros::Msg
  {
    public:
      int32_t model_id;
      object_recognition_msgs::ObjectType type;
      geometry_msgs::PoseStamped pose;
      float confidence;
      char * detector_name;

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
      offset += this->type.serialize(outbuffer + offset);
      offset += this->pose.serialize(outbuffer + offset);
      union {
        float real;
        uint32_t base;
      } u_confidence;
      u_confidence.real = this->confidence;
      *(outbuffer + offset + 0) = (u_confidence.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_confidence.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_confidence.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_confidence.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->confidence);
      uint32_t length_detector_name = strlen( (const char*) this->detector_name);
      memcpy(outbuffer + offset, &length_detector_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->detector_name, length_detector_name);
      offset += length_detector_name;
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
      offset += this->type.deserialize(inbuffer + offset);
      offset += this->pose.deserialize(inbuffer + offset);
      union {
        float real;
        uint32_t base;
      } u_confidence;
      u_confidence.base = 0;
      u_confidence.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_confidence.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_confidence.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_confidence.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->confidence = u_confidence.real;
      offset += sizeof(this->confidence);
      uint32_t length_detector_name;
      memcpy(&length_detector_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_detector_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_detector_name-1]=0;
      this->detector_name = (char *)(inbuffer + offset-1);
      offset += length_detector_name;
     return offset;
    }

    const char * getType(){ return "household_objects_database_msgs/DatabaseModelPose"; };
    const char * getMD5(){ return "6bc39ef48ca57cc7d4341cfc13a5a110"; };

  };

}
#endif