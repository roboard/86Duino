/**
 * @file ESP82664fp4fp.cpp
 * @brief The implementation of class ESP82664fp. 
 * @author Wu Pengfei<pengfei.wu@itead.cc> 
 * @date 2015.02
 *
 * @Modified by CJ Wu<sayter@dmp.com.tw>
 * @date 2017.01
 * 
 * @par Copyright:
 * Copyright (c) 2015 ITEAD Intelligent Systems Co., Ltd. \n\n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version. \n\n
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "utility/ESP82664fp.h"
#include "queue.h"

#define LOG_OUTPUT_DEBUG            (1)
#define LOG_OUTPUT_DEBUG_PREFIX     (1)

#define logDebug(arg)\
    do {\
        if (LOG_OUTPUT_DEBUG)\
        {\
            if (LOG_OUTPUT_DEBUG_PREFIX)\
            {\
                Serial.print("[LOG Debug: ");\
                Serial.print((const char*)__FILE__);\
                Serial.print(",");\
                Serial.print((unsigned int)__LINE__);\
                Serial.print(",");\
                Serial.print((const char*)__FUNCTION__);\
                Serial.print("] ");\
            }\
            Serial.print(arg);\
        }\
    } while(0)


ESP82664fp::ESP82664fp(void)
{
	nb_state_s = 0;
	nb_state_l = 0;
	nb_time_s = millis();
	nb_time_l = millis();
}

void ESP82664fp::init(HardwareSerial &uart, uint32_t baud)
{
	m_puart = &uart;
	m_puart->begin(baud);
    q = CreateQueue(4096);
    ClearQueue(q);
    for (int i=0; i<1024; i++) nbData[i] = '\0';
    clientConnected = false;
}

void ESP82664fp::init(HardwareSerial &uart, uint32_t baud, uint8_t pin)
{
	pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);
	m_puart = &uart;
	m_puart->begin(baud);
    q = CreateQueue(4096);
    ClearQueue(q);
    for (int i=0; i<1024; i++) nbData[i] = '\0';
    clientConnected = false;
}

bool ESP82664fp::kick(void)
{
    return eAT();
}

bool ESP82664fp::restart(void)
{
    unsigned long start;
    if (eATRST()) {
        delay(2000);
        start = millis();
        while (millis() - start < 3000) {
            if (eAT()) {
                delay(1500); /* Waiting for stable */
                return true;
            }
            delay(100);
        }
    }
    return false;
}

bool ESP82664fp::getVersion(char* version, int size)
{
    return eATGMR(version, size);
}

bool ESP82664fp::setOprToStation(char* str_mode, int size)
{
    uint8_t mode;
    if (!qATCWMODE(&mode, str_mode, size)) {
        return false;
    }
    if (mode == 1) {
        return true;
    } else {
        if (sATCWMODE(1) && restart()) {
            return true;
        } else {
            return false;
        }
    }
}

bool ESP82664fp::setOprToSoftAP(char* str_mode, int size)
{
    uint8_t mode;
    if (!qATCWMODE(&mode, str_mode, size)) {
        return false;
    }
    if (mode == 2) {
        return true;
    } else {
        if (sATCWMODE(2) && restart()) {
            return true;
        } else {
            return false;
        }
    }
}

bool ESP82664fp::setOprToStationSoftAP(char* str_mode, int size)
{
    uint8_t mode;
    if (!qATCWMODE(&mode, str_mode, size)) {
        return false;
    }
    if (mode == 3) {
        return true;
    } else {
        if (sATCWMODE(3) && restart()) {
            return true;
        } else {
            return false;
        }
    }
}

bool ESP82664fp::getAPList(char* list, int size)
{
    return eATCWLAP(list, size);
}

bool ESP82664fp::joinAP(String ssid, String pwd)
{
    return sATCWJAP(ssid, pwd);
}

bool ESP82664fp::leaveAP(void)
{
    return eATCWQAP();
}

