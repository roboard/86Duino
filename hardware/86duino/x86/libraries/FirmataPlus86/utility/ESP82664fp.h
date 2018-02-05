/**
 * @file ESP82664fp.h
 * @brief The definition of class ESP82664fp. 
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
#ifndef __ESP82664FP_H__
#define __ESP82664FP_H__

#include <queue>
#include "queue.h"
#include "Arduino.h"


//#define ESP8266_USE_SOFTWARE_SERIAL


#ifdef ESP8266_USE_SOFTWARE_SERIAL
#include "SoftwareSerial.h"
#endif


/**
 * Provide an easy-to-use way to manipulate ESP8266. 
 */
class ESP82664fp {
 public:
	ESP82664fp(void);
    std::queue<unsigned char> wifi_data;
    void init(HardwareSerial &uart, uint32_t baud = 115200);
	void init(HardwareSerial &uart, uint32_t baud, uint8_t pin);
    /** 
     * Verify ESP8266 whether live or not. 
     *
     * Actually, this method will send command "AT" to ESP8266 and waiting for "OK". 
     * 
     * @retval true - alive.
     * @retval false - dead.
     */
    bool kick(void);
    
    /**
     * Restart ESP8266 by "AT+RST". 
     *
     * This method will take 3 seconds or more. 
     *
     * @retval true - success.
     * @retval false - failure.
     */
    bool restart(void);
    
    /**
     * Get the version of AT Command Set. 
     * 
     * @return the string of version. 
     */
    bool getVersion(char* version, int size);
    
    /**
     * Set operation mode to staion. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool setOprToStation(char* str_mode, int size);
    
    /**
     * Set operation mode to softap. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool setOprToSoftAP(char* str_mode, int size);
    
    /**
     * Set operation mode to station + softap. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool setOprToStationSoftAP(char* str_mode, int size);
    
    /**
     * Search available AP list and return it.
     * 
     * @return the list of available APs. 
     * @note This method will occupy a lot of memeory(hundreds of Bytes to a couple of KBytes). 
     *  Do not call this method unless you must and ensure that your board has enough memery left.
     */
    bool getAPList(char* list, int size);
    
    /**
     * Join in AP. 
     *
     * @param ssid - SSID of AP to join in. 
     * @param pwd - Password of AP to join in. 
     * @retval true - success.
     * @retval false - failure.
     * @note This method will take a couple of seconds. 
     */
    bool joinAP(String ssid, String pwd);
    
    /**
     * Leave AP joined before. 
     *
     * @retval true - success.
     * @retval false - failure.
     */
    bool leaveAP(void);
    
    /**
     * Set SoftAP parameters. 
     * 
     * @param ssid - SSID of SoftAP. 
     * @param pwd - PASSWORD of SoftAP. 
     * @param chl - the channel (1 - 13, default: 7). 
     * @param ecn - the way of encrypstion (0 - OPEN, 1 - WEP, 
     *  2 - WPA_PSK, 3 - WPA2_PSK, 4 - WPA_WPA2_PSK, default: 4). 
     * @note This method should not be called when station mode. 
     */
    bool setSoftAPParam(String ssid, String pwd, uint8_t chl = 7, uint8_t ecn = 4);
    
    /**
     * Get the IP list of devices connected to SoftAP. 
     * 
     * @return the list of IP.
     * @note This method should not be called when station mode. 
     */
    bool getJoinedDeviceIP(char* list, int size);
    
    /**
     * Get the current status of connection(UDP and TCP). 
     * 
     * @return the status. 
     */
    bool getIPStatus(char* list, int size);
    
    /**
     * Get the IP address of ESP8266. 
     *
     * @retval true - success.
     * @retval false - failure. 
     */
    bool getLocalIP(int* list, int size);
    
    void digitalLocalIP(char* ip, int* list);
    
    /**
     * Get the Gateway address and Subnetmask address of ESP8266. 
     *
     * @retval true - success.
     * @retval false - failure. 
     */
    bool getGatewayAndSubnetMask(int* gateway, int gsize, int* subnetmask, int ssize);
    
    /**
     * Enable IP MUX(multiple connection mode). 
     *
     * In multiple connection mode, a couple of TCP and UDP communication can be builded. 
     * They can be distinguished by the identifier of TCP or UDP named mux_id. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool enableMUX(void);
    
    /**
     * Disable IP MUX(single connection mode). 
     *
     * In single connection mode, only one TCP or UDP communication can be builded. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool disableMUX(void);
    
    
    /**
     * Create TCP connection in single mode. 
     * 
     * @param addr - the IP or domain name of the target host. 
     * @param port - the port number of the target host. 
     * @retval true - success.
     * @retval false - failure.
     */
    bool createTCP(String addr, uint32_t port);
    
