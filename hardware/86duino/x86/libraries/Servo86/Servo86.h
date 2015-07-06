#ifndef Servo_h
#define Servo_h

#include <stdio.h>
#include <inttypes.h>

#define Servo_VERSION           2      // software version of this library
#define MAX_SERVOS             45
#define MIN_PULSE_WIDTH       450     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      2800     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define REFRESH_INTERVAL    20000     // minumim time to refresh servos in microseconds 
#define INVALID_SERVO         255     // flag indicating an invalid servo index
#define FULL_SPEED            (-1)

typedef struct  {
  uint8_t nbr        :6 ;             // a pin number from 0 to 63
  uint8_t isActive   :1 ;             // true if this channel is enabled, pin not pulsed if false 
} ServoPin_t   ;  

typedef struct {
  ServoPin_t Pin;
  unsigned int ticks;
} servo_t;


class Servo
{
public:
	Servo();
	
	uint8_t servoIndex;                // index into the channel data for this servo
	
	uint8_t attach(int pin);           // attach the given pin to the next free channel, sets pinMode, returns channel number or 0 if failure
	uint8_t attach(int pin, unsigned long min, unsigned long max); // as above but also sets min and max values for writes. 
	void detach();
	void write(int value);             // if value is < 200 its treated as an angle, otherwise as pulse width in microseconds 
	void writeMicroseconds(int value); // Write pulse width in microseconds 
	double read(void);
	unsigned long readMicroseconds();  // returns current pulse width in microseconds for this servo (was read_us() in first release)
	bool attached();                   // return true if this servo is attached, otherwise false
	
	// Added Servo86 API as following
	void angleMap(double angle_of_min_pwm_duty, double angle_of_max_pwm_duty);
	void setVelocity(int velocity)
	{
		setVelocity((unsigned long)velocity);
	}
	void setVelocity(unsigned long velocity);
	void setVelocity(double angular_velocity);

	void setPosition(int pos, unsigned long time=0L)
	{
		setPosition((long)pos, time);
	}
	void setPosition(unsigned long pos, unsigned long time=0L)
	{
		setPosition((long)pos, time);
	}
	void setPosition(long pos, unsigned long time=0L);
	void setPosition(double angle, unsigned long time=0L);
	void setOffset(long offset);
	void setRealTimeMixing(long offset);
	long getPosition(void);
	void write(unsigned long pos);
	void write(double angle);
  
	void run(void);
	void pause(void);
	void resume(void);
	void stop(void);
	void release(void);
	bool isMoving(void);
  
protected:
	long min;                       // minimum is this value times 4 added to MIN_PULSE_WIDTH    
	long max;                       // maximum is this value times 4 added to MAX_PULSE_WIDTH
	long speed_us;
	double angle_resolution;
	unsigned long total_time;
	unsigned long target_position;
	long pos_offset;  
};

extern Servo nullServo;

class ServoFrame
{
public:
	long positions[45]; //45 channels
	ServoFrame();
	ServoFrame(const char* dir);
	void setPositions(Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                      Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                      Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                      Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                      Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                      Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                      Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                      Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                      Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                      Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                      Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                      Servo &s45=nullServo);
    void setPositions(unsigned long playtime, Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                      Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                      Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                      Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                      Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                      Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                      Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                      Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                      Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                      Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                      Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                      Servo &s45=nullServo);
    virtual bool load(const char* dir);
    virtual bool save(const char* dir);
    void playPositions(Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                       Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                       Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                       Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                       Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                       Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                       Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                       Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                       Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                       Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                       Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                       Servo &s45=nullServo);
    void playPositions(unsigned long playtime, Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                       Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                       Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                       Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                       Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                       Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                       Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                       Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                       Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                       Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                       Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                       Servo &s45=nullServo);

	ServoFrame & operator = (const ServoFrame &s) {
		int i;
		for(i=0; i<45; i++) positions[i] = s.positions[i];
	}
};
//extern volatile unsigned long __count;

class ServoOffset
{
public:
	long offsets[45];
	
	ServoOffset();
	ServoOffset(const char* dir);
	void setOffsets(Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,          
                    Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                    Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                    Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                    Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                    Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                    Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                    Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                    Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                    Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                    Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                    Servo &s45=nullServo);
    
	virtual bool load(const char* dir);
    virtual bool save(const char* dir);
    
    ServoOffset & operator = (const ServoOffset &s) {
		int i;
		for(i=0; i<45; i++) offsets[i] = s.offsets[i];
	}
};


/***************************** Other Servo Controler **************************/

class ServoFrameInno : public ServoFrame
{
private:
	unsigned long frmfile[173]; // 3 + 34 * 5
	unsigned long all_string2[10];

public:
	ServoFrameInno();
	ServoFrameInno(const char* dir);

    int frameno;
	int M1ID;
	int M2ID;
	char en[32];
	// long positions[32];
	long time[32];
	long speed[32];
	char mode[32][10];
	
	char all_string[2][256];
	
	bool load(const char* dir);
	
	ServoFrameInno & operator = (const ServoFrame &s) {
		int i;
		for(i=0; i<32; i++) positions[i] = s.positions[i];
	}
	
