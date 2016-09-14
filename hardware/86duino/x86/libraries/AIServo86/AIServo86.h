#ifndef __AISERVO86_h
#define __AISERVO86_h

#include "utility/AX12.h"
#include "utility/MX28.h"
#include "utility/RX28.h"
#include "utility/XL320.h"

#define ROBOTISAX12  AX12Bus
#define ROBOTISRX28  RX28Bus
#define ROBOTISMX28  MX28Bus
#define ROBOTISXL320 XL320Bus
#define AIServoPort(vendor, servo) vendor##servo

class AIServo {
public:
	uint8_t servoIndex;
	unsigned char syncBuffer[256] = {0};
	AIServo();
	
	bool attach(AIServoBus& bus, unsigned char id);
	bool attached(void);
	void detach(void);
	void write(double angle);
	double read(void);
	void setPosition(double angle, unsigned long time = 0L);
	void setPositionAcceleration(double acc);
	void setPositionAcceleration(double acc_in, double acc_out);
	void setVelocity(double velocity);
	
	void setOffset(double offset);
	void setRealTimeMixing(double mixoffset);
	void setRealTimeMixing(double mixoffset, bool interrupt);
	double getPosition(void);

	void run(void);
	void pause(void);
	void resume(void);
	void stop(void);
	void release(void);
	bool isMoving(void);
	
	friend class AIServoFrame;
	
private:
	unsigned char ID;
	AIServoBus* attachedBus;
	AIServoList* nodePtr;
	long long acc_in;
	long long acc_out;
};

extern AIServo nullAIServo;

class AIServoFrame
{
public:
	double positions[64];
	unsigned long playtime[64];
	double accelerations[64][2];
	AIServoFrame();
	AIServoFrame(const char* dir);
	void setPositions(AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
    void setPositions(unsigned long playtime, AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
	void setPositions(int playtime, AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
    virtual bool load(const char* dir);
	virtual bool load(const String &s);
    virtual bool save(const char* dir);
	virtual bool save(const String &s);

	void playPositions(AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
	void playPositions(unsigned long long enabled_servos, AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
	void playPositions(unsigned long playtime, AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
	void playPositions(int playtime, AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
	void playPositions(unsigned long playtime, unsigned long long enabled_servos,
                      AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
	void playPositions(int playtime, unsigned long long enabled_servos,
                      AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);

	AIServoFrame & operator = (const AIServoFrame &s)
	{
		for(int i = 0; i < 64; i++)
			this->positions[i] = s.positions[i];
	}
private:
	unsigned long long used_servos;
};

class AIServoOffset
{
public:
	double offsets[64];
	
	AIServoOffset();
	AIServoOffset(const char* dir);
	void setOffsets(AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                    AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                    AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                    AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                    AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                    AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                    AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                    AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                    AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                    AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                    AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                    AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                    AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                    AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                    AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                    AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);
    
	virtual bool load(const char* dir);
	virtual bool load(const String &s);
    virtual bool save(const char* dir);
	virtual bool save(const String &s);
    
    AIServoOffset & operator = (const AIServoOffset &s)
	{
		for(int i = 0; i < 64; i++)
			this->offsets[i] = s.offsets[i];
	}
};

void aiservoMultiRun(AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                   AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                   AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                   AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                   AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                   AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                   AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                   AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                   AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                   AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                   AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                   AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                   AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                   AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                   AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                   AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);

void aiservoMultiPause(AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                     AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                     AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                     AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                     AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                     AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                     AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                     AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                     AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                     AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                     AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                     AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                     AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                     AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                     AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                     AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);

void aiservoMultiResume(AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                      AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                      AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                      AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                      AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                      AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                      AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                      AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                      AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                      AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                      AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                      AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                      AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                      AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                      AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                      AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);

void aiservoMultiStop(AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                    AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                    AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                    AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                    AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                    AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                    AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                    AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                    AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                    AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                    AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                    AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                    AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                    AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                    AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                    AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);

bool isAIServoMultiMoving(AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                        AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                        AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                        AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                        AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                        AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                        AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                        AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                        AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                        AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                        AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                        AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                        AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                        AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                        AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                        AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);

void aiservoMultiRealTimeMixing(double* mixoffsets, AIServo &s1=nullAIServo, AIServo &s2=nullAIServo, AIServo &s3=nullAIServo, AIServo &s4=nullAIServo,
                              AIServo &s5=nullAIServo, AIServo &s6=nullAIServo, AIServo &s7=nullAIServo, AIServo &s8=nullAIServo,
                              AIServo &s9=nullAIServo, AIServo &s10=nullAIServo, AIServo &s11=nullAIServo, AIServo &s12=nullAIServo,
                              AIServo &s13=nullAIServo, AIServo &s14=nullAIServo, AIServo &s15=nullAIServo, AIServo &s16=nullAIServo,
                              AIServo &s17=nullAIServo, AIServo &s18=nullAIServo, AIServo &s19=nullAIServo, AIServo &s20=nullAIServo,
                              AIServo &s21=nullAIServo, AIServo &s22=nullAIServo, AIServo &s23=nullAIServo, AIServo &s24=nullAIServo,
                              AIServo &s25=nullAIServo, AIServo &s26=nullAIServo, AIServo &s27=nullAIServo, AIServo &s28=nullAIServo,
                              AIServo &s29=nullAIServo, AIServo &s30=nullAIServo, AIServo &s31=nullAIServo, AIServo &s32=nullAIServo,
                              AIServo &s33=nullAIServo, AIServo &s34=nullAIServo, AIServo &s35=nullAIServo, AIServo &s36=nullAIServo,
                              AIServo &s37=nullAIServo, AIServo &s38=nullAIServo, AIServo &s39=nullAIServo, AIServo &s40=nullAIServo,
                              AIServo &s41=nullAIServo, AIServo &s42=nullAIServo, AIServo &s43=nullAIServo, AIServo &s44=nullAIServo,
                              AIServo &s45=nullAIServo, AIServo &s46=nullAIServo, AIServo &s47=nullAIServo, AIServo &s48=nullAIServo,
                              AIServo &s49=nullAIServo, AIServo &s50=nullAIServo, AIServo &s51=nullAIServo, AIServo &s52=nullAIServo,
                              AIServo &s53=nullAIServo, AIServo &s54=nullAIServo, AIServo &s55=nullAIServo, AIServo &s56=nullAIServo,
                              AIServo &s57=nullAIServo, AIServo &s58=nullAIServo, AIServo &s59=nullAIServo, AIServo &s60=nullAIServo,
                              AIServo &s61=nullAIServo, AIServo &s62=nullAIServo, AIServo &s63=nullAIServo, AIServo &s64=nullAIServo);

void aiEnableMixing (void);
void aiDisableMixing (void);

#define CONSTRAINED_CUBIC    (30)
#define NATURAL_CUBIC        (40)
void aiservoBeginSplineMotion(int mode, AIServoFrame *Frames, unsigned long *frameTime, int numFrames);
void aiservoBeginSplineMotion(int mode, AIServoFrame **Frames, unsigned long *frameTime, int numFrames);
void aiservoEndSplineMotion();

#endif

