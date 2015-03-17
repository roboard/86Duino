#ifndef _ROS_object_recognition_msgs_Table_h
#define _ROS_object_recognition_msgs_Table_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "geometry_msgs/Pose.h"
#include "geometry_msgs/Point.h"

namespace object_recognition_msgs
{

  class Table : public ros::Msg
  {
    public:
      std_msgs::Header header;
      geometry_msgs::Pose pose;
      uint8_t convex_hull_length;
      geometry_msgs::Point st_convex_hull;
      geometry_msgs::Point * convex_hull;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->pose.serialize(outbuffer + offset);
      *(outbuffer + offset++) = convex_hull_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < convex_hull_length; i++){
      offset += this->convex_hull[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->pose.deserialize(inbuffer + offset);
      uint8_t convex_hull_lengthT = *(inbuffer + offset++);
      if(convex_hull_lengthT > convex_hull_length)
        this->convex_hull = (geometry_msgs::Point*)realloc(this->convex_hull, convex_hull_lengthT * sizeof(geometry_msgs::Point));
      offset += 3;
      convex_hull_length = convex_hull_lengthT;
      for( uint8_t i = 0; i < convex_hull_length; i++){
      offset += this->st_convex_hull.deserialize(inbuffer + offset);
        memcpy( &(this->convex_hull[i]), &(this->st_convex_hull), sizeof(geometry_msgs::Point));
      }
     return offset;
    }

    const char * getType(){ return "object_recognition_msgs/Table"; };
    const char * getMD5(){ return "39efebc7d51e44bd2d72f2df6c7823a2"; };

  };

}
#endif