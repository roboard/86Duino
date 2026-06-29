#ifndef _ROS_face_detector_FaceDetectorResult_h
#define _ROS_face_detector_FaceDetectorResult_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "people_msgs/PositionMeasurement.h"

namespace face_detector
{

  class FaceDetectorResult : public ros::Msg
  {
    public:
      uint8_t face_positions_length;
      people_msgs::PositionMeasurement st_face_positions;
      people_msgs::PositionMeasurement * face_positions;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset++) = face_positions_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < face_positions_length; i++){
      offset += this->face_positions[i].serialize(outbuffer + offset);
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint8_t face_positions_lengthT = *(inbuffer + offset++);
      if(face_positions_lengthT > face_positions_length)
        this->face_positions = (people_msgs::PositionMeasurement*)realloc(this->face_positions, face_positions_lengthT * sizeof(people_msgs::PositionMeasurement));
      offset += 3;
      face_positions_length = face_positions_lengthT;
      for( uint8_t i = 0; i < face_positions_length; i++){
      offset += this->st_face_positions.deserialize(inbuffer + offset);
        memcpy( &(this->face_positions[i]), &(this->st_face_positions), sizeof(people_msgs::PositionMeasurement));
      }
     return offset;
    }

    const char * getType(){ return "face_detector/FaceDetectorResult"; };
    const char * getMD5(){ return "b5dc843df183dbab7f0ab2f5ef5b6f9d"; };

  };

}
#endif