#ifndef _ROS_calibration_msgs_IntervalStamped_h
#define _ROS_calibration_msgs_IntervalStamped_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"
#include "calibration_msgs/Interval.h"

namespace calibration_msgs
{

  class IntervalStamped : public ros::Msg
  {
    public:
      std_msgs::Header header;
      calibration_msgs::Interval interval;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      offset += this->interval.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      offset += this->interval.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return "calibration_msgs/IntervalStamped"; };
    const char * getMD5(){ return "3b9fc1d72815ffa87542f8dc47ef7484"; };

  };

}
#endif