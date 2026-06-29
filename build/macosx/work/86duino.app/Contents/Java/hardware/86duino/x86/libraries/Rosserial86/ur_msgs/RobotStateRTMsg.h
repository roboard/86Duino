#ifndef _ROS_ur_msgs_RobotStateRTMsg_h
#define _ROS_ur_msgs_RobotStateRTMsg_h

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ros/msg.h"

namespace ur_msgs
{

  class RobotStateRTMsg : public ros::Msg
  {
    public:
      float time;
      uint8_t q_target_length;
      float st_q_target;
      float * q_target;
      uint8_t qd_target_length;
      float st_qd_target;
      float * qd_target;
      uint8_t qdd_target_length;
      float st_qdd_target;
      float * qdd_target;
      uint8_t i_target_length;
      float st_i_target;
      float * i_target;
      uint8_t m_target_length;
      float st_m_target;
      float * m_target;
      uint8_t q_actual_length;
      float st_q_actual;
      float * q_actual;
      uint8_t qd_actual_length;
      float st_qd_actual;
      float * qd_actual;
      uint8_t i_actual_length;
      float st_i_actual;
      float * i_actual;
      uint8_t tool_acc_values_length;
      float st_tool_acc_values;
      float * tool_acc_values;
      uint8_t tcp_force_length;
      float st_tcp_force;
      float * tcp_force;
      uint8_t tool_vector_length;
      float st_tool_vector;
      float * tool_vector;
      uint8_t tcp_speed_length;
      float st_tcp_speed;
      float * tcp_speed;
      float digital_input_bits;
      uint8_t motor_temperatures_length;
      float st_motor_temperatures;
      float * motor_temperatures;
      float controller_timer;
      float test_value;
      float robot_mode;
      uint8_t joint_modes_length;
      float st_joint_modes;
      float * joint_modes;