bool ESP82664fp::setSoftAPParam(String ssid, String pwd, uint8_t chl, uint8_t ecn)
{
    return sATCWSAP(ssid, pwd, chl, ecn);
}

bool ESP82664fp::getJoinedDeviceIP(char* list, int size)
{
    return eATCWLIF(list, size);
}

bool ESP82664fp::getIPStatus(char* list, int size)
{
    return eATCIPSTATUS(list, size);
}

bool ESP82664fp::getLocalIP(int* list, int size)
{
    return eATCIFSR(list, size);
}

bool ESP82664fp::enableMUX(void)
{
    return sATCIPMUX(1);
}

bool ESP82664fp::disableMUX(void)
{
    return sATCIPMUX(0);
}

bool ESP82664fp::createTCP(String addr, uint32_t port)
{
    return sATCIPSTARTSingle("TCP", addr, port);
}

bool ESP82664fp::releaseTCP(void)
{
    return eATCIPCLOSESingle();
}

bool ESP82664fp::registerUDP(String addr, uint32_t port)
{
    return sATCIPSTARTSingle("UDP", addr, port);
}

bool ESP82664fp::unregisterUDP(void)
{
    return eATCIPCLOSESingle();
}

bool ESP82664fp::createTCP(uint8_t mux_id, String addr, uint32_t port)
{
    return sATCIPSTARTMultiple(mux_id, "TCP", addr, port);
}

bool ESP82664fp::releaseTCP(uint8_t mux_id)
{
    return sATCIPCLOSEMulitple(mux_id);
}

bool ESP82664fp::registerUDP(uint8_t mux_id, String addr, uint32_t port)
{
    return sATCIPSTARTMultiple(mux_id, "UDP", addr, port);
}

bool ESP82664fp::unregisterUDP(uint8_t mux_id)
{
    return sATCIPCLOSEMulitple(mux_id);
}

bool ESP82664fp::setTCPServerTimeout(uint32_t timeout)
{
    return sATCIPSTO(timeout);
}

bool ESP82664fp::startTCPServer(uint32_t port)
{
    if (sATCIPSERVER(1, port)) {
        return true;
    }
    return false;
}

bool ESP82664fp::stopTCPServer(void)
{
    sATCIPSERVER(0);
    restart();
    return false;
}

bool ESP82664fp::startServer(uint32_t port)
{
    return startTCPServer(port);
}

bool ESP82664fp::stopServer(void)
{
    return stopTCPServer();
}

bool ESP82664fp::send(const uint8_t *buffer, uint32_t len)
{
    return sATCIPSENDSingle(buffer, len);
}

bool ESP82664fp::send(uint8_t mux_id, const uint8_t *buffer, uint32_t len)
{
    return sATCIPSENDMultiple(mux_id, buffer, len);
}

bool ESP82664fp::send_nb(uint8_t mux_id, const uint8_t *buffer, uint32_t len)
{
    return sATCIPSENDMultiple_nb(mux_id, buffer, len);
}

uint32_t ESP82664fp::recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout)
{
    return recvPkg(buffer, buffer_size, NULL, timeout, NULL);
}

uint32_t ESP82664fp::recv(uint8_t mux_id, uint8_t *buffer, uint32_t buffer_size, uint32_t timeout)
{
    uint8_t id;
    uint32_t ret;
    ret = recvPkg(buffer, buffer_size, NULL, timeout, &id);
    if (ret > 0 && id == mux_id) {
        return ret;
    }
    return 0;
}

uint32_t ESP82664fp::recv(uint8_t *coming_mux_id, uint8_t *buffer, uint32_t buffer_size, uint32_t timeout)
{
    return recvPkg(buffer, buffer_size, NULL, timeout, coming_mux_id);
}

uint32_t ESP82664fp::recv_nb(uint8_t *coming_mux_id, uint8_t *buffer, uint32_t buffer_size)
{
	return recvPkg_nb(buffer, buffer_size, NULL, coming_mux_id);
}

