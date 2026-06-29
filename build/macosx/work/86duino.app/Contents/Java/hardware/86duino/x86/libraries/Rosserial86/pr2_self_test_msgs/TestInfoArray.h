#ifndef _ROS_pr2_self_test_msgs_TestInfoArray_h
#define _ROS_pr2_self_test_msgs_TestInfoArray_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "pr2_self_test_msgs/TestInfo.h"

namespace pr2_self_test_msgs
{

  class TestInfoArray : public ros::Msg
  {
    public:
      uint8_t data_length;
      pr2_self_test_msgs::TestInfo st_data;
      pr2_self_test_msgs::TestInfo * data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = data_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < data_length; i++){
      offset += this->data[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t data_lengthT = *(inbuffer + offset++);
      if(data_lengthT > data_length)
        this->data = (pr2_self_test_msgs::TestInfo*)realloc(this->data, data_lengthT * sizeof(pr2_self_test_msgs::TestInfo));
      offset += 3;
      data_length = data_lengthT;
      for( uint8_t i = 0; i < data_length; i++){
      offset += this->st_data.deserialize(inbuffer + offset);
        memcpy( &(this->data[i]), &(this->st_data), sizeof(pr2_self_test_msgs::TestInfo));
      }
     return offset;
    }

    const char * getType(){ return "pr2_self_test_msgs/TestInfoArray"; };
    const char * getMD5(){ return "1ed4c76da9520649c668cc322d33542a"; };

  };

}
#endif