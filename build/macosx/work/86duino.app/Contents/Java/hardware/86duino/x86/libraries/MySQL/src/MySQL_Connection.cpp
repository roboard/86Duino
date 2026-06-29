/*
  Copyright (c) 2012, 2016 Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

  MySQL_Connection.cpp - Library for communicating with a MySQL Server over
                         Ethernet. (formerly mysql.cpp)

  Change History:

  Version 1.0.0a Created by Dr. Charles A. Bell, April 2012.
  Version 1.0.0b Updated by Dr. Charles A. Bell, October 2013.
  Version 1.0.1b Updated by Dr. Charles A. Bell, February 2014.
  Version 1.0.2b Updated by Dr. Charles A. Bell, April 2014.
  Version 1.0.3rc Updated by Dr. Charles A. Bell, March 2015.
  Version 1.0.4ga Updated by Dr. Charles A. Bell, July 2015.
  Version 1.1.0a Created by Dr. Charles A. Bell, January 2016.
  Version 1.1.1a Created by Dr. Charles A. Bell, January 2016.
  Version 1.1.2b Created by Dr. Charles A. Bell, November 2016.
  Version 1.2.0 Created by Dr. Charles A. Bell, March 2020.
*/
#include <Arduino.h>
#include <MySQL_Connection.h>
#include <MySQL_Encrypt_Sha1.h>

#define MAX_CONNECT_ATTEMPTS 3
#define CONNECT_DELAY_MS     500
#define SUCCESS              1

const char CONNECTED[] PROGMEM = "Connected to server version ";
const char DISCONNECTED[] PROGMEM = "Disconnected.";

    
/*
  connect - Connect to a MySQL server.

  This method is used to connect to a MySQL server. It will attempt to
  connect to the server as a client retrying up to MAX_CONNECT_ATTEMPTS.
  This permits the possibility of longer than normal network lag times
  for wireless networks. You can adjust MAX_CONNECT_ATTEMPTS to suit
  your environment.

  server[in]      IP address of the server as IPAddress type
  port[in]        port number of the server
  user[in]        user name
  password[in]    (optional) user password
  db[in]          (optional) default database

  Returns boolean - True = connection succeeded
*/
boolean MySQL_Connection::connect(IPAddress server, int port, char *user,
                                  char *password, char *db)
{
  int connected = 0;
  int retries = MAX_CONNECT_ATTEMPTS;
  
  // Retry up to MAX_CONNECT_ATTEMPTS times.
  while (retries--)
  {
    Serial.println("...trying...");
    connected = client->connect(server, port);
    if (connected != SUCCESS) {
      Serial.print("...got: ");
      Serial.print(connected);
      Serial.println(" retrying...");
      delay(CONNECT_DELAY_MS);
    } else {
      break;
    }
  }

  if (connected != SUCCESS)
    return false;

  read_packet(true);
  parse_handshake_packet();
  send_authentication_packet(user, password, db);
  read_packet(true);
  if (get_packet_type() != MYSQL_OK_PACKET) {
    parse_error_packet();
    return false;
  }

  show_error(CONNECTED);

  Serial.println(server_version);

  free(server_version); // don't need it anymore
  
  if (_queue == NULL && buf == NULL) {
    _queue = CreateQueue(5120);
    buf = (char *)malloc(5120);
    if (buf == NULL) Serial.println("malloc buf fail. Stop.");
  }
  
  return true;
}

boolean MySQL_Connection::isBufEmpty() {
    if (_queue == NULL) return true;
    return QueueEmpty(_queue);
}

/*
  close - cancel the connection

  This method closes the connection to the server and frees up any memory
  used in the buffer.
*/
void MySQL_Connection::close()
{
  if (connected())
  {
    client->flush();
    client->stop();
    show_error(DISCONNECTED, true);
    DestoryQueue(_queue); _queue = NULL;
    free(buf); buf = NULL;
    ssize = sres = sn = 0;
    state = 0;
  }
}

int MySQL_Connection::handleData(void) {
    int ret = 0;
    
    if (_queue == NULL || buf == NULL) return -3;
    
    if (!connected()) {
        Serial.println("NO CONNECTED");
        return -2;
    }
    
    switch (state) {
    case 0 :
        if (QueueEmpty(_queue)) {
            ssize = sres = 0;
            state = 3; // start to send "heart beat" signal
        } else {
            PopQueueSeq(_queue, buf, 5);
            ssize = buf[0] + (((unsigned long)buf[1]) << 8) + (((unsigned long)buf[2]) << 16);
            ssize--;
            PopQueueSeq(_queue, buf + 5, ssize);
            ssize += 5;
            sres = ssize;
            sn = 0;
            state = 1;
        }
        break;
    case 1:
        if (sn < ssize) {
            if ((ret = client->writeNB((const uint8_t*)&buf[sn], sres)) < 0)
                return -4;
            sn += ret;
            sres -= ret;
        } else {
            // client->flush();
            state = 2;
        }
        break;
    case 2: /* read_packet */
        ret = read_packet(false);
        if (ret == 1) { // 1 : read complete
            client->flush(); // we flush here. (Althought we always discard all data :D)
            if (!QueueEmpty(_queue)) state = 0; // if we have next data, go ahead
            else state = 3;
        } else if (ret == -1)  { // -1 : timeout, if it is true, it will be keep off by connected(), becuase client->stop() will be call by wait_to_byte()
            show_error(DISCONNECTED, true);
            DestoryQueue(_queue); _queue = NULL;
            free(buf); buf = NULL;
            ssize = sres = sn = 0;
            state = 0; // when reconnect manually, state will be 0
        }
        break;
    case 3: // heart beat signal
        // Serial.println("heart beat signal");
        if (buffer != NULL) free(buffer);
        buffer = (byte*) malloc(17+5);
        memcpy(&buffer[5], "SELECT ASCII('a')", 17);
        store_int(&buffer[0], 17+1, 3);
        buffer[3] = byte(0x00);
        buffer[4] = byte(0x03);
  
        client->write((const uint8_t*)&buffer[0], 17+5); // it is block function, but should be enough fast (maybe)
        wait_till = millis() + 2000; // delay 2s
        state = 4;
        break;
    case 4:
        if (millis() > wait_till) state = 2;
        else if (!QueueEmpty(_queue)) {client->flush(); state = 0;}
        break;
    }
    
    return ret;
}