/*----------------------------------------------------------------------------*/
/* +IPD,<id>,<len>:<data> */
/* +IPD,<len>:<data> */

uint32_t ESP82664fp::recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id)
{
    String data;
    char a;
    int32_t index_PIPDcomma = -1;
    int32_t index_colon = -1; /* : */
    int32_t index_comma = -1; /* , */
    int32_t len = -1;
    int8_t id = -1;
    bool has_data = false;
    uint32_t ret;
    unsigned long start;
    uint32_t i;
    
    if (buffer == NULL) {
        return 0;
    }
    
    start = millis();
    while (millis() - start < timeout) {
        if(m_puart->available() > 0) {
            a = m_puart->read();
            data += a;
        }
        
        index_PIPDcomma = data.indexOf("+IPD,");
        if (index_PIPDcomma != -1) {
            index_colon = data.indexOf(':', index_PIPDcomma + 5);
            if (index_colon != -1) {
                index_comma = data.indexOf(',', index_PIPDcomma + 5);
                /* +IPD,id,len:data */
                if (index_comma != -1 && index_comma < index_colon) { 
                    id = data.substring(index_PIPDcomma + 5, index_comma).toInt();
                    if (id < 0 || id > 4) {
                        return 0;
                    }
                    len = data.substring(index_comma + 1, index_colon).toInt();
                    if (len <= 0) {
                        return 0;
                    }
                } else { /* +IPD,len:data */
                    len = data.substring(index_PIPDcomma + 5, index_colon).toInt();
                    if (len <= 0) {
                        return 0;
                    }
                }
                has_data = true;
                break;
            }
        }
    }
    
    if (has_data) {
        i = 0;
        ret = len > buffer_size ? buffer_size : len;
        start = millis();
        while (millis() - start < 3000) {
            while(m_puart->available() > 0 && i < ret) {
                a = m_puart->read();
                buffer[i++] = a;
            }
            if (i == ret) {
                rx_empty();
                if (data_len) {
                    *data_len = len;    
                }
                if (index_comma != -1 && coming_mux_id) {
                    *coming_mux_id = id;
                }
                return ret;
            }
        }
    }
    return 0;
}

uint32_t ESP82664fp::recvPkg_nb(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint8_t *coming_mux_id)
{
    int i, j, slen;
    
    if (buffer == NULL) return 0;
    
    j = slen = 0;
    
    pushData_now();
    
    while (QueueEmpty(q) == false)
    {
        if (rx_parser(nbData, 1024) == true)
        {
            for (i=0; i<s_len; i++, s_len_index++, j++)
                buffer[j] = nbData[s_len_index];

            slen += s_len;
            
            if (coming_mux_id)
                *coming_mux_id = s_id;

            for (i=0; i<1024; i++) nbData[i] = '\0';
        }
    }

    if (slen > 0) return slen;
    return 0;
}

int ESP82664fp::pushData_now()
{
    while(m_puart->available() > 0)
        PushQueue(q, m_puart->read());
    return -1;
}

int ESP82664fp::checkString(char* c, int c_size, char* ch)
{
    int i, j;
    
    if (c == NULL || ch == NULL) return -1;
    
    for (i=0; i<c_size; i++)
    {
        if (ch[0] == c[i])
        {
            for (j=0; ch[j] != '\0' && ((i+j) < c_size); j++)
                if (ch[j] != c[i+j]) break;
            if (ch[j] == '\0') return i; // found, return index
        }
    }
    return -1; // Not found
}

