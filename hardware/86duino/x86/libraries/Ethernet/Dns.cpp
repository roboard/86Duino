/*
  Dns.h - Part of DM&P Vortex86 Ethernet library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

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

#include "SwsSock.h"
#include "EthernetUdp.h"

#include "Dns.h"
#include <string.h>
#include "stdint.h"

DNSClient::DNSClient()
{
	iDNSServer = IPADDR_NONE;
}

void DNSClient::begin(const IPAddress& aDNSServer)
{
    iDNSServer = aDNSServer;
}

int DNSClient::inet_aton(const char* aIPAddrString, IPAddress& aResult)
{
    // See if we've been given a valid IP address
    const char* p =aIPAddrString;
    while (*p &&
           ( (*p == '.') || (*p >= '0') || (*p <= '9') ))
    {
        p++;
    }

    if (*p == '\0')
    {
        // It's looking promising, we haven't found any invalid characters
        p = aIPAddrString;
        int segment =0;
        int segmentValue =0;
        while (*p && (segment < 4))
        {
            if (*p == '.')
            {
                // We've reached the end of a segment
                if (segmentValue > 255)
                {
                    // You can't have IP address segments that don't fit in a byte
                    return 0;
                }
                else
                {
                    aResult[segment] = (uint8_t)segmentValue;
                    segment++;
                    segmentValue = 0;
                }
            }
            else
            {
                // Next digit
                segmentValue = (segmentValue*10)+(*p - '0');
            }
            p++;
        }
        // We've reached the end of address, but there'll still be the last
        // segment to deal with
        if ((segmentValue > 255) || (segment > 3))
        {
            // You can't have IP address segments that don't fit in a byte,
            // or more than four segments
            return 0;
        }
        else
        {
            aResult[segment] = (uint8_t)segmentValue;
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

int DNSClient::getHostByName(const char* aHostname, IPAddress& aResult)
{
	SWS_u_long result;
	struct SWS_hostent *Host;
	
    // See if it's a numeric IP address
    if (inet_aton(aHostname, aResult))
    {
        // It is, our work here is done
        return 1;
    }

    // Check we've got a valid DNS server to use
    if (iDNSServer == IPADDR_NONE)
        return 0;
	
    // Find a socket to use
	Host = SWS_gethostbyname(aHostname);
	if (Host == NULL)
		return 0;
	
	result = *(SWS_u_long*)Host->h_addr_list[0];
	
	aResult[0] = (uint8_t)(result);
	aResult[1] = (uint8_t)(result >> 8);
	aResult[2] = (uint8_t)(result >> 16);
	aResult[3] = (uint8_t)(result >> 24);

	return 1;
}