    /**
     * Release TCP connection in single mode. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool releaseTCP(void);
    
    /**
     * Register UDP port number in single mode.
     * 
     * @param addr - the IP or domain name of the target host. 
     * @param port - the port number of the target host. 
     * @retval true - success.
     * @retval false - failure.
     */
    bool registerUDP(String addr, uint32_t port);
    
    /**
     * Unregister UDP port number in single mode. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool unregisterUDP(void);
  
    /**
     * Create TCP connection in multiple mode. 
     * 
     * @param mux_id - the identifier of this TCP(available value: 0 - 4). 
     * @param addr - the IP or domain name of the target host. 
     * @param port - the port number of the target host. 
     * @retval true - success.
     * @retval false - failure.
     */
    bool createTCP(uint8_t mux_id, String addr, uint32_t port);
    
    /**
     * Release TCP connection in multiple mode. 
     * 
     * @param mux_id - the identifier of this TCP(available value: 0 - 4). 
     * @retval true - success.
     * @retval false - failure.
     */
    bool releaseTCP(uint8_t mux_id);
    
    /**
     * Register UDP port number in multiple mode.
     * 
     * @param mux_id - the identifier of this TCP(available value: 0 - 4). 
     * @param addr - the IP or domain name of the target host. 
     * @param port - the port number of the target host. 
     * @retval true - success.
     * @retval false - failure.
     */
    bool registerUDP(uint8_t mux_id, String addr, uint32_t port);
    
    /**
     * Unregister UDP port number in multiple mode. 
     * 
     * @param mux_id - the identifier of this TCP(available value: 0 - 4). 
     * @retval true - success.
     * @retval false - failure.
     */
    bool unregisterUDP(uint8_t mux_id);


    /**
     * Set the timeout of TCP Server. 
     * 
     * @param timeout - the duration for timeout by second(0 ~ 28800, default:180). 
     * @retval true - success.
     * @retval false - failure.
     */
    bool setTCPServerTimeout(uint32_t timeout = 180);
    
    /**
     * Start TCP Server(Only in multiple mode). 
     * 
     * After started, user should call method: getIPStatus to know the status of TCP connections. 
     * The methods of receiving data can be called for user's any purpose. After communication, 
     * release the TCP connection is needed by calling method: releaseTCP with mux_id. 
     *
     * @param port - the port number to listen(default: 333).
     * @retval true - success.
     * @retval false - failure.
     *
     * @see String getIPStatus(void);
     * @see uint32_t recv(uint8_t *coming_mux_id, uint8_t *buffer, uint32_t len, uint32_t timeout);
     * @see bool releaseTCP(uint8_t mux_id);
     */
    bool startTCPServer(uint32_t port = 333);

    /**
     * Stop TCP Server(Only in multiple mode). 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool stopTCPServer(void);
    
    /**
     * Start Server(Only in multiple mode). 
     * 
     * @param port - the port number to listen(default: 333).
     * @retval true - success.
     * @retval false - failure.
     *
     * @see String getIPStatus(void);
     * @see uint32_t recv(uint8_t *coming_mux_id, uint8_t *buffer, uint32_t len, uint32_t timeout);
     */
    bool startServer(uint32_t port = 333);

    /**
     * Stop Server(Only in multiple mode). 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool stopServer(void);

    /**
     * Send data based on TCP or UDP builded already in single mode. 
     * 
     * @param buffer - the buffer of data to send. 
     * @param len - the length of data to send. 
     * @retval true - success.
     * @retval false - failure.
     */
    bool send(const uint8_t *buffer, uint32_t len);
            
    /**
     * Send data based on one of TCP or UDP builded already in multiple mode. 
     * 
     * @param mux_id - the identifier of this TCP(available value: 0 - 4). 
     * @param buffer - the buffer of data to send. 
     * @param len - the length of data to send. 
     * @retval true - success.
     * @retval false - failure.
     */
	
	bool send_nb(uint8_t mux_id, const uint8_t *buffer, uint32_t len);
    bool send(uint8_t mux_id, const uint8_t *buffer, uint32_t len);
    
    /**
     * Receive data from TCP or UDP builded already in single mode. 
     *
     * @param buffer - the buffer for storing data. 
     * @param buffer_size - the length of the buffer. 
     * @param timeout - the time waiting data. 
     * @return the length of data received actually. 
     */
    uint32_t recv(uint8_t *buffer, uint32_t buffer_size, uint32_t timeout = 1000);
    
    /**
     * Receive data from one of TCP or UDP builded already in multiple mode. 
     *
     * @param mux_id - the identifier of this TCP(available value: 0 - 4). 
     * @param buffer - the buffer for storing data. 
     * @param buffer_size - the length of the buffer. 
     * @param timeout - the time waiting data. 
     * @return the length of data received actually. 
     */
    uint32_t recv(uint8_t mux_id, uint8_t *buffer, uint32_t buffer_size, uint32_t timeout = 1000);
    