// 0 : available
// 1 : now unavailable, but it will be available if waiting a minute
int ESP82664fp::Sentence(char* ret, int size) {
    int len = 0, i, headi, ncomma = 0, id = 0;
    char head[6] = "+IPD,";
    
    // find the head
    headi = checkString(ret, size, head);
    if (headi == -1) return 1;
        
    for (i=headi+5; i<size && ret[i] != ':'; i++)
        if (ret[i] == ',') ncomma++;
    
    if (i == size) return 1;
    i = headi + 5; // have ':' and index start behind "+IPD"

    // Get len
    if (ncomma == 1)
    {
        /* format : +IPD,<id>,<len>:<data> */
        id = ret[i] - '0'; // get 'id'
        
        // ignore ',' to next char
        for (i+=2; ret[i] != ':'; i++)
            len = len*10 + (ret[i] - '0');
    }
    else if (ncomma == 0)
    {
        /* +IPD,<len>:<data> */
        for (; ret[i] != ':'; i++)
            len = len*10 + (ret[i] - '0');
    }
    
    // Now i at ':' and we can take 'len' to analysis
    i++; // ignore ':'
    
    if ((i+len-1) < size)
    {
        s_id = id;
        s_len_index = i;
        s_len = len;
        return 0;
    }
    
    return 1;
}

int ESP82664fp::read_until(char* ret, int ret_size, char terminator1, char terminator2)
{
	static int i = 0; // need to set i as "static"
    int available = 1;
    
    pushData_now();

    if (QueueEmpty(q) == false)
        ret[i++] = PopQueue(q); // one byte one time
    
    if (i >= ret_size) {i = 0; return 2;}
    
    available = Sentence(ret, i);
    if (available == 0)
        i = 0; // OK and clear ret[]
    else if (available == 1)
        ; // Continue to read, do nothing
    
    return available;
}

bool ESP82664fp::rx_parser(char* ret, int ret_size)
{
    int i, r;
    char* tmp;
    
    r = read_until(ret, ret_size, '\r', '\n');
    if (r == 0)
    {
        if (strstr(ret, ">") != NULL)
            nb_state_s = 3;
        return true;
    }
    else if (r == 1)
    {
        if (strstr(ret, "0,CONNECT") != NULL)
            clientConnected = true;
        if (strstr(ret, "0,CLOSED") != NULL)
            clientConnected = false;
        if ((tmp = strstr(ret, "2,CONNECT")) != NULL)
        {
            sATCIPCLOSEMulitple(2);
            for (i=0; i<9; i++) *(ret+i) = 'A'; // "2,CONNECT" is replayed by dummy char 'A'
        }
    }
    else if (r == 2)
    {
        if (strstr(ret, "SEND OK") != NULL)
        {
            nb_state_s = 0;
            nb_time_s = millis();
        }
        
        // Clear ret[]
        for (i=0; i<ret_size; i++) ret[i] = '\0';
        return false;
    }
    
    return false;
}

void ESP82664fp::rx_empty(void) 
{
	char a;
    while(m_puart->available() > 0)
    {
		a = m_puart->read();
        PushQueue(q, a);
    }
}

char* ESP82664fp::recvString(char* target, uint32_t timeout)
{
    int i, j = 0, index;
    char c[1024];
    unsigned long tmp = millis();
    
    for (i=0; i<1024; i++) c[i] = '\0';
    
    while(1)
    {
        if ((millis() - tmp) > timeout) break;
        
        for (;m_puart->available(); j++)
        {
            c[j] = m_puart->read();
            PushQueue(q, c[j]);
        }
        
        index = checkString(c, j, target); // -1: not found; >=0: found
        
        if (index >= 0)
            return target;
    }
    return NULL;
}

char* ESP82664fp::recvString(char* target1, char* target2, uint32_t timeout)
{
    int i, j = 0, index;
    char c[1024];
    char* target = NULL;
    unsigned long tmp = millis();
    
    for (i=0; i<1024; i++) c[i] = '\0';
    
    while(1)
    {
        if ((millis() - tmp) > timeout) break;
        
        for (;m_puart->available(); j++)
        {
            c[j] = m_puart->read();
            PushQueue(q, c[j]);
        }
        
        index = checkString(c, j, target1); // -1: not found; >=0: found
        if (index >= 0)
        {
            target = target1;
            break;
        }
        
        index = checkString(c, j, target2); // -1: not found; >=0: found
        if (index >= 0)
        {
            target = target2;
            break;
        }
    }
    return target;
}