    virtual int serialize(unsigned char *outbuffer) const
    {
      int offset = 0;
      int32_t * val_time = (int32_t *) &(this->time);
      int32_t exp_time = (((*val_time)>>23)&255);
      if(exp_time != 0)
        exp_time += 1023-127;
      int32_t sig_time = *val_time;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_time<<5) & 0xff;
      *(outbuffer + offset++) = (sig_time>>3) & 0xff;
      *(outbuffer + offset++) = (sig_time>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_time<<4) & 0xF0) | ((sig_time>>19)&0x0F);
      *(outbuffer + offset++) = (exp_time>>4) & 0x7F;
      if(this->time < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset++) = q_target_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < q_target_length; i++){
      int32_t * val_q_targeti = (int32_t *) &(this->q_target[i]);
      int32_t exp_q_targeti = (((*val_q_targeti)>>23)&255);
      if(exp_q_targeti != 0)
        exp_q_targeti += 1023-127;
      int32_t sig_q_targeti = *val_q_targeti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_q_targeti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_q_targeti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_q_targeti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_q_targeti<<4) & 0xF0) | ((sig_q_targeti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_q_targeti>>4) & 0x7F;
      if(this->q_target[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = qd_target_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < qd_target_length; i++){
      int32_t * val_qd_targeti = (int32_t *) &(this->qd_target[i]);
      int32_t exp_qd_targeti = (((*val_qd_targeti)>>23)&255);
      if(exp_qd_targeti != 0)
        exp_qd_targeti += 1023-127;
      int32_t sig_qd_targeti = *val_qd_targeti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_qd_targeti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_qd_targeti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_qd_targeti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_qd_targeti<<4) & 0xF0) | ((sig_qd_targeti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_qd_targeti>>4) & 0x7F;
      if(this->qd_target[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = qdd_target_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < qdd_target_length; i++){
      int32_t * val_qdd_targeti = (int32_t *) &(this->qdd_target[i]);
      int32_t exp_qdd_targeti = (((*val_qdd_targeti)>>23)&255);
      if(exp_qdd_targeti != 0)
        exp_qdd_targeti += 1023-127;
      int32_t sig_qdd_targeti = *val_qdd_targeti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_qdd_targeti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_qdd_targeti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_qdd_targeti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_qdd_targeti<<4) & 0xF0) | ((sig_qdd_targeti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_qdd_targeti>>4) & 0x7F;
      if(this->qdd_target[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = i_target_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < i_target_length; i++){
      int32_t * val_i_targeti = (int32_t *) &(this->i_target[i]);
      int32_t exp_i_targeti = (((*val_i_targeti)>>23)&255);
      if(exp_i_targeti != 0)
        exp_i_targeti += 1023-127;
      int32_t sig_i_targeti = *val_i_targeti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_i_targeti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_i_targeti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_i_targeti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_i_targeti<<4) & 0xF0) | ((sig_i_targeti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_i_targeti>>4) & 0x7F;
      if(this->i_target[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = m_target_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < m_target_length; i++){
      int32_t * val_m_targeti = (int32_t *) &(this->m_target[i]);
      int32_t exp_m_targeti = (((*val_m_targeti)>>23)&255);
      if(exp_m_targeti != 0)
        exp_m_targeti += 1023-127;
      int32_t sig_m_targeti = *val_m_targeti;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_m_targeti<<5) & 0xff;
      *(outbuffer + offset++) = (sig_m_targeti>>3) & 0xff;
      *(outbuffer + offset++) = (sig_m_targeti>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_m_targeti<<4) & 0xF0) | ((sig_m_targeti>>19)&0x0F);
      *(outbuffer + offset++) = (exp_m_targeti>>4) & 0x7F;
      if(this->m_target[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = q_actual_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < q_actual_length; i++){
      int32_t * val_q_actuali = (int32_t *) &(this->q_actual[i]);
      int32_t exp_q_actuali = (((*val_q_actuali)>>23)&255);
      if(exp_q_actuali != 0)
        exp_q_actuali += 1023-127;
      int32_t sig_q_actuali = *val_q_actuali;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_q_actuali<<5) & 0xff;
      *(outbuffer + offset++) = (sig_q_actuali>>3) & 0xff;
      *(outbuffer + offset++) = (sig_q_actuali>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_q_actuali<<4) & 0xF0) | ((sig_q_actuali>>19)&0x0F);
      *(outbuffer + offset++) = (exp_q_actuali>>4) & 0x7F;
      if(this->q_actual[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = qd_actual_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < qd_actual_length; i++){
      int32_t * val_qd_actuali = (int32_t *) &(this->qd_actual[i]);
      int32_t exp_qd_actuali = (((*val_qd_actuali)>>23)&255);
      if(exp_qd_actuali != 0)
        exp_qd_actuali += 1023-127;
      int32_t sig_qd_actuali = *val_qd_actuali;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_qd_actuali<<5) & 0xff;
      *(outbuffer + offset++) = (sig_qd_actuali>>3) & 0xff;
      *(outbuffer + offset++) = (sig_qd_actuali>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_qd_actuali<<4) & 0xF0) | ((sig_qd_actuali>>19)&0x0F);
      *(outbuffer + offset++) = (exp_qd_actuali>>4) & 0x7F;
      if(this->qd_actual[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = i_actual_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < i_actual_length; i++){
      int32_t * val_i_actuali = (int32_t *) &(this->i_actual[i]);
      int32_t exp_i_actuali = (((*val_i_actuali)>>23)&255);
      if(exp_i_actuali != 0)
        exp_i_actuali += 1023-127;
      int32_t sig_i_actuali = *val_i_actuali;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_i_actuali<<5) & 0xff;
      *(outbuffer + offset++) = (sig_i_actuali>>3) & 0xff;
      *(outbuffer + offset++) = (sig_i_actuali>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_i_actuali<<4) & 0xF0) | ((sig_i_actuali>>19)&0x0F);
      *(outbuffer + offset++) = (exp_i_actuali>>4) & 0x7F;
      if(this->i_actual[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = tool_acc_values_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tool_acc_values_length; i++){
      int32_t * val_tool_acc_valuesi = (int32_t *) &(this->tool_acc_values[i]);
      int32_t exp_tool_acc_valuesi = (((*val_tool_acc_valuesi)>>23)&255);
      if(exp_tool_acc_valuesi != 0)
        exp_tool_acc_valuesi += 1023-127;
      int32_t sig_tool_acc_valuesi = *val_tool_acc_valuesi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tool_acc_valuesi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tool_acc_valuesi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tool_acc_valuesi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tool_acc_valuesi<<4) & 0xF0) | ((sig_tool_acc_valuesi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tool_acc_valuesi>>4) & 0x7F;
      if(this->tool_acc_values[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = tcp_force_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tcp_force_length; i++){
      int32_t * val_tcp_forcei = (int32_t *) &(this->tcp_force[i]);
      int32_t exp_tcp_forcei = (((*val_tcp_forcei)>>23)&255);
      if(exp_tcp_forcei != 0)
        exp_tcp_forcei += 1023-127;
      int32_t sig_tcp_forcei = *val_tcp_forcei;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tcp_forcei<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tcp_forcei>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tcp_forcei>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tcp_forcei<<4) & 0xF0) | ((sig_tcp_forcei>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tcp_forcei>>4) & 0x7F;
      if(this->tcp_force[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = tool_vector_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tool_vector_length; i++){
      int32_t * val_tool_vectori = (int32_t *) &(this->tool_vector[i]);
      int32_t exp_tool_vectori = (((*val_tool_vectori)>>23)&255);
      if(exp_tool_vectori != 0)
        exp_tool_vectori += 1023-127;
      int32_t sig_tool_vectori = *val_tool_vectori;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tool_vectori<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tool_vectori>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tool_vectori>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tool_vectori<<4) & 0xF0) | ((sig_tool_vectori>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tool_vectori>>4) & 0x7F;
      if(this->tool_vector[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      *(outbuffer + offset++) = tcp_speed_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < tcp_speed_length; i++){
      int32_t * val_tcp_speedi = (int32_t *) &(this->tcp_speed[i]);
      int32_t exp_tcp_speedi = (((*val_tcp_speedi)>>23)&255);
      if(exp_tcp_speedi != 0)
        exp_tcp_speedi += 1023-127;
      int32_t sig_tcp_speedi = *val_tcp_speedi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_tcp_speedi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_tcp_speedi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_tcp_speedi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_tcp_speedi<<4) & 0xF0) | ((sig_tcp_speedi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_tcp_speedi>>4) & 0x7F;
      if(this->tcp_speed[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      int32_t * val_digital_input_bits = (int32_t *) &(this->digital_input_bits);
      int32_t exp_digital_input_bits = (((*val_digital_input_bits)>>23)&255);
      if(exp_digital_input_bits != 0)
        exp_digital_input_bits += 1023-127;
      int32_t sig_digital_input_bits = *val_digital_input_bits;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_digital_input_bits<<5) & 0xff;
      *(outbuffer + offset++) = (sig_digital_input_bits>>3) & 0xff;
      *(outbuffer + offset++) = (sig_digital_input_bits>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_digital_input_bits<<4) & 0xF0) | ((sig_digital_input_bits>>19)&0x0F);
      *(outbuffer + offset++) = (exp_digital_input_bits>>4) & 0x7F;
      if(this->digital_input_bits < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset++) = motor_temperatures_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < motor_temperatures_length; i++){
      int32_t * val_motor_temperaturesi = (int32_t *) &(this->motor_temperatures[i]);
      int32_t exp_motor_temperaturesi = (((*val_motor_temperaturesi)>>23)&255);
      if(exp_motor_temperaturesi != 0)
        exp_motor_temperaturesi += 1023-127;
      int32_t sig_motor_temperaturesi = *val_motor_temperaturesi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_motor_temperaturesi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_motor_temperaturesi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_motor_temperaturesi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_motor_temperaturesi<<4) & 0xF0) | ((sig_motor_temperaturesi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_motor_temperaturesi>>4) & 0x7F;
      if(this->motor_temperatures[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      int32_t * val_controller_timer = (int32_t *) &(this->controller_timer);
      int32_t exp_controller_timer = (((*val_controller_timer)>>23)&255);
      if(exp_controller_timer != 0)
        exp_controller_timer += 1023-127;
      int32_t sig_controller_timer = *val_controller_timer;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_controller_timer<<5) & 0xff;
      *(outbuffer + offset++) = (sig_controller_timer>>3) & 0xff;
      *(outbuffer + offset++) = (sig_controller_timer>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_controller_timer<<4) & 0xF0) | ((sig_controller_timer>>19)&0x0F);
      *(outbuffer + offset++) = (exp_controller_timer>>4) & 0x7F;
      if(this->controller_timer < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_test_value = (int32_t *) &(this->test_value);
      int32_t exp_test_value = (((*val_test_value)>>23)&255);
      if(exp_test_value != 0)
        exp_test_value += 1023-127;
      int32_t sig_test_value = *val_test_value;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_test_value<<5) & 0xff;
      *(outbuffer + offset++) = (sig_test_value>>3) & 0xff;
      *(outbuffer + offset++) = (sig_test_value>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_test_value<<4) & 0xF0) | ((sig_test_value>>19)&0x0F);
      *(outbuffer + offset++) = (exp_test_value>>4) & 0x7F;
      if(this->test_value < 0) *(outbuffer + offset -1) |= 0x80;
      int32_t * val_robot_mode = (int32_t *) &(this->robot_mode);
      int32_t exp_robot_mode = (((*val_robot_mode)>>23)&255);
      if(exp_robot_mode != 0)
        exp_robot_mode += 1023-127;
      int32_t sig_robot_mode = *val_robot_mode;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_robot_mode<<5) & 0xff;
      *(outbuffer + offset++) = (sig_robot_mode>>3) & 0xff;
      *(outbuffer + offset++) = (sig_robot_mode>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_robot_mode<<4) & 0xF0) | ((sig_robot_mode>>19)&0x0F);
      *(outbuffer + offset++) = (exp_robot_mode>>4) & 0x7F;
      if(this->robot_mode < 0) *(outbuffer + offset -1) |= 0x80;
      *(outbuffer + offset++) = joint_modes_length;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      for( uint8_t i = 0; i < joint_modes_length; i++){
      int32_t * val_joint_modesi = (int32_t *) &(this->joint_modes[i]);
      int32_t exp_joint_modesi = (((*val_joint_modesi)>>23)&255);
      if(exp_joint_modesi != 0)
        exp_joint_modesi += 1023-127;
      int32_t sig_joint_modesi = *val_joint_modesi;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = 0;
      *(outbuffer + offset++) = (sig_joint_modesi<<5) & 0xff;
      *(outbuffer + offset++) = (sig_joint_modesi>>3) & 0xff;
      *(outbuffer + offset++) = (sig_joint_modesi>>11) & 0xff;
      *(outbuffer + offset++) = ((exp_joint_modesi<<4) & 0xF0) | ((sig_joint_modesi>>19)&0x0F);
      *(outbuffer + offset++) = (exp_joint_modesi>>4) & 0x7F;
      if(this->joint_modes[i] < 0) *(outbuffer + offset -1) |= 0x80;
      }
      return offset;
    }

    virtual int deserialize(unsigned char *inbuffer)
    {
      int offset = 0;
      uint32_t * val_time = (uint32_t*) &(this->time);
      offset += 3;
      *val_time = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_time |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_time |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_time = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_time |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_time !=0)
        *val_time |= ((exp_time)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->time = -this->time;
      uint8_t q_target_lengthT = *(inbuffer + offset++);
      if(q_target_lengthT > q_target_length)
        this->q_target = (float*)realloc(this->q_target, q_target_lengthT * sizeof(float));
      offset += 3;
      q_target_length = q_target_lengthT;
      for( uint8_t i = 0; i < q_target_length; i++){
      uint32_t * val_st_q_target = (uint32_t*) &(this->st_q_target);
      offset += 3;
      *val_st_q_target = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_q_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_q_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_q_target |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_q_target = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_q_target |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_q_target !=0)
        *val_st_q_target |= ((exp_st_q_target)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_q_target = -this->st_q_target;
        memcpy( &(this->q_target[i]), &(this->st_q_target), sizeof(float));
      }
      uint8_t qd_target_lengthT = *(inbuffer + offset++);
      if(qd_target_lengthT > qd_target_length)
        this->qd_target = (float*)realloc(this->qd_target, qd_target_lengthT * sizeof(float));
      offset += 3;
      qd_target_length = qd_target_lengthT;
      for( uint8_t i = 0; i < qd_target_length; i++){
      uint32_t * val_st_qd_target = (uint32_t*) &(this->st_qd_target);
      offset += 3;
      *val_st_qd_target = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_qd_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_qd_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_qd_target |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_qd_target = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_qd_target |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_qd_target !=0)
        *val_st_qd_target |= ((exp_st_qd_target)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_qd_target = -this->st_qd_target;
        memcpy( &(this->qd_target[i]), &(this->st_qd_target), sizeof(float));
      }
      uint8_t qdd_target_lengthT = *(inbuffer + offset++);
      if(qdd_target_lengthT > qdd_target_length)
        this->qdd_target = (float*)realloc(this->qdd_target, qdd_target_lengthT * sizeof(float));
      offset += 3;
      qdd_target_length = qdd_target_lengthT;
      for( uint8_t i = 0; i < qdd_target_length; i++){
      uint32_t * val_st_qdd_target = (uint32_t*) &(this->st_qdd_target);
      offset += 3;
      *val_st_qdd_target = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_qdd_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_qdd_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_qdd_target |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_qdd_target = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_qdd_target |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_qdd_target !=0)
        *val_st_qdd_target |= ((exp_st_qdd_target)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_qdd_target = -this->st_qdd_target;
        memcpy( &(this->qdd_target[i]), &(this->st_qdd_target), sizeof(float));
      }
      uint8_t i_target_lengthT = *(inbuffer + offset++);
      if(i_target_lengthT > i_target_length)
        this->i_target = (float*)realloc(this->i_target, i_target_lengthT * sizeof(float));
      offset += 3;
      i_target_length = i_target_lengthT;
      for( uint8_t i = 0; i < i_target_length; i++){
      uint32_t * val_st_i_target = (uint32_t*) &(this->st_i_target);
      offset += 3;
      *val_st_i_target = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_i_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_i_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_i_target |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_i_target = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_i_target |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_i_target !=0)
        *val_st_i_target |= ((exp_st_i_target)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_i_target = -this->st_i_target;
        memcpy( &(this->i_target[i]), &(this->st_i_target), sizeof(float));
      }
      uint8_t m_target_lengthT = *(inbuffer + offset++);
      if(m_target_lengthT > m_target_length)
        this->m_target = (float*)realloc(this->m_target, m_target_lengthT * sizeof(float));
      offset += 3;
      m_target_length = m_target_lengthT;
      for( uint8_t i = 0; i < m_target_length; i++){
      uint32_t * val_st_m_target = (uint32_t*) &(this->st_m_target);
      offset += 3;
      *val_st_m_target = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_m_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_m_target |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_m_target |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_m_target = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_m_target |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_m_target !=0)
        *val_st_m_target |= ((exp_st_m_target)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_m_target = -this->st_m_target;
        memcpy( &(this->m_target[i]), &(this->st_m_target), sizeof(float));
      }
      uint8_t q_actual_lengthT = *(inbuffer + offset++);
      if(q_actual_lengthT > q_actual_length)
        this->q_actual = (float*)realloc(this->q_actual, q_actual_lengthT * sizeof(float));
      offset += 3;
      q_actual_length = q_actual_lengthT;
      for( uint8_t i = 0; i < q_actual_length; i++){
      uint32_t * val_st_q_actual = (uint32_t*) &(this->st_q_actual);
      offset += 3;
      *val_st_q_actual = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_q_actual |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_q_actual |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_q_actual |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_q_actual = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_q_actual |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_q_actual !=0)
        *val_st_q_actual |= ((exp_st_q_actual)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_q_actual = -this->st_q_actual;
        memcpy( &(this->q_actual[i]), &(this->st_q_actual), sizeof(float));
      }
      uint8_t qd_actual_lengthT = *(inbuffer + offset++);
      if(qd_actual_lengthT > qd_actual_length)
        this->qd_actual = (float*)realloc(this->qd_actual, qd_actual_lengthT * sizeof(float));
      offset += 3;
      qd_actual_length = qd_actual_lengthT;
      for( uint8_t i = 0; i < qd_actual_length; i++){
      uint32_t * val_st_qd_actual = (uint32_t*) &(this->st_qd_actual);
      offset += 3;
      *val_st_qd_actual = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_qd_actual |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_qd_actual |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_qd_actual |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_qd_actual = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_qd_actual |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_qd_actual !=0)
        *val_st_qd_actual |= ((exp_st_qd_actual)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_qd_actual = -this->st_qd_actual;
        memcpy( &(this->qd_actual[i]), &(this->st_qd_actual), sizeof(float));
      }
      uint8_t i_actual_lengthT = *(inbuffer + offset++);
      if(i_actual_lengthT > i_actual_length)
        this->i_actual = (float*)realloc(this->i_actual, i_actual_lengthT * sizeof(float));
      offset += 3;
      i_actual_length = i_actual_lengthT;
      for( uint8_t i = 0; i < i_actual_length; i++){
      uint32_t * val_st_i_actual = (uint32_t*) &(this->st_i_actual);
      offset += 3;
      *val_st_i_actual = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_i_actual |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_i_actual |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_i_actual |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_i_actual = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_i_actual |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_i_actual !=0)
        *val_st_i_actual |= ((exp_st_i_actual)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_i_actual = -this->st_i_actual;
        memcpy( &(this->i_actual[i]), &(this->st_i_actual), sizeof(float));
      }
      uint8_t tool_acc_values_lengthT = *(inbuffer + offset++);
      if(tool_acc_values_lengthT > tool_acc_values_length)
        this->tool_acc_values = (float*)realloc(this->tool_acc_values, tool_acc_values_lengthT * sizeof(float));
      offset += 3;
      tool_acc_values_length = tool_acc_values_lengthT;
      for( uint8_t i = 0; i < tool_acc_values_length; i++){
      uint32_t * val_st_tool_acc_values = (uint32_t*) &(this->st_tool_acc_values);
      offset += 3;
      *val_st_tool_acc_values = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_tool_acc_values |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_tool_acc_values |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_tool_acc_values |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_tool_acc_values = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_tool_acc_values |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_tool_acc_values !=0)
        *val_st_tool_acc_values |= ((exp_st_tool_acc_values)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_tool_acc_values = -this->st_tool_acc_values;
        memcpy( &(this->tool_acc_values[i]), &(this->st_tool_acc_values), sizeof(float));
      }
      uint8_t tcp_force_lengthT = *(inbuffer + offset++);
      if(tcp_force_lengthT > tcp_force_length)
        this->tcp_force = (float*)realloc(this->tcp_force, tcp_force_lengthT * sizeof(float));
      offset += 3;
      tcp_force_length = tcp_force_lengthT;
      for( uint8_t i = 0; i < tcp_force_length; i++){
      uint32_t * val_st_tcp_force = (uint32_t*) &(this->st_tcp_force);
      offset += 3;
      *val_st_tcp_force = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_tcp_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_tcp_force |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_tcp_force |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_tcp_force = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_tcp_force |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_tcp_force !=0)
        *val_st_tcp_force |= ((exp_st_tcp_force)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_tcp_force = -this->st_tcp_force;
        memcpy( &(this->tcp_force[i]), &(this->st_tcp_force), sizeof(float));
      }
      uint8_t tool_vector_lengthT = *(inbuffer + offset++);
      if(tool_vector_lengthT > tool_vector_length)
        this->tool_vector = (float*)realloc(this->tool_vector, tool_vector_lengthT * sizeof(float));
      offset += 3;
      tool_vector_length = tool_vector_lengthT;
      for( uint8_t i = 0; i < tool_vector_length; i++){
      uint32_t * val_st_tool_vector = (uint32_t*) &(this->st_tool_vector);
      offset += 3;
      *val_st_tool_vector = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_tool_vector |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_tool_vector |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_tool_vector |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_tool_vector = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_tool_vector |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_tool_vector !=0)
        *val_st_tool_vector |= ((exp_st_tool_vector)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_tool_vector = -this->st_tool_vector;
        memcpy( &(this->tool_vector[i]), &(this->st_tool_vector), sizeof(float));
      }
      uint8_t tcp_speed_lengthT = *(inbuffer + offset++);
      if(tcp_speed_lengthT > tcp_speed_length)
        this->tcp_speed = (float*)realloc(this->tcp_speed, tcp_speed_lengthT * sizeof(float));
      offset += 3;
      tcp_speed_length = tcp_speed_lengthT;
      for( uint8_t i = 0; i < tcp_speed_length; i++){
      uint32_t * val_st_tcp_speed = (uint32_t*) &(this->st_tcp_speed);
      offset += 3;
      *val_st_tcp_speed = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_tcp_speed |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_tcp_speed |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_tcp_speed |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_tcp_speed = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_tcp_speed |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_tcp_speed !=0)
        *val_st_tcp_speed |= ((exp_st_tcp_speed)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_tcp_speed = -this->st_tcp_speed;
        memcpy( &(this->tcp_speed[i]), &(this->st_tcp_speed), sizeof(float));
      }
      uint32_t * val_digital_input_bits = (uint32_t*) &(this->digital_input_bits);
      offset += 3;
      *val_digital_input_bits = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_digital_input_bits |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_digital_input_bits |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_digital_input_bits |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_digital_input_bits = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_digital_input_bits |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_digital_input_bits !=0)
        *val_digital_input_bits |= ((exp_digital_input_bits)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->digital_input_bits = -this->digital_input_bits;
      uint8_t motor_temperatures_lengthT = *(inbuffer + offset++);
      if(motor_temperatures_lengthT > motor_temperatures_length)
        this->motor_temperatures = (float*)realloc(this->motor_temperatures, motor_temperatures_lengthT * sizeof(float));
      offset += 3;
      motor_temperatures_length = motor_temperatures_lengthT;
      for( uint8_t i = 0; i < motor_temperatures_length; i++){
      uint32_t * val_st_motor_temperatures = (uint32_t*) &(this->st_motor_temperatures);
      offset += 3;
      *val_st_motor_temperatures = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_motor_temperatures |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_motor_temperatures |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_motor_temperatures |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_motor_temperatures = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_motor_temperatures |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_motor_temperatures !=0)
        *val_st_motor_temperatures |= ((exp_st_motor_temperatures)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_motor_temperatures = -this->st_motor_temperatures;
        memcpy( &(this->motor_temperatures[i]), &(this->st_motor_temperatures), sizeof(float));
      }
      uint32_t * val_controller_timer = (uint32_t*) &(this->controller_timer);
      offset += 3;
      *val_controller_timer = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_controller_timer |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_controller_timer |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_controller_timer |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_controller_timer = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_controller_timer |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_controller_timer !=0)
        *val_controller_timer |= ((exp_controller_timer)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->controller_timer = -this->controller_timer;
      uint32_t * val_test_value = (uint32_t*) &(this->test_value);
      offset += 3;
      *val_test_value = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_test_value |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_test_value |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_test_value |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_test_value = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_test_value |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_test_value !=0)
        *val_test_value |= ((exp_test_value)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->test_value = -this->test_value;
      uint32_t * val_robot_mode = (uint32_t*) &(this->robot_mode);
      offset += 3;
      *val_robot_mode = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_robot_mode |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_robot_mode |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_robot_mode |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_robot_mode = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_robot_mode |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_robot_mode !=0)
        *val_robot_mode |= ((exp_robot_mode)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->robot_mode = -this->robot_mode;
      uint8_t joint_modes_lengthT = *(inbuffer + offset++);
      if(joint_modes_lengthT > joint_modes_length)
        this->joint_modes = (float*)realloc(this->joint_modes, joint_modes_lengthT * sizeof(float));
      offset += 3;
      joint_modes_length = joint_modes_lengthT;
      for( uint8_t i = 0; i < joint_modes_length; i++){
      uint32_t * val_st_joint_modes = (uint32_t*) &(this->st_joint_modes);
      offset += 3;
      *val_st_joint_modes = ((uint32_t)(*(inbuffer + offset++))>>5 & 0x07);
      *val_st_joint_modes |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<3;
      *val_st_joint_modes |= ((uint32_t)(*(inbuffer + offset++)) & 0xff)<<11;
      *val_st_joint_modes |= ((uint32_t)(*(inbuffer + offset)) & 0x0f)<<19;
      uint32_t exp_st_joint_modes = ((uint32_t)(*(inbuffer + offset++))&0xf0)>>4;
      exp_st_joint_modes |= ((uint32_t)(*(inbuffer + offset)) & 0x7f)<<4;
      if(exp_st_joint_modes !=0)
        *val_st_joint_modes |= ((exp_st_joint_modes)-1023+127)<<23;
      if( ((*(inbuffer+offset++)) & 0x80) > 0) this->st_joint_modes = -this->st_joint_modes;
        memcpy( &(this->joint_modes[i]), &(this->st_joint_modes), sizeof(float));
      }
     return offset;
    }

    const char * getType(){ return "ur_msgs/RobotStateRTMsg"; };
    const char * getMD5(){ return "ce6feddd3ccb4ca7dbcd0ff105b603c7"; };

  };

}
#endif