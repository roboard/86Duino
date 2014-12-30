#include <GSM3ShieldV1ModemCore.h>
#include <Arduino.h>

GSM3ShieldV1ModemCore theGSM3ShieldV1ModemCore;

char* __ok__="OK";

extern bool hwcomIsUsed;
extern int hwcomport;

GSM3ShieldV1ModemCore::GSM3ShieldV1ModemCore() : gss()
{
	gss.registerMgr(this);
	_dataInBufferFrom=0;
	_dataInBufferTo=0;
	commandError=1;
	commandCounter=0;
	ongoingCommand=NONE;
	takeMilliseconds();
	
	for(int i=0;i<UMPROVIDERS;i++)
		UMProvider[i]=0;
}

void GSM3ShieldV1ModemCore::registerUMProvider(GSM3ShieldV1BaseProvider* provider)
{
	for(int i=0;i<UMPROVIDERS;i++)
	{
		if(UMProvider[i]==0)
		{
			UMProvider[i]=provider;
			break;
		}

	}

}

void GSM3ShieldV1ModemCore::unRegisterUMProvider(GSM3ShieldV1BaseProvider* provider)
{
	for(int i=0;i<UMPROVIDERS;i++)
	{
		if(UMProvider[i]==provider)
		{
			UMProvider[i]=0;
			break;
		}
	}
}


//Response parse.
bool GSM3ShieldV1ModemCore::genericParse_rsp(bool& rsp, char* string, char* string2)
{
	if((string==0) && (string2==0))
		string=__ok__;

	rsp=theBuffer().locate(string);
	
	if((!rsp)&&(string2!=0))
		rsp=theBuffer().locate(string2);
	
	return true;
}

void GSM3ShieldV1ModemCore::closeCommand(int code)
{
	// If we were configuring the modem,
	// and there's been an error
	// we don't know exactly where we are
	if((code!=1)&&(theGSM3ShieldV1ModemCore.getOngoingCommand()==MODEMCONFIG))
		theGSM3ShieldV1ModemCore.setStatus(ERROR);

	setCommandError(code);
	ongoingCommand=NONE;
	activeProvider=0;
	commandCounter=1;
}

//Generic command (stored in flash).
void GSM3ShieldV1ModemCore::genericCommand_rq(char str[], bool addCR)
{
	theBuffer().flush();
	writePGM(str, addCR);	
}

//Generic command (const string).
void GSM3ShieldV1ModemCore::genericCommand_rqc(const char* str, bool addCR)
{
	theBuffer().flush();
	print(str);
	if(addCR)
		print("\r");
}

// If we are not debugging, lets manage data in interrupt time
// but if we are not, just take note.
void GSM3ShieldV1ModemCore::manageMsg(byte from, byte to)
{
	if(_debug)
	{
		_dataInBufferFrom=from;
		_dataInBufferTo=to;
	}
	else
	{
		manageMsgNow(from, to);
	}
}

void GSM3ShieldV1ModemCore::manageReceivedData()
{
	bool fullbuffer;
	bool firstByte=true;
	byte thisHead;
	uint8_t d = 0;
	
	if(_debug)
	{
/*		Serial.print(theBuffer().getHead());
		Serial.print(" ");
		Serial.println(theBuffer().getTail());*/
		if(_dataInBufferFrom != _dataInBufferTo)
		{
			theBuffer().debugBuffer();
			manageMsgNow(_dataInBufferFrom, _dataInBufferTo);
			_dataInBufferFrom=0;
			_dataInBufferTo=0;
		}
	}
	else if(hwcomIsUsed == true)
	{
		// simulate the functions in original soft-Serial ISR, but get one byte one time.
		if(HWSerial[hwcomport]->available() > 0)
		{
			fullbuffer = (gss.cb.availableBytes()<6);
			if(!fullbuffer)
			{
				d = HWSerial[hwcomport]->read();
				if(gss.keepThisChar(&d))
				{
					gss.cb.write(d);
					if(firstByte)
					{
						firstByte=false;
						thisHead=gss.cb.getTail();
					}
				}
				if(d==10)
					manageMsg(thisHead, gss.cb.getTail());
				else if (d==32)
					manageMsg(thisHead, gss.cb.getTail());
			}
			else
				manageMsg(thisHead, gss.cb.getTail()); 
		}
	}
	else
	{
		// Just debugging the non debugging
		//		Serial.println();
		//		Serial.print("Com:");
		//		Serial.print(ongoingCommand);
		//		Serial.print("   Step:");
		//		Serial.print(commandCounter);
	}
}

//Select between URC or response.
void GSM3ShieldV1ModemCore::manageMsgNow(byte from, byte to)
{
	bool recognized=false;
	
	for(int i=0;(i<UMPROVIDERS)&&(!recognized);i++)
	{
		if(UMProvider[i])
			recognized=UMProvider[i]->recognizeUnsolicitedEvent(from);
	}
	if((!recognized)&&(activeProvider))
		activeProvider->manageResponse(from, to);
}


void GSM3ShieldV1ModemCore::openCommand(GSM3ShieldV1BaseProvider* provider, GSM3_commandType_e c)
{	
	activeProvider=provider;
	commandError=0;
	commandCounter=1;
	ongoingCommand=c;
	_dataInBufferFrom=0;
	_dataInBufferTo=0;

};

size_t GSM3ShieldV1ModemCore::writePGM(char str[], bool CR)
{
	int i=0;
	char c;
	
	do
	{	
		c=*(str + i); 
		if(c!=0)
			write(c);
		i++;
	} while (c!=0);
	if(CR)
		print("\r");
		
	return 1;
}

size_t GSM3ShieldV1ModemCore::write(uint8_t c)
{
	if(_debug)
		GSM3CircularBuffer::printCharDebug(c);
	return gss.write(c);
}

unsigned long GSM3ShieldV1ModemCore::takeMilliseconds()
{
	unsigned long now=millis();
	unsigned long delta;
	delta=now-milliseconds;
	milliseconds=now;
	return delta;
}

void GSM3ShieldV1ModemCore::delayInsideInterrupt(unsigned long milliseconds)
{
	for (unsigned long k=0;k<milliseconds;k++)  
		theGSM3ShieldV1ModemCore.gss.tunedDelay(1000); 
}