char* ESP82664fp::recvString(char* target1, char* target2, char* target3, uint32_t timeout)
{
    int i, j = 0, index;
    char c[1024];
    char* target = NULL;
    unsigned long tmp = millis();
    
    for (i=0; i<1024; i++) c[i] = '\0';
    
    while(1)
    {
        if ((millis() - tmp) > timeout) break;
        
        for (;m_puart->available(); j++)
        {
            c[j] = m_puart->read();
            PushQueue(q, c[j]);
        }
        
        index = checkString(c, j, target1); // -1: not found; >=0: found
        if (index >= 0)
        {
            target = target1;
            break;
        }
        
        index = checkString(c, j, target2); // -1: not found; >=0: found
        if (index >= 0)
        {
            target = target2;
            break;
        }
        
        index = checkString(c, j, target3); // -1: not found; >=0: found
        if (index >= 0)
        {
            target = target3;
            break;
        }
    }
    return target;
}

bool ESP82664fp::recvFind(char* target, uint32_t timeout)
{
    if (recvString(target, timeout) != NULL)
        return true;
    else
        return false;
}

void ESP82664fp::subString(char* c, int32_t index1, int32_t index2, char* data, int datasize) {
    int i, j;
    
    if ((index2 - index1) >= datasize) return;
    
    for (i=0; i<datasize; i++) data[i] = '\0';
    for (i=index1, j=0; i<index2; i++, j++)
        data[j] = c[i];

    return;
}

bool ESP82664fp::recvFindAndFilter(char* target, char* begin, char* end, char* data, int size, uint32_t timeout)
{
    int i, j = 0, index, index1, index2, beginSize;
    char c[1024];
    unsigned long tmp = millis();
    int status = 0;
    
    for (i=0; i<1024; i++) c[i] = '\0';
    
    while(1)
    {
        if ((millis() - tmp) > timeout) break;
        
        for (;m_puart->available(); j++)
        {
            c[j] = m_puart->read();
            PushQueue(q, c[j]);
        }
        
        if (status == 0)
        {
            index = checkString(c, j, target); // -1: not found; >=0: found
            if (index >= 0)
                status = 1;
        }
        else if (status == 1)
        {
            index1 = checkString(c, j, begin); // -1: not found; >=0: found
            if (index >= 0)
                status = 2;
        }
        else if (status == 2)
        {
            index2 = checkString(c, j, end); // -1: not found; >=0: found
            if (index >= 0)
                status = 3;
        }
        else if (status == 3)
        {
            for (i=0; begin[i] != '\0'; i++) beginSize++;
            subString(c, index1+beginSize, index2, data, size);
            return true;
        }
    }
    
    return false;
}

bool ESP82664fp::enableEcho(void)
{
    rx_empty();
    m_puart->println("ATE1");
    return recvFind("OK");
}

bool ESP82664fp::disableEcho(void)
{
    rx_empty();
    m_puart->println("ATE0");
    return recvFind("OK");
}

bool ESP82664fp::eAT(void)
{
    rx_empty();
    m_puart->println("AT");
    return recvFind("OK");
}

bool ESP82664fp::eATRST(void) 
{
    rx_empty();
    m_puart->println("AT+RST");
    return recvFind("ready");
}

bool ESP82664fp::eATGMR(char* version, int size)
{
    rx_empty();
    m_puart->println("AT+GMR");
    return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", version, size); 
}

bool ESP82664fp::qATCWMODE(uint8_t *mode, char* str_mode, int size) 
{
    bool ret;
    if (!mode) {
        return false;
    }
    rx_empty();
    m_puart->println("AT+CWMODE?");
    ret = recvFindAndFilter("OK", "+CWMODE:", "\r\n\r\nOK", str_mode, size); 
    if (ret) {
        *mode = (uint8_t)atoi(str_mode);
        return true;
    } else {
        return false;
    }
}