    /**
     * Receive data from all of TCP or UDP builded already in multiple mode. 
     *
     * After return, coming_mux_id store the id of TCP or UDP from which data coming. 
     * User should read the value of coming_mux_id and decide what next to do. 
     * 
     * @param coming_mux_id - the identifier of TCP or UDP. 
     * @param buffer - the buffer for storing data. 
     * @param buffer_size - the length of the buffer. 
     * @param timeout - the time waiting data. 
     * @return the length of data received actually. 
     */
    uint32_t recv(uint8_t *coming_mux_id, uint8_t *buffer, uint32_t buffer_size, uint32_t timeout = 1000);
	uint32_t recv_nb(uint8_t *coming_mux_id, uint8_t *buffer, uint32_t buffer_size);
	
    bool enableEcho();
	bool disableEcho();
    
    bool clientConnected;

 private:

	int pushData_now();
    int checkString(char* c, int size, int startpoint, char* ch);
	int read_until(char* ret, int ret_size, char terminator1, char terminator2);
    bool rx_parser(char* ret, int ret_size);
    /* 
     * Empty the buffer or UART RX.
     */
    void rx_empty(void);
 
    /* 
     * Recvive data from uart. Return all received data if target found or timeout. 
     */
    char* recvString(char* target, uint32_t timeout = 1000);
    
    /* 
     * Recvive data from uart. Return all received data if one of target1 and target2 found or timeout. 
     */
    char* recvString(char* target1, char* target2, uint32_t timeout = 1000);
    
    /* 
     * Recvive data from uart. Return all received data if one of target1, target2 and target3 found or timeout. 
     */
    char* recvString(char* target1, char* target2, char* target3, uint32_t timeout = 1000);
    
    /* 
     * Recvive data from uart and search first target. Return true if target found, false for timeout.
     */
    bool recvFind(char* target, uint32_t timeout = 1000);
    
    /* 
     * Recvive data from uart and search first target and cut out the substring between begin and end(excluding begin and end self). 
     * Return true if target found, false for timeout.
     */
    bool recvFindAndFilter(char* target, char* begin, char* end, char* data, int size, uint32_t timeout = 1000);
    
    void subString(char* c, int32_t index1, int32_t index2, char* data, int datasize); 
    /*
     * Receive a package from uart. 
     *
     * @param buffer - the buffer storing data. 
     * @param buffer_size - guess what!
     * @param data_len - the length of data actually received(maybe more than buffer_size, the remained data will be abandoned).
     * @param timeout - the duration waitting data comming.
     * @param coming_mux_id - in single connection mode, should be NULL and not NULL in multiple. 
     */
    uint32_t recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id);
    uint32_t recvPkg_nb(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint8_t *coming_mux_id);
    int Sentence(char* ret, int size);
    
    bool eAT(void);
    bool eATRST(void);
    bool eATGMR(char* version, int size);
    
    bool qATCWMODE(uint8_t *mode, char* str_mode, int size);
    bool sATCWMODE(uint8_t mode);
    bool sATCWJAP(String ssid, String pwd);
    bool eATCWLAP(char* list, int size);
    bool eATCWQAP(void);
    bool sATCWSAP(String ssid, String pwd, uint8_t chl, uint8_t ecn);
    bool eATCWLIF(char* list, int size);
    
    bool eATCIPSTATUS(char* list, int size);
    bool eATCIPSTA(int* gateway, int gsize, int* subnetmask, int ssize);
    bool eATCIPAP(int* gateway, int gsize, int* subnetmask, int ssize);
    bool sATCIPSTARTSingle(String type, String addr, uint32_t port);
    bool sATCIPSTARTMultiple(uint8_t mux_id, String type, String addr, uint32_t port);
    bool sATCIPSENDSingle(const uint8_t *buffer, uint32_t len);
    bool sATCIPSENDMultiple(uint8_t mux_id, const uint8_t *buffer, uint32_t len);
	bool sATCIPSENDMultiple_nb(uint8_t mux_id, const uint8_t *buffer, uint32_t len);
    bool sATCIPCLOSEMulitple(uint8_t mux_id);
    bool eATCIPCLOSESingle(void);
    bool eATCIFSR(int* list, int size);
    bool sATCIPMUX(uint8_t mode);
    bool sATCIPSERVER(uint8_t mode, uint32_t port = 333);
    bool sATCIPSTO(uint32_t timeout);
	
    Queue* q;
    int s_id;
	int s_len;
    int s_len_index;
    char nbData[1024];
    bool locked;
    
	int nb_state_l;
	int nb_state_s;
	unsigned long nb_time_s;
	unsigned long nb_time_l;
    
    /*
     * +IPD,len:data
     * +IPD,id,len:data
     */
    
#ifdef ESP8266_USE_SOFTWARE_SERIAL
    SoftwareSerial *m_puart; /* The UART to communicate with ESP8266 */
#else
    HardwareSerial *m_puart; /* The UART to communicate with ESP8266 */
#endif
};

#endif /* #ifndef __ESP8266_H__ */

