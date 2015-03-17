#ifndef ROS_86DUINO_CONFIG_H
#define ROS_86DUINO_CONFIG_H

/* Use Turbo-Serial on Vortex86 chip. */
// #define ROS_VORTEX86_TURBO_SERIAL
#ifdef  ROS_VORTEX86_TURBO_SERIAL

/* Initial Vortex86 chip, such as IO initialize. */
#define ROS_VORTEX86_CHIP_INITIALIZE

#endif

#endif