bool ESP82664fp::sATCWMODE(uint8_t mode)
{
    char* data;
    rx_empty();
    m_puart->print("AT+CWMODE=");
    m_puart->println(mode);
    
    data = recvString("OK", "no change");
    if (data != NULL)
        return true;
    else
        return false;
}

bool ESP82664fp::sATCWJAP(String ssid, String pwd)
{
    char* data;
    rx_empty();
    m_puart->print("AT+CWJAP=\"");
    m_puart->print(ssid);
    m_puart->print("\",\"");
    m_puart->print(pwd);
    m_puart->println("\"");
    
    data = recvString("OK", "FAIL", 10000);
    if (data != NULL && strcmp(data, "OK") == 0)
        return true;
    else
        return false;
}

bool ESP82664fp::eATCWLAP(char* list, int size)
{
    rx_empty();
    m_puart->println("AT+CWLAP");
    return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", list, size, 10000);
}

bool ESP82664fp::eATCWQAP(void)
{
    rx_empty();
    m_puart->println("AT+CWQAP");
    return recvFind("OK");
}

bool ESP82664fp::sATCWSAP(String ssid, String pwd, uint8_t chl, uint8_t ecn)
{
    char* data;
    rx_empty();
    m_puart->print("AT+CWSAP=\"");
    m_puart->print(ssid);
    m_puart->print("\",\"");
    m_puart->print(pwd);
    m_puart->print("\",");
    m_puart->print(chl);
    m_puart->print(",");
    m_puart->println(ecn);
    
    data = recvString("OK", "ERROR", 5000);
    if (data != NULL && strcmp(data, "OK") == 0)
        return true;
    else
        return false;
}