	ServoFrameInno & operator = (const ServoFrameInno &s) {
		int i, j;
		for(i=0; i<32; i++) en[i] = s.en[i];
		for(i=0; i<32; i++) positions[i] = s.positions[i];
		for(i=0; i<32; i++) for(j=0; j<10; j++) mode[i][j] = s.mode[i][j];
		for(i=0; i<32; i++) speed[i] = s.speed[i];
		for(i=0; i<32; i++) time[i] = s.time[i];
		for(i=0; i<2; i++) for(j=0; j<256; j++) all_string[i][j] = s.all_string[i][j];
	}
};


class ServoOffsetInno : public ServoOffset
{
private:
	long ofsfile[32]; // 32 * 4
	
public:
	ServoOffsetInno();
	ServoOffsetInno(const char* dir);
	
	bool load(const char* dir);
	
	ServoOffsetInno & operator = (const ServoOffset &s) {
		int i;
		for(i=0; i<32; i++) offsets[i] = s.offsets[i];
	}
	
	ServoOffsetInno & operator = (const ServoOffsetInno &s) {
		int i;
		for(i=0; i<32; i++) offsets[i] = s.offsets[i];
	}
};


class ServoFrameKondo : public ServoFrame
{
public:
	ServoFrameKondo();
    ServoFrameKondo(const char* dir, const char* fname);

    bool load(const char* dir, const char* fname = NULL);
	
    long capture(Servo &s);
    
    ServoFrameKondo & operator = (const ServoFrame &s) {
		int i;
		for(i=0; i<24; i++) positions[i] = s.positions[i];
	}
	
	ServoFrameKondo & operator = (const ServoFrameKondo &s) {
		int i;
		for(i=0; i<24; i++) positions[i] = s.positions[i];
	}
};

class ServoOffsetKondo : public ServoOffset
{	
public:
	ServoOffsetKondo();
	ServoOffsetKondo(const char* dir);
	bool load(const char* dir);
	
	ServoOffsetKondo & operator = (const ServoOffset &s) {
		int i;
		for(i=0; i<24; i++) offsets[i] = s.offsets[i];
	}
	
	ServoOffsetKondo & operator = (const ServoOffsetKondo &s) {
		int i;
		for(i=0; i<24; i++) offsets[i] = s.offsets[i];
	}
};


class ServoFramePololu : public ServoFrame
{
public:
	ServoFramePololu();
    ServoFramePololu(const char* dir, const char* sname, const char* fname);
    
    bool load(const char* dir, const char* sname=NULL, const char* fname=NULL);

	ServoFramePololu & operator = (const ServoFrame &s) {
		int i;
		for(i=0; i<24; i++) positions[i] = s.positions[i];
	}
	
	ServoFramePololu & operator = (const ServoFramePololu &s) {
		int i;
		for(i=0; i<24; i++) positions[i] = s.positions[i];
	}
};


/* TODO:
 * Hitec Servo Controler
 * Lynxmotion Servo Controler
 * vstone Servo Controler  
*/

////////////////////////////////////////////////////////////////////////////////

void servoMultiRun(Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                   Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                   Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                   Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                   Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                   Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                   Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                   Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                   Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                   Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                   Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                   Servo &s45=nullServo);


void servoMultiPause(Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                     Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                     Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                     Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                     Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                     Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                     Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                     Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                     Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                     Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                     Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                     Servo &s45=nullServo);

void servoMultiResume(Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                      Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                      Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                      Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                      Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                      Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                      Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                      Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                      Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                      Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                      Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                      Servo &s45=nullServo);

void servoMultiStop(Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                    Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                    Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                    Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                    Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                    Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                    Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                    Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                    Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                    Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                    Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                    Servo &s45=nullServo);

bool isServoMultiMoving(Servo &s1=nullServo, Servo &s2=nullServo, Servo &s3=nullServo, Servo &s4=nullServo,
                        Servo &s5=nullServo, Servo &s6=nullServo, Servo &s7=nullServo, Servo &s8=nullServo,
                        Servo &s9=nullServo, Servo &s10=nullServo, Servo &s11=nullServo, Servo &s12=nullServo,
                        Servo &s13=nullServo, Servo &s14=nullServo, Servo &s15=nullServo, Servo &s16=nullServo,
                        Servo &s17=nullServo, Servo &s18=nullServo, Servo &s19=nullServo, Servo &s20=nullServo,
                        Servo &s21=nullServo, Servo &s22=nullServo, Servo &s23=nullServo, Servo &s24=nullServo,
                        Servo &s25=nullServo, Servo &s26=nullServo, Servo &s27=nullServo, Servo &s28=nullServo,
                        Servo &s29=nullServo, Servo &s30=nullServo, Servo &s31=nullServo, Servo &s32=nullServo,
                        Servo &s33=nullServo, Servo &s34=nullServo, Servo &s35=nullServo, Servo &s36=nullServo,
                        Servo &s37=nullServo, Servo &s38=nullServo, Servo &s39=nullServo, Servo &s40=nullServo,
                        Servo &s41=nullServo, Servo &s42=nullServo, Servo &s43=nullServo, Servo &s44=nullServo,
                        Servo &s45=nullServo);


//void update_position();
#endif
