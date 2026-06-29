#ifndef _ROS_SERVICE_GetAppDetails_h
#define _ROS_SERVICE_GetAppDetails_h
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "app_manager/ExchangeApp.h"

namespace app_manager
{

static const char GETAPPDETAILS[] = "app_manager/GetAppDetails";

  class GetAppDetailsRequest : public ros::Msg
  {
    public:
      char * name;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      uint32_t length_name = strlen( (const char*) this->name);
      memcpy(outbuffer + offset, &length_name, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->name, length_name);
      offset += length_name;
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
     return offset;
    }

    const char * getType(){ return GETAPPDETAILS; };
    const char * getMD5(){ return "c1f3d28f1b044c871e6eff2e9fc3c667"; };

  };

  class GetAppDetailsResponse : public ros::Msg
  {
    public:
      app_manager::ExchangeApp app;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->app.serialize(outbuffer + offset);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->app.deserialize(inbuffer + offset);
     return offset;
    }

    const char * getType(){ return GETAPPDETAILS; };
    const char * getMD5(){ return "404cd76612a719d24ac22fba2d495de8"; };

  };

  class GetAppDetails {
    public:
    typedef GetAppDetailsRequest Request;
    typedef GetAppDetailsResponse Response;
  };

}
#endif