bool ESP82664fp::eATCWLIF(char* list, int size)
{
    rx_empty();
    m_puart->println("AT+CWLIF");
    return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", list, size);
}
bool ESP82664fp::eATCIPSTATUS(char* list, int size)
{
    delay(100);
    rx_empty();
    m_puart->println("AT+CIPSTATUS");
    return recvFindAndFilter("OK", "\r\r\n", "\r\n\r\nOK", list, size);
}
bool ESP82664fp::sATCIPSTARTSingle(String type, String addr, uint32_t port)
{
    char* data;
    rx_empty();
    m_puart->print("AT+CIPSTART=\"");
    m_puart->print(type);
    m_puart->print("\",\"");
    m_puart->print(addr);
    m_puart->print("\",");
    m_puart->println(port);
    
    data = recvString("OK", "ERROR", "ALREADY CONNECT", 10000);
    if (data != NULL && (strcmp(data, "OK") == 0 || strcmp(data, "ALREADY CONNECT") == 0))
        return true;
    else
        return false;
}
bool ESP82664fp::sATCIPSTARTMultiple(uint8_t mux_id, String type, String addr, uint32_t port)
{
    char* data;
    rx_empty();
    m_puart->print("AT+CIPSTART=");
    m_puart->print(mux_id);
    m_puart->print(",\"");
    m_puart->print(type);
    m_puart->print("\",\"");
    m_puart->print(addr);
    m_puart->print("\",");
    m_puart->println(port);
    
    data = recvString("OK", "ERROR", "ALREADY CONNECT", 10000);
    if (data != NULL && (strcmp(data, "OK") == 0 || strcmp(data, "ALREADY CONNECT") == 0))
        return true;
    else
        return false;
}
bool ESP82664fp::sATCIPSENDSingle(const uint8_t *buffer, uint32_t len)
{
    rx_empty();
    m_puart->print("AT+CIPSEND=");
    m_puart->println(len);
    if (recvFind(">", 5000)) {
        rx_empty();
        for (uint32_t i = 0; i < len; i++) {
            m_puart->write(buffer[i]);
        }
        return recvFind("SEND OK", 10000);
    }
    return false;
}
bool ESP82664fp::sATCIPSENDMultiple(uint8_t mux_id, const uint8_t *buffer, uint32_t len)
{
    rx_empty(); // rx_parser(nbData, 128);
    m_puart->print("AT+CIPSEND=");
    m_puart->print(mux_id);
    m_puart->print(",");
    m_puart->println(len);
    if (recvFind(">", 500)) {
        rx_empty(); // rx_parser(nbData, 128);
        for (uint32_t i = 0; i < len; i++) {
            m_puart->write(buffer[i]);
        }
        return recvFind("SEND OK", 10000);
    }
    return false;
}
bool ESP82664fp::sATCIPSENDMultiple_nb(uint8_t mux_id, const uint8_t *buffer, uint32_t len)
{
	switch (nb_state_s) {
		case 0:
			if (millis() - nb_time_s >= 15)
				nb_state_s = 1;
			break;
		case 1:
			m_puart->print("AT+CIPSEND=");
			m_puart->print(mux_id);
			m_puart->print(",");
			m_puart->println(len);
			nb_state_s = 2;
		case 2:
			rx_parser(nbData, 1024);
			if (nb_state_s == 3)
				break;
			else
				return false;
		case 3:
			for (uint32_t i = 0; i < len; i++)
				m_puart->write(buffer[i]);
			nb_state_s = 4;
		case 4:
			rx_parser(nbData, 1024);
			if (nb_state_s == 0)
				return true;
			break;
		default:
			break;
	}
    return false;
}
bool ESP82664fp::sATCIPCLOSEMulitple(uint8_t mux_id)
{
    char* data;
    rx_empty();
    m_puart->print("AT+CIPCLOSE=");
    m_puart->println(mux_id);
    
    data = recvString("OK", "link is not", 5000);
    if (data != NULL && (strcmp(data, "OK") == 0 || strcmp(data, "link is not") == 0))
        return true;
    else
        return false;
}
bool ESP82664fp::eATCIPCLOSESingle(void)
{
    rx_empty();
    m_puart->println("AT+CIPCLOSE");
    return recvFind("OK", 5000);
}
bool ESP82664fp::eATCIFSR(int* list, int size)
{
    char ip[64];
    bool result;
    rx_empty();
    m_puart->println("AT+CIFSR");
    result = recvFindAndFilter("OK", "+CIFSR:STAIP,\"", "\"\r\n", ip, 64);
    if (result == true)
    {
        for (int i=0; i<size; i++) list[i] = 0;
        digitalLocalIP(ip, list);
    }
    return result;
}
void ESP82664fp::digitalLocalIP(char* ip, int* list)
{
    int i;
    for (; ip[i] != '.'; i++)
        list[0] = list[0]*10 + (ip[i] - '0');
    for (i++; ip[i] != '.'; i++)
        list[1] = list[1]*10 + (ip[i] - '0');
    for (i++; ip[i] != '.'; i++)
        list[2] = list[2]*10 + (ip[i] - '0');
    for (i++; ip[i] != '\0'; i++)
        list[3] = list[3]*10 + (ip[i] - '0');
}
bool ESP82664fp::sATCIPMUX(uint8_t mode)
{
    char* data;
    rx_empty();
    m_puart->print("AT+CIPMUX=");
    m_puart->println(mode);
    
    data = recvString("OK", "Link is builded");
    if (data != NULL && strcmp(data, "OK") == 0)
        return true;
    else
        return false;
}
bool ESP82664fp::sATCIPSERVER(uint8_t mode, uint32_t port)
{
    char* data;
    if (mode) {
        rx_empty();
        m_puart->print("AT+CIPSERVER=1,");
        m_puart->println(port);
        
        data = recvString("OK", "no change");
        if (data != NULL && (strcmp(data, "OK") == 0 || strcmp(data, "no change") == 0))
            return true;
        else
            return false;
    } else {
        rx_empty();
        m_puart->println("AT+CIPSERVER=0");
        return recvFind("\r\r\n");
    }
}
bool ESP82664fp::sATCIPSTO(uint32_t timeout)
{
    rx_empty();
    m_puart->print("AT+CIPSTO=");
    m_puart->println(timeout);
    return recvFind("OK");
}

