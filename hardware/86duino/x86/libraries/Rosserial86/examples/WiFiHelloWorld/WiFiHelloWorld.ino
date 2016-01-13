#include <ros.h>
#include <std_msgs/String.h>

ros::NodeHandle nh;

std_msgs::String str_msg;
ros::Publisher chatter("chatter", &str_msg);

char hello[13] = "hello world!";

void setup()
{
  nh.getHardware()->setESP8266(Serial1, 115200);
  nh.getHardware()->setWiFi("SSID", "PASSWORD");
  nh.initNode("192.168.0.1");
  nh.advertise(chatter);
}

void loop()
{  
  str_msg.data = hello;
  chatter.publish( &str_msg );
  nh.spinOnce();
  delay(1000);
}