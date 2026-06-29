#ifndef _ROS_laser_cb_detector_ConfigGoal_h
#define _ROS_laser_cb_detector_ConfigGoal_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace laser_cb_detector
{

  class ConfigGoal : public ros::Msg
  {
    public:
      uint32_t num_x;
      uint32_t num_y;
      float spacing_x;
      float spacing_y;
      float width_scaling;
      float height_scaling;
      float min_intensity;
      float max_intensity;
      uint32_t subpixel_window;
      int32_t subpixel_zero_zone;
      uint8_t flip_horizontal;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      *(outbuffer + offset + 0) = (this->num_x >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->num_x >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->num_x >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->num_x >> (8 * 3)) & 0xFF;
      offset += sizeof(this->num_x);
      *(outbuffer + offset + 0) = (this->num_y >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->num_y >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->num_y >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->num_y >> (8 * 3)) & 0xFF;
      offset += sizeof(this->num_y);
      union {
        float real;
        uint32_t base;
      } u_spacing_x;
      u_spacing_x.real = this->spacing_x;
      *(outbuffer + offset + 0) = (u_spacing_x.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_spacing_x.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_spacing_x.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_spacing_x.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->spacing_x);
      union {
        float real;
        uint32_t base;
      } u_spacing_y;
      u_spacing_y.real = this->spacing_y;
      *(outbuffer + offset + 0) = (u_spacing_y.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_spacing_y.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_spacing_y.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_spacing_y.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->spacing_y);
      union {
        float real;
        uint32_t base;
      } u_width_scaling;
      u_width_scaling.real = this->width_scaling;
      *(outbuffer + offset + 0) = (u_width_scaling.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_width_scaling.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_width_scaling.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_width_scaling.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->width_scaling);
      union {
        float real;
        uint32_t base;
      } u_height_scaling;
      u_height_scaling.real = this->height_scaling;
      *(outbuffer + offset + 0) = (u_height_scaling.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_height_scaling.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_height_scaling.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_height_scaling.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->height_scaling);
      union {
        float real;
        uint32_t base;
      } u_min_intensity;
      u_min_intensity.real = this->min_intensity;
      *(outbuffer + offset + 0) = (u_min_intensity.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_min_intensity.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_min_intensity.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_min_intensity.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->min_intensity);
      union {
        float real;
        uint32_t base;
      } u_max_intensity;
      u_max_intensity.real = this->max_intensity;
      *(outbuffer + offset + 0) = (u_max_intensity.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_max_intensity.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_max_intensity.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_max_intensity.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->max_intensity);
      *(outbuffer + offset + 0) = (this->subpixel_window >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (this->subpixel_window >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (this->subpixel_window >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (this->subpixel_window >> (8 * 3)) & 0xFF;
      offset += sizeof(this->subpixel_window);
      union {
        int32_t real;
        uint32_t base;
      } u_subpixel_zero_zone;
      u_subpixel_zero_zone.real = this->subpixel_zero_zone;
      *(outbuffer + offset + 0) = (u_subpixel_zero_zone.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_subpixel_zero_zone.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_subpixel_zero_zone.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_subpixel_zero_zone.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->subpixel_zero_zone);
      *(outbuffer + offset + 0) = (this->flip_horizontal >> (8 * 0)) & 0xFF;
      offset += sizeof(this->flip_horizontal);
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      this->num_x =  ((uint32_t) (*(inbuffer + offset)));
      this->num_x |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->num_x |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->num_x |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->num_x);
      this->num_y =  ((uint32_t) (*(inbuffer + offset)));
      this->num_y |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->num_y |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->num_y |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->num_y);
      union {
        float real;
        uint32_t base;
      } u_spacing_x;
      u_spacing_x.base = 0;
      u_spacing_x.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_spacing_x.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_spacing_x.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_spacing_x.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->spacing_x = u_spacing_x.real;
      offset += sizeof(this->spacing_x);
      union {
        float real;
        uint32_t base;
      } u_spacing_y;
      u_spacing_y.base = 0;
      u_spacing_y.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_spacing_y.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_spacing_y.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_spacing_y.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->spacing_y = u_spacing_y.real;
      offset += sizeof(this->spacing_y);
      union {
        float real;
        uint32_t base;
      } u_width_scaling;
      u_width_scaling.base = 0;
      u_width_scaling.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_width_scaling.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_width_scaling.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_width_scaling.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->width_scaling = u_width_scaling.real;
      offset += sizeof(this->width_scaling);
      union {
        float real;
        uint32_t base;
      } u_height_scaling;
      u_height_scaling.base = 0;
      u_height_scaling.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_height_scaling.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_height_scaling.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_height_scaling.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->height_scaling = u_height_scaling.real;
      offset += sizeof(this->height_scaling);
      union {
        float real;
        uint32_t base;
      } u_min_intensity;
      u_min_intensity.base = 0;
      u_min_intensity.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_min_intensity.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_min_intensity.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_min_intensity.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->min_intensity = u_min_intensity.real;
      offset += sizeof(this->min_intensity);
      union {
        float real;
        uint32_t base;
      } u_max_intensity;
      u_max_intensity.base = 0;
      u_max_intensity.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_max_intensity.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_max_intensity.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_max_intensity.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->max_intensity = u_max_intensity.real;
      offset += sizeof(this->max_intensity);
      this->subpixel_window =  ((uint32_t) (*(inbuffer + offset)));
      this->subpixel_window |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      this->subpixel_window |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      this->subpixel_window |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      offset += sizeof(this->subpixel_window);
      union {
        int32_t real;
        uint32_t base;
      } u_subpixel_zero_zone;
      u_subpixel_zero_zone.base = 0;
      u_subpixel_zero_zone.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_subpixel_zero_zone.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_subpixel_zero_zone.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_subpixel_zero_zone.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->subpixel_zero_zone = u_subpixel_zero_zone.real;
      offset += sizeof(this->subpixel_zero_zone);
      this->flip_horizontal =  ((uint8_t) (*(inbuffer + offset)));
      offset += sizeof(this->flip_horizontal);
     return offset;
    }

    const char * getType(){ return "laser_cb_detector/ConfigGoal"; };
    const char * getMD5(){ return "d592564bc71ebb8458e3d0d3a079d731"; };

  };

}
#endif