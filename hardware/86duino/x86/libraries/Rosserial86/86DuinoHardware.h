#ifndef ROS_86DUINO_HARDWARE_H
#define ROS_86DUINO_HARDWARE_H

class Vortex86Hardware;

class x86DuinoHardware
{
public:
	x86DuinoHardware();
	x86DuinoHardware(char *pName, long baud = 57600L);
	~x86DuinoHardware();
	
	void setBaud(long baud);
	long getBaud();
	
	void init();
	void init(char *pName);
	
	int read();
	
	void write(unsigned char *pBuf, int nByte);
	
	unsigned long time();
	void delay(unsigned long millisecond);

	// only for DOS
	bool setDhcp();
	bool setEthernet(char *ip);
	bool setEthernet(char *ip, char *dns);
	bool setEthernet(char *ip, char *dns, char *gateway);
	bool setEthernet(char *ip, char *dns, char *gateway, char *subnet);
	
private:
	char portName[128];
	long baud_;
	Vortex86Hardware *x86;
};

#endif
