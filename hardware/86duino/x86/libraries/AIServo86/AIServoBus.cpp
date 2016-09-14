/*
  AIServoBus.cpp - The part of Servo library
  
  Copyright (c) Designer 
    2016 Android Lin <acen@dmp.com.tw>. All right reserved.
    2016 AAA <aaa@dmp.com.tw>. All right reserved.
  Copyright (c) Programmer
    2016 Sayter <sayter@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#include "AIServoBus.h"

bool AIServoBus::contained(unsigned char id)
{
	bool res = false;
	static AIServoList* tmp;
	tmp = aisHead;
	while(tmp->next != NULL)
	{
		if(tmp->next->id == id)
		{
			res = true;
			break;
		}
		tmp = tmp->next;
	}
	return res;
}

void AIServoBus::del(unsigned char id)
{
	AIServoList* target = aisHead;
	while(target->next != NULL)
	{
		if(target->next->id == id)
		{
			AIServoList* tmp = target->next;
			target->next = target->next->next;
			delete(tmp);
			length--;
			return;
		}
		target = target->next;
	}
}

AIServoList* AIServoBus::tail()
{
	if(aisHead->next == NULL)
		return aisHead;
	else
	{
		AIServoList* tail = aisHead->next;
		while(tail->next != NULL)
			tail = tail->next;
		return tail;
	}
}

void AIServoBus::show()
{
	static AIServoList* node;
	node = aisHead;
	Serial.print("Length ");
	Serial.print(length);
	Serial.print(" : ");
	while(node->next != NULL)
	{
		Serial.print(node->next->id);
		Serial.print("->");
		node = node->next;
	}
	Serial.println("NULL");
}

