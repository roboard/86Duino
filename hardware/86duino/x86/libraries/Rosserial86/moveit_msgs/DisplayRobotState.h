#ifndef _ROS_moveit_msgs_DisplayRobotState_h
#define _ROS_moveit_msgs_DisplayRobotState_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "moveit_msgs/RobotState.h"
#include "moveit_msgs/ObjectColor.h"

namespace moveit_msgs
{

  class DisplayRobotState : public ros::Msg
  {
    public:
      moveit_msgs::RobotState state;
      uint8_t highlight_links_length;
      moveit_msgs::ObjectColor st_highlight_links;
      moveit_msgs::ObjectColor * highlight_links;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->state.serialize(outbuffer + offset);
      *(outbuffer + offset++) = highlight_links_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < highlight_links_length; i++){
      offset += this->highlight_links[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->state.deserialize(inbuffer + offset);
      uint8_t highlight_links_lengthT = *(inbuffer + offset++);
      if(highlight_links_lengthT > highlight_links_length)
        this->highlight_links = (moveit_msgs::ObjectColor*)realloc(this->highlight_links, highlight_links_lengthT * sizeof(moveit_msgs::ObjectColor));
      offset += 3;
      highlight_links_length = highlight_links_lengthT;
      for( uint8_t i = 0; i < highlight_links_length; i++){
      offset += this->st_highlight_links.deserialize(inbuffer + offset);
        memcpy( &(this->highlight_links[i]), &(this->st_highlight_links), sizeof(moveit_msgs::ObjectColor));
      }
     return offset;
    }

    const char * getType(){ return "moveit_msgs/DisplayRobotState"; };
    const char * getMD5(){ return "6a3bab3a33a8c47aee24481a455a21aa"; };

  };

}
#endif