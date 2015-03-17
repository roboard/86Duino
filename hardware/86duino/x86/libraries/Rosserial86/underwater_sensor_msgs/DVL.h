#ifndef _ROS_underwater_sensor_msgs_DVL_h
#define _ROS_underwater_sensor_msgs_DVL_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"
#include "std_msgs/Header.h"

namespace underwater_sensor_msgs
{

  class DVL : public ros::Msg
  {
    public:
      std_msgs::Header header;
      char * header_dvl;
      char * date;
      float salinity;
      float temperature;
      float depth;
      float sound_speed;
      int32_t test;
      float wi_x_axis;
      float wi_y_axis;
      float wi_z_axis;
      float wi_error;
      char * wi_status;
      float bi_x_axis;
      float bi_y_axis;
      float bi_z_axis;
      float bi_error;
      char * bi_status;
      float ws_transverse;
      float ws_longitudinal;
      float ws_normal;
      char * ws_status;
      float bs_transverse;
      float bs_longitudinal;
      float bs_normal;
      char * bs_status;
      float we_east;
      float we_north;
      float we_upwards;
      char * we_status;
      float be_east;
      float be_north;
      float be_upwards;
      char * be_status;
      float wd_east;
      float wd_north;
      float wd_upwards;
      float wd_range;
      float wd_time;
      float bd_east;
      float bd_north;
      float bd_upwards;
      float bd_range;
      float bd_time;
      char * raw_data;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      offset += this->header.serialize(outbuffer + offset);
      uint32_t length_header_dvl = strlen( (const char*) this->header_dvl);
      memcpy(outbuffer + offset, &length_header_dvl, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->header_dvl, length_header_dvl);
      offset += length_header_dvl;
      uint32_t length_date = strlen( (const char*) this->date);
      memcpy(outbuffer + offset, &length_date, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->date, length_date);
      offset += length_date;
      int32_t * val_salinity = (int32_t *) &(this->salinity);
      int32_t exp_salinity = (((*val_salinity)>>23)&255);
      if(exp_salinity != 0)
        exp_salinity += 1023-127;
      int32_t sig_salinity = *val_salinity;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_salinity<<5) & 0xff;
      *(outbuffer + offset++) = (sig_salinity>>3) & 0xff;
      *(outbuffer + offset++) = (sig_salinity>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_salinity<<4) & 0xF0) | ((sig_salinity>>19)&0x0F);
      *(outbuffer + offset++) = (exp_salinity>>4) & 0x7F;
      if(this->salinity < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_temperature = (int32_t *) &(this->temperature);
      int32_t exp_temperature = (((*val_temperature)>>23)&255);
      if(exp_temperature != 0)
        exp_temperature += 1023-127;
      int32_t sig_temperature = *val_temperature;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_temperature<<5) & 0xff;
      *(outbuffer + offset++) = (sig_temperature>>3) & 0xff;
      *(outbuffer + offset++) = (sig_temperature>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_temperature<<4) & 0xF0) | ((sig_temperature>>19)&0x0F);
      *(outbuffer + offset++) = (exp_temperature>>4) & 0x7F;
      if(this->temperature < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_depth = (int32_t *) &(this->depth);
      int32_t exp_depth = (((*val_depth)>>23)&255);
      if(exp_depth != 0)
        exp_depth += 1023-127;
      int32_t sig_depth = *val_depth;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_depth<<5) & 0xff;
      *(outbuffer + offset++) = (sig_depth>>3) & 0xff;
      *(outbuffer + offset++) = (sig_depth>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_depth<<4) & 0xF0) | ((sig_depth>>19)&0x0F);
      *(outbuffer + offset++) = (exp_depth>>4) & 0x7F;
      if(this->depth < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_sound_speed = (int32_t *) &(this->sound_speed);
      int32_t exp_sound_speed = (((*val_sound_speed)>>23)&255);
      if(exp_sound_speed != 0)
        exp_sound_speed += 1023-127;
      int32_t sig_sound_speed = *val_sound_speed;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_sound_speed<<5) & 0xff;
      *(outbuffer + offset++) = (sig_sound_speed>>3) & 0xff;
      *(outbuffer + offset++) = (sig_sound_speed>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_sound_speed<<4) & 0xF0) | ((sig_sound_speed>>19)&0x0F);
      *(outbuffer + offset++) = (exp_sound_speed>>4) & 0x7F;
      if(this->sound_speed < 0) *(outbuffer + offset -1) |= 0x80;
      union {
        int32_t real;
        uint32_t base;
      } u_test;
      u_test.real = this->test;
      *(outbuffer + offset + 0) = (u_test.base >> (8 * 0)) & 0xFF;
      *(outbuffer + offset + 1) = (u_test.base >> (8 * 1)) & 0xFF;
      *(outbuffer + offset + 2) = (u_test.base >> (8 * 2)) & 0xFF;
      *(outbuffer + offset + 3) = (u_test.base >> (8 * 3)) & 0xFF;
      offset += sizeof(this->test);
      int32_t * val_wi_x_axis = (int32_t *) &(this->wi_x_axis);
      int32_t exp_wi_x_axis = (((*val_wi_x_axis)>>23)&255);
      if(exp_wi_x_axis != 0)
        exp_wi_x_axis += 1023-127;
      int32_t sig_wi_x_axis = *val_wi_x_axis;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wi_x_axis<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wi_x_axis>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wi_x_axis>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wi_x_axis<<4) & 0xF0) | ((sig_wi_x_axis>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wi_x_axis>>4) & 0x7F;
      if(this->wi_x_axis < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_wi_y_axis = (int32_t *) &(this->wi_y_axis);
      int32_t exp_wi_y_axis = (((*val_wi_y_axis)>>23)&255);
      if(exp_wi_y_axis != 0)
        exp_wi_y_axis += 1023-127;
      int32_t sig_wi_y_axis = *val_wi_y_axis;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wi_y_axis<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wi_y_axis>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wi_y_axis>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wi_y_axis<<4) & 0xF0) | ((sig_wi_y_axis>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wi_y_axis>>4) & 0x7F;
      if(this->wi_y_axis < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_wi_z_axis = (int32_t *) &(this->wi_z_axis);
      int32_t exp_wi_z_axis = (((*val_wi_z_axis)>>23)&255);
      if(exp_wi_z_axis != 0)
        exp_wi_z_axis += 1023-127;
      int32_t sig_wi_z_axis = *val_wi_z_axis;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wi_z_axis<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wi_z_axis>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wi_z_axis>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wi_z_axis<<4) & 0xF0) | ((sig_wi_z_axis>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wi_z_axis>>4) & 0x7F;
      if(this->wi_z_axis < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_wi_error = (int32_t *) &(this->wi_error);
      int32_t exp_wi_error = (((*val_wi_error)>>23)&255);
      if(exp_wi_error != 0)
        exp_wi_error += 1023-127;
      int32_t sig_wi_error = *val_wi_error;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wi_error<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wi_error>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wi_error>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wi_error<<4) & 0xF0) | ((sig_wi_error>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wi_error>>4) & 0x7F;
      if(this->wi_error < 0) *(outbuffer + offset -1) |= 0x80;
      uint32_t length_wi_status = strlen( (const char*) this->wi_status);
      memcpy(outbuffer + offset, &length_wi_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->wi_status, length_wi_status);
      offset += length_wi_status;
      int32_t * val_bi_x_axis = (int32_t *) &(this->bi_x_axis);
      int32_t exp_bi_x_axis = (((*val_bi_x_axis)>>23)&255);
      if(exp_bi_x_axis != 0)
        exp_bi_x_axis += 1023-127;
      int32_t sig_bi_x_axis = *val_bi_x_axis;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bi_x_axis<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bi_x_axis>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bi_x_axis>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bi_x_axis<<4) & 0xF0) | ((sig_bi_x_axis>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bi_x_axis>>4) & 0x7F;
      if(this->bi_x_axis < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bi_y_axis = (int32_t *) &(this->bi_y_axis);
      int32_t exp_bi_y_axis = (((*val_bi_y_axis)>>23)&255);
      if(exp_bi_y_axis != 0)
        exp_bi_y_axis += 1023-127;
      int32_t sig_bi_y_axis = *val_bi_y_axis;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bi_y_axis<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bi_y_axis>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bi_y_axis>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bi_y_axis<<4) & 0xF0) | ((sig_bi_y_axis>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bi_y_axis>>4) & 0x7F;
      if(this->bi_y_axis < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bi_z_axis = (int32_t *) &(this->bi_z_axis);
      int32_t exp_bi_z_axis = (((*val_bi_z_axis)>>23)&255);
      if(exp_bi_z_axis != 0)
        exp_bi_z_axis += 1023-127;
      int32_t sig_bi_z_axis = *val_bi_z_axis;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bi_z_axis<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bi_z_axis>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bi_z_axis>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bi_z_axis<<4) & 0xF0) | ((sig_bi_z_axis>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bi_z_axis>>4) & 0x7F;
      if(this->bi_z_axis < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bi_error = (int32_t *) &(this->bi_error);
      int32_t exp_bi_error = (((*val_bi_error)>>23)&255);
      if(exp_bi_error != 0)
        exp_bi_error += 1023-127;
      int32_t sig_bi_error = *val_bi_error;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bi_error<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bi_error>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bi_error>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bi_error<<4) & 0xF0) | ((sig_bi_error>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bi_error>>4) & 0x7F;
      if(this->bi_error < 0) *(outbuffer + offset -1) |= 0x80;
      uint32_t length_bi_status = strlen( (const char*) this->bi_status);
      memcpy(outbuffer + offset, &length_bi_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->bi_status, length_bi_status);
      offset += length_bi_status;
      int32_t * val_ws_transverse = (int32_t *) &(this->ws_transverse);
      int32_t exp_ws_transverse = (((*val_ws_transverse)>>23)&255);
      if(exp_ws_transverse != 0)
        exp_ws_transverse += 1023-127;
      int32_t sig_ws_transverse = *val_ws_transverse;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_ws_transverse<<5) & 0xff;
      *(outbuffer + offset++) = (sig_ws_transverse>>3) & 0xff;
      *(outbuffer + offset++) = (sig_ws_transverse>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_ws_transverse<<4) & 0xF0) | ((sig_ws_transverse>>19)&0x0F);
      *(outbuffer + offset++) = (exp_ws_transverse>>4) & 0x7F;
      if(this->ws_transverse < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_ws_longitudinal = (int32_t *) &(this->ws_longitudinal);
      int32_t exp_ws_longitudinal = (((*val_ws_longitudinal)>>23)&255);
      if(exp_ws_longitudinal != 0)
        exp_ws_longitudinal += 1023-127;
      int32_t sig_ws_longitudinal = *val_ws_longitudinal;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_ws_longitudinal<<5) & 0xff;
      *(outbuffer + offset++) = (sig_ws_longitudinal>>3) & 0xff;
      *(outbuffer + offset++) = (sig_ws_longitudinal>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_ws_longitudinal<<4) & 0xF0) | ((sig_ws_longitudinal>>19)&0x0F);
      *(outbuffer + offset++) = (exp_ws_longitudinal>>4) & 0x7F;
      if(this->ws_longitudinal < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_ws_normal = (int32_t *) &(this->ws_normal);
      int32_t exp_ws_normal = (((*val_ws_normal)>>23)&255);
      if(exp_ws_normal != 0)
        exp_ws_normal += 1023-127;
      int32_t sig_ws_normal = *val_ws_normal;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_ws_normal<<5) & 0xff;
      *(outbuffer + offset++) = (sig_ws_normal>>3) & 0xff;
      *(outbuffer + offset++) = (sig_ws_normal>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_ws_normal<<4) & 0xF0) | ((sig_ws_normal>>19)&0x0F);
      *(outbuffer + offset++) = (exp_ws_normal>>4) & 0x7F;
      if(this->ws_normal < 0) *(outbuffer + offset -1) |= 0x80;
      uint32_t length_ws_status = strlen( (const char*) this->ws_status);
      memcpy(outbuffer + offset, &length_ws_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->ws_status, length_ws_status);
      offset += length_ws_status;
      int32_t * val_bs_transverse = (int32_t *) &(this->bs_transverse);
      int32_t exp_bs_transverse = (((*val_bs_transverse)>>23)&255);
      if(exp_bs_transverse != 0)
        exp_bs_transverse += 1023-127;
      int32_t sig_bs_transverse = *val_bs_transverse;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bs_transverse<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bs_transverse>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bs_transverse>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bs_transverse<<4) & 0xF0) | ((sig_bs_transverse>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bs_transverse>>4) & 0x7F;
      if(this->bs_transverse < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bs_longitudinal = (int32_t *) &(this->bs_longitudinal);
      int32_t exp_bs_longitudinal = (((*val_bs_longitudinal)>>23)&255);
      if(exp_bs_longitudinal != 0)
        exp_bs_longitudinal += 1023-127;
      int32_t sig_bs_longitudinal = *val_bs_longitudinal;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bs_longitudinal<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bs_longitudinal>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bs_longitudinal>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bs_longitudinal<<4) & 0xF0) | ((sig_bs_longitudinal>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bs_longitudinal>>4) & 0x7F;
      if(this->bs_longitudinal < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bs_normal = (int32_t *) &(this->bs_normal);
      int32_t exp_bs_normal = (((*val_bs_normal)>>23)&255);
      if(exp_bs_normal != 0)
        exp_bs_normal += 1023-127;
      int32_t sig_bs_normal = *val_bs_normal;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bs_normal<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bs_normal>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bs_normal>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bs_normal<<4) & 0xF0) | ((sig_bs_normal>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bs_normal>>4) & 0x7F;
      if(this->bs_normal < 0) *(outbuffer + offset -1) |= 0x80;
      uint32_t length_bs_status = strlen( (const char*) this->bs_status);
      memcpy(outbuffer + offset, &length_bs_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->bs_status, length_bs_status);
      offset += length_bs_status;
      int32_t * val_we_east = (int32_t *) &(this->we_east);
      int32_t exp_we_east = (((*val_we_east)>>23)&255);
      if(exp_we_east != 0)
        exp_we_east += 1023-127;
      int32_t sig_we_east = *val_we_east;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_we_east<<5) & 0xff;
      *(outbuffer + offset++) = (sig_we_east>>3) & 0xff;
      *(outbuffer + offset++) = (sig_we_east>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_we_east<<4) & 0xF0) | ((sig_we_east>>19)&0x0F);
      *(outbuffer + offset++) = (exp_we_east>>4) & 0x7F;
      if(this->we_east < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_we_north = (int32_t *) &(this->we_north);
      int32_t exp_we_north = (((*val_we_north)>>23)&255);
      if(exp_we_north != 0)
        exp_we_north += 1023-127;
      int32_t sig_we_north = *val_we_north;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_we_north<<5) & 0xff;
      *(outbuffer + offset++) = (sig_we_north>>3) & 0xff;
      *(outbuffer + offset++) = (sig_we_north>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_we_north<<4) & 0xF0) | ((sig_we_north>>19)&0x0F);
      *(outbuffer + offset++) = (exp_we_north>>4) & 0x7F;
      if(this->we_north < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_we_upwards = (int32_t *) &(this->we_upwards);
      int32_t exp_we_upwards = (((*val_we_upwards)>>23)&255);
      if(exp_we_upwards != 0)
        exp_we_upwards += 1023-127;
      int32_t sig_we_upwards = *val_we_upwards;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_we_upwards<<5) & 0xff;
      *(outbuffer + offset++) = (sig_we_upwards>>3) & 0xff;
      *(outbuffer + offset++) = (sig_we_upwards>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_we_upwards<<4) & 0xF0) | ((sig_we_upwards>>19)&0x0F);
      *(outbuffer + offset++) = (exp_we_upwards>>4) & 0x7F;
      if(this->we_upwards < 0) *(outbuffer + offset -1) |= 0x80;
      uint32_t length_we_status = strlen( (const char*) this->we_status);
      memcpy(outbuffer + offset, &length_we_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->we_status, length_we_status);
      offset += length_we_status;
      int32_t * val_be_east = (int32_t *) &(this->be_east);
      int32_t exp_be_east = (((*val_be_east)>>23)&255);
      if(exp_be_east != 0)
        exp_be_east += 1023-127;
      int32_t sig_be_east = *val_be_east;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_be_east<<5) & 0xff;
      *(outbuffer + offset++) = (sig_be_east>>3) & 0xff;
      *(outbuffer + offset++) = (sig_be_east>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_be_east<<4) & 0xF0) | ((sig_be_east>>19)&0x0F);
      *(outbuffer + offset++) = (exp_be_east>>4) & 0x7F;
      if(this->be_east < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_be_north = (int32_t *) &(this->be_north);
      int32_t exp_be_north = (((*val_be_north)>>23)&255);
      if(exp_be_north != 0)
        exp_be_north += 1023-127;
      int32_t sig_be_north = *val_be_north;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_be_north<<5) & 0xff;
      *(outbuffer + offset++) = (sig_be_north>>3) & 0xff;
      *(outbuffer + offset++) = (sig_be_north>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_be_north<<4) & 0xF0) | ((sig_be_north>>19)&0x0F);
      *(outbuffer + offset++) = (exp_be_north>>4) & 0x7F;
      if(this->be_north < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_be_upwards = (int32_t *) &(this->be_upwards);
      int32_t exp_be_upwards = (((*val_be_upwards)>>23)&255);
      if(exp_be_upwards != 0)
        exp_be_upwards += 1023-127;
      int32_t sig_be_upwards = *val_be_upwards;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_be_upwards<<5) & 0xff;
      *(outbuffer + offset++) = (sig_be_upwards>>3) & 0xff;
      *(outbuffer + offset++) = (sig_be_upwards>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_be_upwards<<4) & 0xF0) | ((sig_be_upwards>>19)&0x0F);
      *(outbuffer + offset++) = (exp_be_upwards>>4) & 0x7F;
      if(this->be_upwards < 0) *(outbuffer + offset -1) |= 0x80;
      uint32_t length_be_status = strlen( (const char*) this->be_status);
      memcpy(outbuffer + offset, &length_be_status, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->be_status, length_be_status);
      offset += length_be_status;
      int32_t * val_wd_east = (int32_t *) &(this->wd_east);
      int32_t exp_wd_east = (((*val_wd_east)>>23)&255);
      if(exp_wd_east != 0)
        exp_wd_east += 1023-127;
      int32_t sig_wd_east = *val_wd_east;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wd_east<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wd_east>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wd_east>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wd_east<<4) & 0xF0) | ((sig_wd_east>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wd_east>>4) & 0x7F;
      if(this->wd_east < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_wd_north = (int32_t *) &(this->wd_north);
      int32_t exp_wd_north = (((*val_wd_north)>>23)&255);
      if(exp_wd_north != 0)
        exp_wd_north += 1023-127;
      int32_t sig_wd_north = *val_wd_north;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wd_north<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wd_north>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wd_north>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wd_north<<4) & 0xF0) | ((sig_wd_north>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wd_north>>4) & 0x7F;
      if(this->wd_north < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_wd_upwards = (int32_t *) &(this->wd_upwards);
      int32_t exp_wd_upwards = (((*val_wd_upwards)>>23)&255);
      if(exp_wd_upwards != 0)
        exp_wd_upwards += 1023-127;
      int32_t sig_wd_upwards = *val_wd_upwards;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wd_upwards<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wd_upwards>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wd_upwards>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wd_upwards<<4) & 0xF0) | ((sig_wd_upwards>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wd_upwards>>4) & 0x7F;
      if(this->wd_upwards < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_wd_range = (int32_t *) &(this->wd_range);
      int32_t exp_wd_range = (((*val_wd_range)>>23)&255);
      if(exp_wd_range != 0)
        exp_wd_range += 1023-127;
      int32_t sig_wd_range = *val_wd_range;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wd_range<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wd_range>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wd_range>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wd_range<<4) & 0xF0) | ((sig_wd_range>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wd_range>>4) & 0x7F;
      if(this->wd_range < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_wd_time = (int32_t *) &(this->wd_time);
      int32_t exp_wd_time = (((*val_wd_time)>>23)&255);
      if(exp_wd_time != 0)
        exp_wd_time += 1023-127;
      int32_t sig_wd_time = *val_wd_time;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_wd_time<<5) & 0xff;
      *(outbuffer + offset++) = (sig_wd_time>>3) & 0xff;
      *(outbuffer + offset++) = (sig_wd_time>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_wd_time<<4) & 0xF0) | ((sig_wd_time>>19)&0x0F);
      *(outbuffer + offset++) = (exp_wd_time>>4) & 0x7F;
      if(this->wd_time < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bd_east = (int32_t *) &(this->bd_east);
      int32_t exp_bd_east = (((*val_bd_east)>>23)&255);
      if(exp_bd_east != 0)
        exp_bd_east += 1023-127;
      int32_t sig_bd_east = *val_bd_east;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bd_east<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bd_east>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bd_east>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bd_east<<4) & 0xF0) | ((sig_bd_east>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bd_east>>4) & 0x7F;
      if(this->bd_east < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bd_north = (int32_t *) &(this->bd_north);
      int32_t exp_bd_north = (((*val_bd_north)>>23)&255);
      if(exp_bd_north != 0)
        exp_bd_north += 1023-127;
      int32_t sig_bd_north = *val_bd_north;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bd_north<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bd_north>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bd_north>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bd_north<<4) & 0xF0) | ((sig_bd_north>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bd_north>>4) & 0x7F;
      if(this->bd_north < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bd_upwards = (int32_t *) &(this->bd_upwards);
      int32_t exp_bd_upwards = (((*val_bd_upwards)>>23)&255);
      if(exp_bd_upwards != 0)
        exp_bd_upwards += 1023-127;
      int32_t sig_bd_upwards = *val_bd_upwards;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bd_upwards<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bd_upwards>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bd_upwards>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bd_upwards<<4) & 0xF0) | ((sig_bd_upwards>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bd_upwards>>4) & 0x7F;
      if(this->bd_upwards < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bd_range = (int32_t *) &(this->bd_range);
      int32_t exp_bd_range = (((*val_bd_range)>>23)&255);
      if(exp_bd_range != 0)
        exp_bd_range += 1023-127;
      int32_t sig_bd_range = *val_bd_range;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bd_range<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bd_range>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bd_range>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bd_range<<4) & 0xF0) | ((sig_bd_range>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bd_range>>4) & 0x7F;
      if(this->bd_range < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_bd_time = (int32_t *) &(this->bd_time);
      int32_t exp_bd_time = (((*val_bd_time)>>23)&255);
      if(exp_bd_time != 0)
        exp_bd_time += 1023-127;
      int32_t sig_bd_time = *val_bd_time;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_bd_time<<5) & 0xff;
      *(outbuffer + offset++) = (sig_bd_time>>3) & 0xff;
      *(outbuffer + offset++) = (sig_bd_time>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_bd_time<<4) & 0xF0) | ((sig_bd_time>>19)&0x0F);
      *(outbuffer + offset++) = (exp_bd_time>>4) & 0x7F;
      if(this->bd_time < 0) *(outbuffer + offset -1) |= 0x80;
      uint32_t length_raw_data = strlen( (const char*) this->raw_data);
      memcpy(outbuffer + offset, &length_raw_data, sizeof(uint32_t));
      offset += 4;
      memcpy(outbuffer + offset, this->raw_data, length_raw_data);
      offset += length_raw_data;
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      offset += this->header.deserialize(inbuffer + offset);
      uint32_t length_header_dvl;
      memcpy(&length_header_dvl, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_header_dvl; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_header_dvl-1]=0;
      this->header_dvl = (char *)(inbuffer + offset-1);
      offset += length_header_dvl;
      uint32_t length_date;
      memcpy(&length_date, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_date; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_date-1]=0;
      this->date = (char *)(inbuffer + offset-1);
      offset += length_date;
      uint32_t * val_salinity = (uint32_t*) &(this->salinity);
      offset += 3;
      *val_salinity = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_salinity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_salinity |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_salinity |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_salinity = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_salinity |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_salinity !=0)
        *val_salinity |= ((exp_salinity)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->salinity = -this->salinity;
      uint32_t * val_temperature = (uint32_t*) &(this->temperature);
      offset += 3;
      *val_temperature = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_temperature |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_temperature |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_temperature |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_temperature = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_temperature |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_temperature !=0)
        *val_temperature |= ((exp_temperature)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->temperature = -this->temperature;
      uint32_t * val_depth = (uint32_t*) &(this->depth);
      offset += 3;
      *val_depth = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_depth |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_depth |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_depth |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_depth = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_depth |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_depth !=0)
        *val_depth |= ((exp_depth)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->depth = -this->depth;
      uint32_t * val_sound_speed = (uint32_t*) &(this->sound_speed);
      offset += 3;
      *val_sound_speed = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_sound_speed |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_sound_speed |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_sound_speed |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_sound_speed = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_sound_speed |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_sound_speed !=0)
        *val_sound_speed |= ((exp_sound_speed)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->sound_speed = -this->sound_speed;
      union {
        int32_t real;
        uint32_t base;
      } u_test;
      u_test.base = 0;
      u_test.base |= ((uint32_t) (*(inbuffer + offset + 0))) << (8 * 0);
      u_test.base |= ((uint32_t) (*(inbuffer + offset + 1))) << (8 * 1);
      u_test.base |= ((uint32_t) (*(inbuffer + offset + 2))) << (8 * 2);
      u_test.base |= ((uint32_t) (*(inbuffer + offset + 3))) << (8 * 3);
      this->test = u_test.real;
      offset += sizeof(this->test);
      uint32_t * val_wi_x_axis = (uint32_t*) &(this->wi_x_axis);
      offset += 3;
      *val_wi_x_axis = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wi_x_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wi_x_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wi_x_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wi_x_axis = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wi_x_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wi_x_axis !=0)
        *val_wi_x_axis |= ((exp_wi_x_axis)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wi_x_axis = -this->wi_x_axis;
      uint32_t * val_wi_y_axis = (uint32_t*) &(this->wi_y_axis);
      offset += 3;
      *val_wi_y_axis = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wi_y_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wi_y_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wi_y_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wi_y_axis = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wi_y_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wi_y_axis !=0)
        *val_wi_y_axis |= ((exp_wi_y_axis)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wi_y_axis = -this->wi_y_axis;
      uint32_t * val_wi_z_axis = (uint32_t*) &(this->wi_z_axis);
      offset += 3;
      *val_wi_z_axis = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wi_z_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wi_z_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wi_z_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wi_z_axis = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wi_z_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wi_z_axis !=0)
        *val_wi_z_axis |= ((exp_wi_z_axis)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wi_z_axis = -this->wi_z_axis;
      uint32_t * val_wi_error = (uint32_t*) &(this->wi_error);
      offset += 3;
      *val_wi_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wi_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wi_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wi_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wi_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wi_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wi_error !=0)
        *val_wi_error |= ((exp_wi_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wi_error = -this->wi_error;
      uint32_t length_wi_status;
      memcpy(&length_wi_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_wi_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_wi_status-1]=0;
      this->wi_status = (char *)(inbuffer + offset-1);
      offset += length_wi_status;
      uint32_t * val_bi_x_axis = (uint32_t*) &(this->bi_x_axis);
      offset += 3;
      *val_bi_x_axis = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bi_x_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bi_x_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bi_x_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bi_x_axis = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bi_x_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bi_x_axis !=0)
        *val_bi_x_axis |= ((exp_bi_x_axis)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bi_x_axis = -this->bi_x_axis;
      uint32_t * val_bi_y_axis = (uint32_t*) &(this->bi_y_axis);
      offset += 3;
      *val_bi_y_axis = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bi_y_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bi_y_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bi_y_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bi_y_axis = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bi_y_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bi_y_axis !=0)
        *val_bi_y_axis |= ((exp_bi_y_axis)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bi_y_axis = -this->bi_y_axis;
      uint32_t * val_bi_z_axis = (uint32_t*) &(this->bi_z_axis);
      offset += 3;
      *val_bi_z_axis = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bi_z_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bi_z_axis |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bi_z_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bi_z_axis = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bi_z_axis |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bi_z_axis !=0)
        *val_bi_z_axis |= ((exp_bi_z_axis)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bi_z_axis = -this->bi_z_axis;
      uint32_t * val_bi_error = (uint32_t*) &(this->bi_error);
      offset += 3;
      *val_bi_error = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bi_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bi_error |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bi_error |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bi_error = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bi_error |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bi_error !=0)
        *val_bi_error |= ((exp_bi_error)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bi_error = -this->bi_error;
      uint32_t length_bi_status;
      memcpy(&length_bi_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_bi_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_bi_status-1]=0;
      this->bi_status = (char *)(inbuffer + offset-1);
      offset += length_bi_status;
      uint32_t * val_ws_transverse = (uint32_t*) &(this->ws_transverse);
      offset += 3;
      *val_ws_transverse = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_ws_transverse |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_ws_transverse |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_ws_transverse |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_ws_transverse = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_ws_transverse |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_ws_transverse !=0)
        *val_ws_transverse |= ((exp_ws_transverse)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->ws_transverse = -this->ws_transverse;
      uint32_t * val_ws_longitudinal = (uint32_t*) &(this->ws_longitudinal);
      offset += 3;
      *val_ws_longitudinal = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_ws_longitudinal |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_ws_longitudinal |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_ws_longitudinal |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_ws_longitudinal = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_ws_longitudinal |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_ws_longitudinal !=0)
        *val_ws_longitudinal |= ((exp_ws_longitudinal)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->ws_longitudinal = -this->ws_longitudinal;
      uint32_t * val_ws_normal = (uint32_t*) &(this->ws_normal);
      offset += 3;
      *val_ws_normal = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_ws_normal |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_ws_normal |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_ws_normal |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_ws_normal = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_ws_normal |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_ws_normal !=0)
        *val_ws_normal |= ((exp_ws_normal)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->ws_normal = -this->ws_normal;
      uint32_t length_ws_status;
      memcpy(&length_ws_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_ws_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_ws_status-1]=0;
      this->ws_status = (char *)(inbuffer + offset-1);
      offset += length_ws_status;
      uint32_t * val_bs_transverse = (uint32_t*) &(this->bs_transverse);
      offset += 3;
      *val_bs_transverse = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bs_transverse |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bs_transverse |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bs_transverse |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bs_transverse = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bs_transverse |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bs_transverse !=0)
        *val_bs_transverse |= ((exp_bs_transverse)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bs_transverse = -this->bs_transverse;
      uint32_t * val_bs_longitudinal = (uint32_t*) &(this->bs_longitudinal);
      offset += 3;
      *val_bs_longitudinal = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bs_longitudinal |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bs_longitudinal |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bs_longitudinal |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bs_longitudinal = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bs_longitudinal |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bs_longitudinal !=0)
        *val_bs_longitudinal |= ((exp_bs_longitudinal)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bs_longitudinal = -this->bs_longitudinal;
      uint32_t * val_bs_normal = (uint32_t*) &(this->bs_normal);
      offset += 3;
      *val_bs_normal = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bs_normal |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bs_normal |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bs_normal |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bs_normal = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bs_normal |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bs_normal !=0)
        *val_bs_normal |= ((exp_bs_normal)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bs_normal = -this->bs_normal;
      uint32_t length_bs_status;
      memcpy(&length_bs_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_bs_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_bs_status-1]=0;
      this->bs_status = (char *)(inbuffer + offset-1);
      offset += length_bs_status;
      uint32_t * val_we_east = (uint32_t*) &(this->we_east);
      offset += 3;
      *val_we_east = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_we_east |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_we_east |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_we_east |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_we_east = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_we_east |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_we_east !=0)
        *val_we_east |= ((exp_we_east)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->we_east = -this->we_east;
      uint32_t * val_we_north = (uint32_t*) &(this->we_north);
      offset += 3;
      *val_we_north = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_we_north |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_we_north |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_we_north |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_we_north = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_we_north |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_we_north !=0)
        *val_we_north |= ((exp_we_north)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->we_north = -this->we_north;
      uint32_t * val_we_upwards = (uint32_t*) &(this->we_upwards);
      offset += 3;
      *val_we_upwards = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_we_upwards |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_we_upwards |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_we_upwards |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_we_upwards = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_we_upwards |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_we_upwards !=0)
        *val_we_upwards |= ((exp_we_upwards)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->we_upwards = -this->we_upwards;
      uint32_t length_we_status;
      memcpy(&length_we_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_we_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_we_status-1]=0;
      this->we_status = (char *)(inbuffer + offset-1);
      offset += length_we_status;
      uint32_t * val_be_east = (uint32_t*) &(this->be_east);
      offset += 3;
      *val_be_east = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_be_east |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_be_east |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_be_east |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_be_east = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_be_east |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_be_east !=0)
        *val_be_east |= ((exp_be_east)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->be_east = -this->be_east;
      uint32_t * val_be_north = (uint32_t*) &(this->be_north);
      offset += 3;
      *val_be_north = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_be_north |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_be_north |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_be_north |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_be_north = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_be_north |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_be_north !=0)
        *val_be_north |= ((exp_be_north)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->be_north = -this->be_north;
      uint32_t * val_be_upwards = (uint32_t*) &(this->be_upwards);
      offset += 3;
      *val_be_upwards = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_be_upwards |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_be_upwards |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_be_upwards |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_be_upwards = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_be_upwards |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_be_upwards !=0)
        *val_be_upwards |= ((exp_be_upwards)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->be_upwards = -this->be_upwards;
      uint32_t length_be_status;
      memcpy(&length_be_status, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_be_status; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_be_status-1]=0;
      this->be_status = (char *)(inbuffer + offset-1);
      offset += length_be_status;
      uint32_t * val_wd_east = (uint32_t*) &(this->wd_east);
      offset += 3;
      *val_wd_east = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wd_east |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wd_east |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wd_east |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wd_east = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wd_east |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wd_east !=0)
        *val_wd_east |= ((exp_wd_east)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wd_east = -this->wd_east;
      uint32_t * val_wd_north = (uint32_t*) &(this->wd_north);
      offset += 3;
      *val_wd_north = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wd_north |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wd_north |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wd_north |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wd_north = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wd_north |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wd_north !=0)
        *val_wd_north |= ((exp_wd_north)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wd_north = -this->wd_north;
      uint32_t * val_wd_upwards = (uint32_t*) &(this->wd_upwards);
      offset += 3;
      *val_wd_upwards = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wd_upwards |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wd_upwards |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wd_upwards |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wd_upwards = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wd_upwards |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wd_upwards !=0)
        *val_wd_upwards |= ((exp_wd_upwards)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wd_upwards = -this->wd_upwards;
      uint32_t * val_wd_range = (uint32_t*) &(this->wd_range);
      offset += 3;
      *val_wd_range = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wd_range |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wd_range |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wd_range |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wd_range = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wd_range |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wd_range !=0)
        *val_wd_range |= ((exp_wd_range)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wd_range = -this->wd_range;
      uint32_t * val_wd_time = (uint32_t*) &(this->wd_time);
      offset += 3;
      *val_wd_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_wd_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_wd_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_wd_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_wd_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_wd_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_wd_time !=0)
        *val_wd_time |= ((exp_wd_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->wd_time = -this->wd_time;
      uint32_t * val_bd_east = (uint32_t*) &(this->bd_east);
      offset += 3;
      *val_bd_east = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bd_east |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bd_east |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bd_east |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bd_east = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bd_east |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bd_east !=0)
        *val_bd_east |= ((exp_bd_east)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bd_east = -this->bd_east;
      uint32_t * val_bd_north = (uint32_t*) &(this->bd_north);
      offset += 3;
      *val_bd_north = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bd_north |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bd_north |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bd_north |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bd_north = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bd_north |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bd_north !=0)
        *val_bd_north |= ((exp_bd_north)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bd_north = -this->bd_north;
      uint32_t * val_bd_upwards = (uint32_t*) &(this->bd_upwards);
      offset += 3;
      *val_bd_upwards = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bd_upwards |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bd_upwards |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bd_upwards |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bd_upwards = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bd_upwards |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bd_upwards !=0)
        *val_bd_upwards |= ((exp_bd_upwards)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bd_upwards = -this->bd_upwards;
      uint32_t * val_bd_range = (uint32_t*) &(this->bd_range);
      offset += 3;
      *val_bd_range = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bd_range |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bd_range |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bd_range |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bd_range = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bd_range |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bd_range !=0)
        *val_bd_range |= ((exp_bd_range)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bd_range = -this->bd_range;
      uint32_t * val_bd_time = (uint32_t*) &(this->bd_time);
      offset += 3;
      *val_bd_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_bd_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_bd_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_bd_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_bd_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_bd_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_bd_time !=0)
        *val_bd_time |= ((exp_bd_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->bd_time = -this->bd_time;
      uint32_t length_raw_data;
      memcpy(&length_raw_data, (inbuffer + offset), sizeof(uint32_t));
      offset += 4;
      for(unsigned int k= offset; k< offset+length_raw_data; ++k){
          inbuffer[k-1]=inbuffer[k];
      }
      inbuffer[offset+length_raw_data-1]=0;
      this->raw_data = (char *)(inbuffer + offset-1);
      offset += length_raw_data;
     return offset;
    }

    const char * getType(){ return "underwater_sensor_msgs/DVL"; };
    const char * getMD5(){ return "c455f331872552e620e26fc7caad335d"; };

  };

}
#endif