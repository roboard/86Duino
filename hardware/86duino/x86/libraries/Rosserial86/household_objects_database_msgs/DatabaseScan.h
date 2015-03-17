#ifndef _ROS_household_objects_database_msgs_DatabaseScan_h
#define _ROS_household_objects_database_msgs_DatabaseScan_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "geometry_msgs/PoseStamped.h"

namespace household_objects_database_msgs
{

  class DatabaseScan : public ros::Msg
  {
    public:
      int32_t model_id;
      char * bagfile_location;
      char * scan_source;
      geometry_msgs::PoseStamped pose;
      char * cloud_topic;

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
      uint32_t length_bagfile_location = strlen( (const char*) this->bagfile_location);
      memcpy(outbuffer + offset, &length_bagfile_location, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->bagfile_location, length_bagfile_location);
      offset += length_bagfile_location;
      uint32_t length_scan_source = strlen( (const char*) this->scan_source);
      memcpy(outbuffer + offset, &length_scan_source, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->scan_source, length_scan_source);
      offset += length_scan_source;
      offset += this->pose.serialize(outbuffer + offset);
      uint32_t length_cloud_topic = strlen( (const char*) this->cloud_topic);
      memcpy(outbuffer + offset, &length_cloud_topic, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->cloud_topic, length_cloud_topic);
      offset += length_cloud_topic;
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
      uint32_t length_bagfile_location;
      memcpy(&length_bagfile_location, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_bagfile_location; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_bagfile_location-1]=0;
      this->bagfile_location = (char *)(inbuffer + offset-1);
      offset += length_bagfile_location;
      uint32_t length_scan_source;
      memcpy(&length_scan_source, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_scan_source; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_scan_source-1]=0;
      this->scan_source = (char *)(inbuffer + offset-1);
      offset += length_scan_source;
      offset += this->pose.deserialize(inbuffer + offset);
      uint32_t length_cloud_topic;
      memcpy(&length_cloud_topic, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_cloud_topic; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_cloud_topic-1]=0;
      this->cloud_topic = (char *)(inbuffer + offset-1);
      offset += length_cloud_topic;
     return offset;
    }

    const char * getType(){ return "household_objects_database_msgs/DatabaseScan"; };
    const char * getMD5(){ return "7edb7abec4973143a801c25c336b4bb1"; };

  };

}
#endif