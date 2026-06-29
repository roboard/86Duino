#ifndef _ROS_moveit_msgs_PlannerInterfaceDescription_h
#define _ROS_moveit_msgs_PlannerInterfaceDescription_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace moveit_msgs
{

  class PlannerInterfaceDescription : public ros::Msg
  {
    public:
      char * name;
      uint8_t planner_ids_length;
      char* st_planner_ids;
      char* * planner_ids;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
      *(outbuffer + offset++) = planner_ids_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < planner_ids_length; i++){
      uint32_t length_planner_idsi = strlen( (const char*) this->planner_ids[i]);
      memcpy(outbuffer + offset, &length_planner_idsi, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->planner_ids[i], length_planner_idsi);
      offset += length_planner_idsi;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t length_name;
      memcpy(&length_name, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_name; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_name-1]=0;
      this->name = (char *)(inbuffer + offset-1);
      offset += length_name;
      uint8_t planner_ids_lengthT = *(inbuffer + offset++);
      if(planner_ids_lengthT > planner_ids_length)
        this->planner_ids = (char**)realloc(this->planner_ids, planner_ids_lengthT * sizeof(char*));
      offset += 3;
      planner_ids_length = planner_ids_lengthT;
      for( uint8_t i = 0; i < planner_ids_length; i++){
      uint32_t length_st_planner_ids;
      memcpy(&length_st_planner_ids, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_st_planner_ids; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_st_planner_ids-1]=0;
      this->st_planner_ids = (char *)(inbuffer + offset-1);
      offset += length_st_planner_ids;
        memcpy( &(this->planner_ids[i]), &(this->st_planner_ids), sizeof(char*));
      }
     return offset;
    }

    const char * getType(){ return "moveit_msgs/PlannerInterfaceDescription"; };
    const char * getMD5(){ return "ea5f6e6129aa1b110ccda9900d2bf25e"; };

  };

}
#endif