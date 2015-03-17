#ifndef _ROS_SERVICE_GetModelScans_h
#define _ROS_SERVICE_GetModelScans_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "household_objects_database_msgs/DatabaseReturnCode.h"
#include "household_objects_database_msgs/DatabaseScan.h"

namespace household_objects_database_msgs
{

static const char GETMODELSCANS[] = "household_objects_database_msgs/GetModelScans";

  class GetModelScansRequest : public ros::Msg
  {
    public:
      int32_t model_id;
      char * scan_source;

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
      uint32_t length_scan_source = strlen( (const char*) this->scan_source);
      memcpy(outbuffer + offset, &length_scan_source, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->scan_source, length_scan_source);
      offset += length_scan_source;
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
      uint32_t length_scan_source;
      memcpy(&length_scan_source, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_scan_source; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_scan_source-1]=0;
      this->scan_source = (char *)(inbuffer + offset-1);
      offset += length_scan_source;
     return offset;
    }

    const char * getType(){ return GETMODELSCANS; };
    const char * getMD5(){ return "4f31b0f27ba251f6d1f17eafced83cb7"; };

  };

  class GetModelScansResponse : public ros::Msg
  {
    public:
      household_objects_database_msgs::DatabaseReturnCode return_code;
      uint8_t matching_scans_length;
      household_objects_database_msgs::DatabaseScan st_matching_scans;
      household_objects_database_msgs::DatabaseScan * matching_scans;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->return_code.serialize(outbuffer + offset);
      *(outbuffer + offset++) = matching_scans_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < matching_scans_length; i++){
      offset += this->matching_scans[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->return_code.deserialize(inbuffer + offset);
      uint8_t matching_scans_lengthT = *(inbuffer + offset++);
      if(matching_scans_lengthT > matching_scans_length)
        this->matching_scans = (household_objects_database_msgs::DatabaseScan*)realloc(this->matching_scans, matching_scans_lengthT * sizeof(household_objects_database_msgs::DatabaseScan));
      offset += 3;
      matching_scans_length = matching_scans_lengthT;
      for( uint8_t i = 0; i < matching_scans_length; i++){
      offset += this->st_matching_scans.deserialize(inbuffer + offset);
        memcpy( &(this->matching_scans[i]), &(this->st_matching_scans), sizeof(household_objects_database_msgs::DatabaseScan));
      }
     return offset;
    }

    const char * getType(){ return GETMODELSCANS; };
    const char * getMD5(){ return "8d1bb6e95c26a5d891987d9d9195e958"; };

  };

  class GetModelScans {
    public:
    typedef GetModelScansRequest Request;
    typedef GetModelScansResponse Response;
  };

}
#endif
