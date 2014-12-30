#include <GSM3ShieldV1BaseProvider.h>
#include <GSM3ShieldV1ModemCore.h>
#include <Arduino.h>

// Returns 0 if last command is still executing
// 1 if success
// >1 if error 
int GSM3ShieldV1BaseProvider::ready() 
{   
	theGSM3ShieldV1ModemCore.manageReceivedData();

	return theGSM3ShieldV1ModemCore.getCommandError();
};

void GSM3ShieldV1BaseProvider::prepareAuxLocate(char str[], char auxLocate[])
{
	int i=0;
	char c;

	do
	{	
		c=*(str + i); 
		auxLocate[i]=c;
		i++;
	} while (c!=0);
}

