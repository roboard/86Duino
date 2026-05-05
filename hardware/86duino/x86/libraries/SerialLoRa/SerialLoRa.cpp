#include "VirtualSerial.h"

// AT Command 最後結尾都需要加入Enter或0x0D 0x0A
// 所有指令必須等到模組回覆+OK後才能繼續執行下一個指令
char test_comm[] = "AT"; // 回覆 +OK
char reset_comm[] = "AT+RESET"; // 軟體  RESET，回覆 +RESET/+READY
char setMode_comm[] = "AT+MODE="; //  設定無線工作模式 : 設定為智慧接收省電模式 AT+MODE=2,3000,3000，回覆 +OK
char setBaud_comm[] = "AT+IPR="; // 設定UART介面Baud Rate : AT+IPR=115200，回覆 +IPR=<rate> 
char setRF_comm[] = "AT+PARAMETER="; // 設定RF無線參數 AT+PARAMETER=<Spreading Factor>, <Bandwidth>,<Coding Rate>, <Programmed Preamble> : AT+PARAMETER=7,9,4,12，回覆 +OK
char setHz_comm[] = "AT+BAND="; // 設定無線頻段中心頻率 <parameter>為無線頻段中心頻率,單位為Hz : AT+BAND=868500000，回覆 +OK
char setAddrID_comm[] = "AT+ADDRESS="; // 設定LoRa模組  ADDRESS ID <Address>=0~65535 : AT+ADDRESS=120，回覆 +OK
char setNetID_comm[] = "AT+NETWORKID="; // 設定網路ID <Network ID>3~15 : AT+NETWORKID=6，回覆 +OK
char setPINCode_comm[] = "AT+CPIN="; // 設定網域密碼(為8個字元長度密碼)  : AT+CPIN=EEDCAA90，回覆 +OK
char setRFPower_comm[] = "AT+CRFOP="; // 設定LoRa 模組發射功率 : AT+CRFOP=<power> : AT+CRFOP=10 (輸出功率必須為  AT+CRFOP=14  或以下才能符合CE認證需求)，回覆 +OK
char sendMsg_comm[] = "AT+SEND="; // 使用Command Mode傳送資料到指定Address : AT+SEND=50,5,HELLO，回覆 +OK
char getMsg_comm[] = "+RCV"; // 主動提示收到資料 : +RCV=50,5,HELLO,-99,40  (看我要不要加 FIFO 50 組)
char getUID_comm[] = "AT+UID?"; // 讀取模組序號  12BYTES : +UID=104737333437353600170029
char getVersion_comm[]  = "AT+VER?"; // 讀取軟體版本資訊 : +VER=RYLRxx8_Vx.x.x 
char setDefVal_comm[] = "AT+FACTORY"; // 設定模組成為出廠模式 : +FACTORY

#define LORA_DEBUG     (false)
#define INBUFF_SIZE    (8912)
#define LORA_TIMEOUT   (1000)

SerialLoRa::SerialLoRa() {
    _serial = NULL;
    _state = LORAIDLE;
    _result = -1;
    idx = fhead = ftail = 0;
    peek_stored = false;
    peek_val = -1;
    
    for (int i=0; i<FIFO_SIZE; i++) {
        for (int j=0; j<32; j++) {
            _FIFO[i][j] = 0;
        }
    }
}

SerialLoRa::SerialLoRa(SerialInterface &serialport) {
    _serial = &serialport;
    _state = LORAIDLE;
    _result = -1;
    idx = fhead = ftail = 0;
    peek_stored = false;
    peek_val = -1;
    
    for (int i=0; i<FIFO_SIZE; i++) {
        for (int j=0; j<32; j++) {
            _FIFO[i][j] = 0;
        }
    }
}

char* SerialLoRa::combineCommand(char* command, char* para1, char* para2, char* para3, char* para4, char* para5) {
    char* tmp = (char*)calloc(256); 
    
    sprintf(tmp, "%s", command);
    if (para1 != NULL) { strcat(tmp, para1); }
    if (para2 != NULL) { strcat(tmp, ","); strcat(tmp, para2); }
    if (para3 != NULL) { strcat(tmp, ","); strcat(tmp, para3); }
    if (para4 != NULL) { strcat(tmp, ","); strcat(tmp, para4); }
    if (para5 != NULL) { strcat(tmp, ","); strcat(tmp, para5); }
    
    return tmp;
}

void SerialLoRa::sendCommand(int command, int para1, int para2, int para3, int para4, int para5) {
    char* AT_command;
    char buf[5][16];
    
    if (para1 != -1) sprintf(&buf[0][0], "%d", para1);
    if (para2 != -1) sprintf(&buf[1][0], "%d", para2);
    if (para3 != -1) sprintf(&buf[2][0], "%d", para3);
    if (para4 != -1) sprintf(&buf[3][0], "%d", para4);
    if (para5 != -1) sprintf(&buf[4][0], "%d", para5);
    
    switch (command) {
        case LORA_SET_BAUD:
            AT_command = combineCommand(setBaud_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_TEST:
            AT_command = combineCommand(test_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_RESET:
            AT_command = combineCommand(reset_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_SET_MODE:
            AT_command = combineCommand(setMode_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_SET_RF:
            AT_command = combineCommand(setRF_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_SET_HZ:
            AT_command = combineCommand(setHz_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_SET_ADDRESSID:
            AT_command = combineCommand(setAddrID_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_SET_NETID:
            AT_command = combineCommand(setNetID_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_SET_RF_POWER:
            AT_command = combineCommand(setRFPower_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        // case LORA_SET_MSG:
        // break;
        // case LORA_GET_MSG:
        // break;
        case LORA_GET_UID:
            AT_command = combineCommand(getUID_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_GET_VER:
            AT_command = combineCommand(getVersion_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
        case LORA_GET_DEF_VAL:
            AT_command = combineCommand(setDefVal_comm, (para1 != -1) ? &buf[0][0] : NULL, (para2 != -1) ? &buf[1][0] : NULL, (para3 != -1) ? &buf[2][0] : NULL, (para4 != -1) ? &buf[3][0] : NULL, (para5 != -1) ? &buf[4][0] : NULL);
        break;
    }
    
    strcat(AT_command, "\r\n");
    
    if (LORA_DEBUG) printf("CMD: %s", AT_command);
    
    _serial->print(AT_command);
    free(AT_command);
}

int SerialLoRa::_pushDataToFifo(void) {
    
    if (fhead > (FIFO_SIZE - 1)) fhead = 0;
    
    memcpy(&_FIFO[fhead++][0], _recvBuf, idx);

}

int SerialLoRa::_popDataToFifo(char* data) {
    
    int ret = 0; // 沒有資料
    
    if (_FIFO[ftail][0] > 0) { // FIFO 中有資料
        memcpy(data, &_FIFO[ftail][0], 32);
        memset(&_FIFO[ftail][0], 0, 32);
        ftail++;
        
        if (ftail > (FIFO_SIZE - 1)) ftail = 0;
        ret = 1; // 成功拿到資料
    }
    
    return ret;
}

int SerialLoRa::_haveDataInFifo() {
    return (fhead - ftail);
}


// 回傳值
int SerialLoRa::update(void) {
    if (_state == LORAIDLE) {
        _state = LORAREADY1;
        idx = 0;
        _t = millis();
    } else if (_state == LORAREADY) {
        while (_serial->available() > 0 && _serial->peek() != '+') _serial->read();
        if (_serial->peek() == '+') {
            _recvBuf[idx++] = _serial->read(); // '+' 放進內部 buf 以續後面判斷
            _state = LORARECV1;
            _t = millis();
        } else if ((millis() - _t) > LORA_TIMEOUT) {
            _state = LORAIDLE;
            _result = -1;
            Serial.println("read '+' Timeout!");
        }
    } else if (_state == LORARECV1) {
        if (_serial->available() > 0) {
            if ((_recvBuf[idx++] = _serial->read()) == '\r') // 一直放進內部 buf，假如讀到 \r 那下一個應該是 \n
                _state = LORARECV2;
            _t = millis();
        } else if ((millis() - _t) > LORA_TIMEOUT) {
            _state = LORAIDLE;
            _result = -1;
            Serial.println("read '\\r' Timeout!");
        }
    } else if (_state == LORARECV2) {
        if (_serial->available() > 0) {
            if ((_recvBuf[idx++] = _serial->read()) == '\n') {
                _recvBuf[idx] = '\0'; // 結束
                
                // 開始處理數據，看 _recvBuf 是屬於哪一種回應
                if (strcmp(_recvBuf, "+OK\r\n") == 0) {
                    _result = LORA_OK;
                } else if (strcmp(_recvBuf, "+RESET\r\n") == 0) {
                    _result = LORA_RESET;
                } else if (strcmp(_recvBuf, "+READY\r\n") == 0) {
                    _result = LORA_READY;
                } else if (strcmp(_recvBuf, "+FACTORY\r\n") == 0) {
                    _result = LORA_FACTORY;
                } else if (strstr(_recvBuf, "+ERR") != 0) {
                    _result = LORA_ERR;
                } else if (strstr(_recvBuf, "+RCV") != 0) {
                    _result = LORA_RCV;
                    _pushDataToFifo();
                } else
                    _result = LORA_OTHER; // 自己檢查 _recvBuf 裡面放了什麼回應
            }
            _state = LORAIDLE;
        } else if ((millis() - _t) > LORA_TIMEOUT) {
            _state = LORAIDLE;
            _result = -1;
            Serial.println("read '\\n' Timeout!");
        }
    }
    
    return _result;
}

// 送命令後的 ACK，所有指令必須等到模組回覆+OK後才能繼續執行下一個指令
int SerialLoRa::ack(int ackCmd) {
    unsigned long t = 0L;
    bool success = false;
    
    t = millis();
    while ((millis() - t) < LORA_TIMEOUT) {
        if (update() == ackCmd) { // 等待回覆 +OK or ackCmd
            success = true;
            break;
        }
        __yield();
    }
    
    Serial.println("read ack Timeout!");
    
    return success;
}

extern void __yield();

// 設定 LoRa 參數 API (Blocking)
int SerialLoRa::begin(int baudrate, int config)
{
    int _baud[9] = {300, 1200, 4800, 9600, 19200, 28800, 38400, 57600, 115200};
    int i, diff1, diff2, ret = 0; 
    if (_serial == NULL) return -2;

    for (i = 0; i < 9; i++) {
        if (baudrate >= _baud[i]) continue; 
        diff1 = abs(baudrate - _baud[i - 1]); 
        diff2 = abs(_baud[i] - baudrate); 
        if (diff2 > diff1) baudrate = _baud[i - 1]; 
        else if (diff1 > diff2) baudrate = _baud[i]; 
        break; 
    }
    if (i == 9) baudrate = 115200;
    
    // _serial->begin(baudrate, config); // User 必須在外面自己先設定

    // Send AT command
    sendCommand(LORA_SET_BAUD, baudrate, -1, -1, -1, -1);

    if (ack(LORA_OK)) return 0;

    return -1; // Timeout
}

int SerialLoRa::setWorkingMode(int mode1, int mode2, int mode3)
{
    if (mode1 < 0 || mode1 > 2) return -2;
    
    // Send AT command
    if (mode1 == 0 || mode1 == 1)
        sendCommand(LORA_SET_MODE, mode1, -1, -1, -1, -1);
    else {
        if (mode2 < 30 || mode2 > 60000) return -3;
        if (mode3 < 30 || mode3 > 60000) return -4;
        sendCommand(LORA_SET_MODE, mode1, mode2, mode3, -1, -1);
    }    
    
    if (ack(LORA_OK)) return 0;

    return -1; // Timeout
}

int SerialLoRa::setRF(int para1, int para2, int para3, int para4)
{
    if (para1 < 5 || para1 > 11) return -2;
    if (para2 < 7 || para2 > 9) return -3;
    if (para3 < 1 || para3 > 4) return -4;
    if (_rolaData.LoRaNetID == 18) {
        if (para4 < 4 || para4 > 24) return -5;
    } else {
        if (para4 != 12) return -6;
    }
    
    sendCommand(LORA_SET_RF, para1, para2, para3, para4, -1);
    
    if (ack(LORA_OK)) return 0;
    
    return -1; // Timeout
}

int SerialLoRa::setHz(int Hz)
{
    // DO TO : 記憶在Flash.(Only support after F/W version 1.2.0) AT+BAND=868500000,M
    sendCommand(LORA_SET_HZ, Hz, -1, -1, -1, -1);
    
    if (ack(LORA_OK)) return 0;
    
    return -1; // Timeout
}

int SerialLoRa::setAddressID(int addrID) {
    
    if (addrID < 0 || addrID > 65535) return -2;
    
    sendCommand(LORA_SET_ADDRESSID, addrID, -1, -1, -1, -1);
    
    if (ack(LORA_OK)) return 0;
    
    return -1; // Timeout
}
    
int SerialLoRa::setNetworkID(int netID) {
    
    if (netID != 18 && (netID < 3 || netID > 15)) return -2;
    
    sendCommand(LORA_SET_NETID, netID, -1, -1, -1, -1);
    
    if (ack(LORA_OK)) return 0;
    
    return -1; // Timeout
}

int SerialLoRa::setPassword(char *password) {
    
    char* AT_command = NULL;
    if (password == NULL) return -2;

    // 8個字元長度密碼，由00000001到FFFFFFFF，RESET後密碼會消失
    AT_command = combineCommand(setPINCode_comm, password, NULL, NULL, NULL, NULL);
    strcat(AT_command, "\r\n");
    _serial->print(AT_command);
    free(AT_command);
    
    if (ack(LORA_OK)) return 0;
    
    return -1; // Timeout
}
    
int SerialLoRa::setRFPower(int RFpower) {
    
    if (RFpower < 0 || RFpower > 22) return -2;
    
    sendCommand(LORA_SET_RF_POWER, RFpower, -1, -1, -1, -1);
    
    if (ack(LORA_OK)) return 0;
    
    return -1; // Timeout
}

int SerialLoRa::sendMsg(int addrID, char* data, int dataLen) {
    
    char* AT_command = NULL;
    char _dataLen[16] = {'\0'};
    char _addrID[16] = {'\0'};
    int ATC_LEN = 0;
    
    if (addrID < 0 || addrID > 65535 || dataLen < 0 || dataLen > 240 || data == NULL) return -2;
    
    sprintf(_addrID, "%d", addrID);
    sprintf(_dataLen, "%d", dataLen);
    AT_command = combineCommand(sendMsg_comm, _addrID, _dataLen, NULL, NULL, NULL, NULL);
    ATC_LEN = strlen(AT_command);
    memcpy(AT_command + ATC_LEN, data, dataLen);
    AT_command[ATC_LEN + dataLen] = '\r';
    AT_command[ATC_LEN + dataLen + 1] = '\n';
    _serial->print(AT_command);
    free(AT_command);
    
    // if (ack(LORA_OK)) return 0; // 由 available 去收掉這個 +OK\r\n
    
    return dataLen;
}

int SerialLoRa::setParameters(LoRaData data, SerialInterface &serialport) {
    int error = 1;
    
    if (&serialport == NULL) return -99;
    
    _serial = &serialport;
    _rolaData = data;
    
    if (begin(data.LoRaBaud, SERIAL_8N1) < 0) {
        Serial.println("Error: set LoRa baudrate fail!");
        return -error;
    }
    error++;
    
    if (setWorkingMode(data.LoRaMode1, data.LoRaMode2, data.LoRaMode3) < 0) {
        Serial.println("Error: set LoRa mode fail!");
        return -error;
    }
    error++;
    
    if (setRF(data.LoRaRF1, data.LoRaRF2, data.LoRaRF3, data.LoRaRF4) < 0) {
        Serial.println("Error: set LoRa RF fail!");
        return -error;
    }
    error++;
    
    if (setHz(data.LoRaHz) < 0) {
        Serial.println("Error: set LoRa Hz fail!");
        return -error;
    }
    error++;
    
    if (setAddressID(data.LoRaAddrID) < 0) {
        Serial.println("Error: set LoRa address ID fail!");
        return -error;
    }
    error++;
    
    if (setNetworkID(data.LoRaNetID) < 0) {
        Serial.println("Error: set LoRa network ID fail!");
        return -error;
    }
    error++;
    
    if (setPassword(data.LoRaPINCode) < 0) {
        Serial.println("Error: set LoRa password fail!");
        return -error;
    }
    error++;
    
    if (setRFPower(data.LoRaRFPower) < 0) {
        Serial.println("Error: set LoRa RF power fail!");
        return -error;
    }
    
    return 0;
}



// 通訊 API (non-Blocking)
// 平常沒事的 routine，主要是處理 +RCV 的內容放進 FIFO
int SerialLoRa::routine() {
    update();
}

void SerialLoRa::setTargetAddress(int addressID) {
    _targetAddressID = addressID;
}

size_t SerialLoRa::write(uint8_t value)
{
    if (_slave == NULL) return 1;

    sendMsg(_targetAddressID, &value, 1);
    
    routine();
    
    return 1;
}

size_t SerialLoRa::write(const uint8_t* buf, size_t size)
{
    if (_slave == NULL || size <= 0) return 1;

    sendMsg(_targetAddressID, buf, size);
    
    routine();
    
    return size;
}

int SerialLoRa::read()
{
    int c;
    if (_slave == NULL) return -1;

    routine();
    
    // 都改成從 FIFO 讀取
    if(peek_stored == true)
	{
		peek_stored = false;
		return peek_val;
	}
    else
    {
        // Send AT command (未完成)
        if ((c = _serial->print(AT_command)) < 0)
            return -1;
        
        return (c == 0xFFFF) ? -1 : c;
    }
}

int SerialLoRa::available()
{
    if (_slave == NULL) return 0;

    routine();
    
    return _haveDataInFifo();
}

int SerialLoRa::peek()
{
    if (_slave == NULL) return -1;

    if (peek_stored == true)
		return peek_val;
    else {
        // Send AT command (未完成)
        if ((peek_val = _serial->print(AT_command)) == 0xFFFF) {
            peek_val = -1;
            return -1;
        }
        peek_stored = true;
        return peek_val;
    }
}

void SerialLoRa::flush(int w) {
    if (_slave == NULL) return;
    
    
    if (w == 0)
        _serial->print(AT_command);
    else
        _serial->print(AT_command);
}





BarcodeScanner::BarcodeScanner() {
    serialport = NULL;
    _Mode = NOSETMODE;
    _state = INIT;
    _tst = 0;
}

BarcodeScanner::BarcodeScanner(BarcodeDevice dev, SerialInterface &sp) {
    if (dev == HONEYWELL_N4680 || dev == HONEYWELL_HF521) {
        serialport = &sp;
    } else
        serialport = NULL;
    _Mode = ETHERCAT;
    _state = INIT;
    _tst = 0;
}



bool BarcodeScanner::begin(BarcodeDevice dev, SerialInterface &sp, unsigned long triggerInterval, unsigned int new_baud) {

    if (dev == HONEYWELL_N4680 || dev == HONEYWELL_HF521) {
        // if (new_baud == DEFAULT_BAUD) sp.begin(115200);
        // else if (new_baud > 50) sp.begin(new_baud);
        
        serialport = &sp;
        _Mode = ETHERCAT;
        
        _buf = (char*)malloc(sizeof(char) * INBUFF_SIZE);
        if (_buf == NULL) {
            Serial.println("ERROR: alloc internal buffer fail");
            return false;
        }
        
        // Step 1 : set Serial Read TimeOut
        int i = 0;
        sp.print(set_SerialReadTimeOut);
        for (i = 0; i < INBUFF_SIZE; i++) _buf[i] = '\0';
        int c = sp.readBytesUntil(0x06, _buf, INBUFF_SIZE); // 0x06 is ACK
        if (c > 0) {
            _buf[c] = '!'; sp.read(); // read again. To clear '!' in fifo
            _buf[c+1] = '\0';
        }
        for (i = 0; _buf[i] == '\0'; i++); // skip '\0' in _buf
        if (strcmp(&_buf[i], &set_SerialReadTimeOut[3]) != 0) {
            printf("_buf=%s\n", &_buf[i]);
            printf("command=%s\n", &set_SerialReadTimeOut[3]);
            Serial.print("ERROR: ");
            Serial.println(&_buf[i]);
            return false;
        }
        // Step 2 : add suffix char, here is '\r'
        sp.print(add_suffix_CR);
        for (int i = 0; i < INBUFF_SIZE; i++) _buf[i] = '\0';
        c = sp.readBytesUntil(0x06, _buf, INBUFF_SIZE); // 0x06 is ACK
        if (c > 0) {
            _buf[c] = '!'; sp.read(); // read again. To clear '!' in fifo
            _buf[c+1] = '\0';
        }
        if (strcmp(_buf, &add_suffix_CR[3]) != 0) {
            printf("_buf=%s\n", &_buf[1]);
            printf("command=%s\n", &add_suffix_CR[3]);
            Serial.print("ERROR: ");
            Serial.println(_buf);
            return false;
        }
        
        _state = IDLE;
        _triIv = triggerInterval;
        _trt = _tst = millis();
    } else
        return false;
    
    return true;
}

bool BarcodeScanner::begin(unsigned long triggerInterval) {
    if (serialport == NULL) return false;
    
    _buf = (char*)malloc(sizeof(char) * INBUFF_SIZE);
    if (_buf == NULL) {
        Serial.println("ERROR: alloc internal buffer fail");
        return false;
    }
    
    // Step 1 : set Serial Read TimeOut
    int i = 0;
    serialport->print(set_SerialReadTimeOut);
    for (i = 0; i < INBUFF_SIZE; i++) _buf[i] = '\0';
    int c = serialport->readBytesUntil(0x06, _buf, INBUFF_SIZE); // 0x06 is ACK
    if (c > 0) {
        _buf[c] = '!'; serialport->read(); // read again. To clear '!' in fifo
        _buf[c+1] = '\0';
    }
    for (i = 0; _buf[i] == '\0'; i++); // skip '\0' in _buf
    if (strcmp(&_buf[i], &set_SerialReadTimeOut[3]) != 0) {
        printf("_buf=%s\n", &_buf[i]);
        printf("command=%s\n", &set_SerialReadTimeOut[3]);
        Serial.print("ERROR: ");
        Serial.println(&_buf[i]);
        return false;
    }
    // Step 2 : add suffix char, here is '\r'
    serialport->print(add_suffix_CR);
    for (int i = 0; i < INBUFF_SIZE; i++) _buf[i] = '\0';
    c = serialport->readBytesUntil(0x06, _buf, INBUFF_SIZE); // 0x06 is ACK
    if (c > 0) {
        _buf[c] = '!'; serialport->read(); // read again. To clear '!' in fifo
        _buf[c+1] = '\0';
    }
    if (strcmp(_buf, &add_suffix_CR[3]) != 0) {
        printf("_buf=%s\n", &_buf[1]);
        printf("command=%s\n", &add_suffix_CR[3]);
        Serial.print("ERROR: ");
        Serial.println(_buf);
        return false;
    }
    
    _state = IDLE;
    _triIv = triggerInterval;
    _trt = _tst = millis();
}

// void BarcodeScanner::terminal() {
//     if (_Mode == ETHERCAT) {
//         MyVirtualSerial* eva = (MyVirtualSerial*)serialport;
//         eva->print(terminal_comm);
//     } else if (_Mode == HWSERIAL) {
//         HardwareSerial* hs = (HardwareSerial*)serialport;
//         hs->print(terminal_comm);
//     }
// }

void BarcodeScanner::sendCommand(char* command) {
    serialport->print(command);
}

void BarcodeScanner::triggerScan() {
    if (_state == INIT || serialport == NULL) return;
    
    if ((millis() - _trt) < _triIv) return;
    _trt = millis();
    
    if (_state == IDLE) {
        
        sendCommand(trigger_comm);
        
        _state = SCANNING;
    } else if (_state == SCANNING && (millis() - _tst) > SCANTIMEOUT) {
        _tst = millis();
        _state = IDLE;
    }
}

void BarcodeScanner::stopScan() {
    if (_state == IDLE || _state == INIT || serialport == NULL) return;
    
    _state = IDLE;
    
    sendCommand(untrigger_comm);
}

void BarcodeScanner::command(char* command) {
    if (command == NULL || _state == INIT || serialport == NULL) return;
    
    sendCommand(command);
}

void BarcodeScanner::enableBeep() {
    if (serialport == NULL || _state == INIT) return;
    
    sendCommand(beeperOn);
}

void BarcodeScanner::disableBeep() {
    if (serialport == NULL || _state == INIT) return;
    
    sendCommand(beeperOff);
}

int BarcodeScanner::available() {
    int c = 0;
    
    triggerScan();

    if (_state == SCANNING) {
        c = serialport->available(); // call update() and yield() in SerialEVA class.
    }
    
    return c;
}

int BarcodeScanner::read() {
    int c = '\0';
    
    if (available() > 0) {
        c = serialport->read(); // call update() and yield() in SerialEVA class.
        // printf("%c", c);
        if (c == '\r') { _state = IDLE; printf("\n"); }
    }
    
    return c;
}

int BarcodeScanner::readLine(char* buf, int size) {
    int c = 0;
    if (buf == NULL || size <= 0) return 0;

    if (available() > 0) {
        for (int i = 0; i < size; i++) buf[i] = '\0';
        
        while (serialport->peek() == 0) serialport->read();
        c = serialport->readBytesUntil('\r', buf, size); // call update() and yield() in SerialEVA class.

        _state = IDLE;
    }
    
    return c;
}

char* BarcodeScanner::readLine() {
    int c = 0;
    
    if (available() > 0) {
        for (int i = 0; i < INBUFF_SIZE; i++) _buf[i] = '\0';

        while (serialport->peek() == 0) serialport->read();
        c = serialport->readBytesUntil('\r', _buf, INBUFF_SIZE); // call update() and yield() in SerialEVA class.

        _state = IDLE;
    }
    
    if (_state == IDLE) return _buf;
    
    return NULL;
}



// ForceSensor
#define FBMOD    (LITTLE_END)

_ForceSensor::_ForceSensor(unsigned long timeout) {
    serialport = NULL;
    _FMode = F_NOSETMODE;
    _state = FSIDLE;
    _timeout = timeout;
    _Mx = -1;
    _My = -1;
    _Fz = -1;
    _F0 = -1;
    _F1 = -1;
    _F2 = -1;
    _id = -1;
    _baudIdx = -1;
    _newid = -1;
    _footLength = 0.0;
    _footWidth = 0.0;
    _begined = false;
    pre_command = NO_FCMD;
    _Mode = SLAVE_MODE;
    _refreshRate = 0x14; // refresh rate default: 20ms
}

_ForceSensor::_ForceSensor(ForceSensorDevice dev, SerialInterface &sp, unsigned long timeout) {
    if (dev == DM320) {
        serialport = &sp;
    } else
        serialport = NULL;
    
    _FMode = F_ETHERCAT;
    _state = FSIDLE;
    _timeout = timeout;
    _Mx = -1;
    _My = -1;
    _Fz = -1;
    _F0 = -1;
    _F1 = -1;
    _F2 = -1;
    _id = -1;
    _baudIdx = -1;
    _newid = -1;
    _footLength = 0.0;
    _footWidth = 0.0;
    _begined = false;
    pre_command = NO_FCMD;
    _Mode = SLAVE_MODE;
    _refreshRate = 0x14; // refresh rate default: 20ms
}

static bool isFSCheckSumOK(unsigned char* buf, int len) {
    unsigned char chksum = 0;
    for (int i=0; i<(len-1); i++)
        chksum += buf[i];
    if (chksum == buf[len-1]) return true;
    return false;
}

static unsigned char getFSCheckSum(unsigned char* buf, int len) {
    unsigned char chksum = 0;
    for (int i=4; i<len; i++)
        chksum += buf[i];
    return chksum;
}

float bytesToFloat(char b0, char b1, char b2, char b3) {
    float2byte data;
    
    if (FBMOD == LITTLE_END) {
        data.bytes[0] = b0;
        data.bytes[1] = b1;
        data.bytes[2] = b2;
        data.bytes[3] = b3;
    } else {
        data.bytes[0] = b3;
        data.bytes[1] = b2;
        data.bytes[2] = b1;
        data.bytes[3] = b0;
    }
    
    return data.value;
}

void _ForceSensor::_handleData() {
    // _recvBuf[0] 總是 command
    int cmd = _recvBuf[0];
    if (cmd == FCMD_SET_ID) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = true;
            Serial.println("Set new ID OK.");
        } else {
            _result = false;
            Serial.println("Error: Set new ID NG.");
        }
    } else if (cmd == FCMD_SET_BAUD) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = true;
            Serial.println("Set new baudrate OK.");
        } else {
            _result = false;
            Serial.println("Error: Set new baudrate NG.");
        }
    } else if (cmd == FCMD_RESET) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = true;
            Serial.println("Reset OK.");
        } else {
            _result = false;
            Serial.println("Error: Reset NG.");
        }
    } else if (cmd == FCMD_SET_FOOTSIZE) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = true;
            Serial.println("Set the foot size OK.");
        } else {
            _result = false;
            Serial.println("Error: Set the foot size NG.");
        }
    } else if (cmd == FCMD_GET_FOOTSIZE) {
        _footLength = bytesToFloat(_recvBuf[1], _recvBuf[2], _recvBuf[3], _recvBuf[4]);
        _footWidth  = bytesToFloat(_recvBuf[5], _recvBuf[6], _recvBuf[7], _recvBuf[8]);
        _result = true;
    } else if (cmd == FCMD_GET_FORCEDATA) {
        _Fz = bytesToFloat(_recvBuf[1], _recvBuf[2], _recvBuf[3], _recvBuf[4]);
        _Mx = bytesToFloat(_recvBuf[5], _recvBuf[6], _recvBuf[7], _recvBuf[8]);
        _My = bytesToFloat(_recvBuf[9], _recvBuf[10], _recvBuf[11], _recvBuf[12]);
        _result = true;
    } else if (cmd == FCMD_GET_RAWDATA) {
        _F0 = bytesToFloat(_recvBuf[1], _recvBuf[2], _recvBuf[3], _recvBuf[4]);
        _F1 = bytesToFloat(_recvBuf[5], _recvBuf[6], _recvBuf[7], _recvBuf[8]);
        _F2 = bytesToFloat(_recvBuf[9], _recvBuf[10], _recvBuf[11], _recvBuf[12]);
        _result = true;
    }
}

bool _ForceSensor::_si_update() {
    
    if (_state == FSIDLE) {
        serialport->write(_commandBuf, _commLen);
        _state = FSREADY1;
        _t = millis();
    } else if (_state == FSREADY1) {
        while (serialport->available() > 0 && serialport->peek() != _commandBuf[0]) serialport->read();
        if (serialport->peek() == _commandBuf[0]) {
            _state = FSREADY2;
            _t = millis();
        } else if ((millis() - _t) > _timeout) {
            _state = FSIDLE;
            _result = false;
            Serial.println("state FSREADY1 Timeout!");
        } 
    } else if (_state == FSREADY2) {
        if (serialport->available() >= 3) { // 確保一定是收到真 ACK 3 bytes : 0x55 0xAA (ID)
            if (serialport->read() == _commandBuf[0] && serialport->read() == _commandBuf[1] && serialport->read() == _id) {
                _state = FSRECV1;
                _t = millis();
            } else {
                _state = FSIDLE;
                _result = false;
            }
        } else if ((millis() - _t) > _timeout) {
            _state = FSIDLE;
            _result = false;
            Serial.println("state FSREADY2 Timeout!");
        } 
    } else if (_state == FSRECV1) {
        if (serialport->available() > 0) {
            _recvlen = serialport->read();
            _t = millis();
            _state = FSRECV2;
        } else if ((millis() - _t) > _timeout) {
            _state = FSIDLE;
            _result = false;
            Serial.println("state FSRECV1 Timeout!");
        } 
    } else if (_state == FSRECV2) {
        if (serialport->available() >= _recvlen) {
            for (int i=0; i<_recvlen; i++) _recvBuf[i] = serialport->read(); // 先全部放進內部 buf
            // 開始處理數據，_recvBuf[0] 是 command，可以跳過
            if (!isFSCheckSumOK(_recvBuf, _recvlen)) {
                _state = FSIDLE;
                _result = false;
                Serial.println("Check sum error\n");
            } else {
                // Check Sum 正確後
                _handleData();
            }
            
            _state = FSIDLE;
        } else if ((millis() - _t) > _timeout) {
            _state = FSIDLE;
            _result = false;
            Serial.println("state FSRECV2 Timeout!");
        }
    }
    
    if (_state != FSIDLE)
        return true; // Working...

    return false;
}

bool _ForceSensor::update() { // 最後加入 pre_command != NO_FCMD 讓 update() 可以在外部無限呼叫(在沒有命令的情況下)
    bool ret = false;
    
    if (pre_command != NO_FCMD) {
        ret = _si_update();
    } else
        return false;
    
    if(!ret) {
        pre_command = NO_FCMD;
        return false; // Complete
    }
    
    return true; // Working...
}

bool _ForceSensor::begin(ForceSensorDevice dev, SerialInterface &sp, int id, unsigned int new_baud) {

    if (dev == DM320) {
        // if (new_baud == DEFAULT_BAUD) sp.begin(115200);
        // else if (new_baud > 50) sp.begin(new_baud);
        
        serialport = &sp;
        
        _FMode = F_HWSERIAL;
        _commandBuf = (unsigned char*)malloc(256);
        _recvBuf = (unsigned char*)malloc(256);
        if (_commandBuf == NULL || _recvBuf == NULL) return false;
        
        if (id != -1)
            _id = id;
        _state = FSIDLE;
        
        unsigned long t = millis();
        while ((millis() - t) < 500) sp.available(); // call update() for 500ms
        while (sp.available()) sp.read(); // clear ALL recv date
    
        _begined = true;
    } else
        return false;
    
    return true;
}

bool _ForceSensor::begin(int id) {
    if (_FMode == F_NOSETMODE) return false;
    
    if (serialport != NULL) {
        _commandBuf = (unsigned char*)malloc(256);
        _recvBuf = (unsigned char*)malloc(256);
        if (_commandBuf == NULL || _recvBuf == NULL) return false;
        
        if (id != -1)
            _id = id;
        _state = FSIDLE;
        
        unsigned long t = millis();
        while ((millis() - t) < 500) serialport->available(); // call update() for 500ms
        while (serialport->available()) serialport->read(); // clear ALL recv date
    
        _begined = true;
    } else
        return false;

    return true;
}

bool _ForceSensor::reset(void) {
    if (!_begined) return 0;
    while (pre_command != NO_FCMD) update(); // 等之前的命令做完或者 timeout
    
    if (_Mode == MASTER_MODE) {
        _state = FSREADY1;
        _t = millis();
    }
    
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = _id;
    _commandBuf[3] = 2;
    _commandBuf[4] = FCMD_RESET;
    _commandBuf[5] = FCMD_RESET; // 因為內容只有 1 byte，所以 checksum  剛好等於內容自己，不用計算
    _commLen = 6;
    pre_command = FCMD_RESET;
    _result = false;
    while (update());
    
    return _result;
}

bool _ForceSensor::setNewID(int newid) { // 總是 block funciton
    if (!_begined) return false;
    if (newid >= 0 && newid != _id) _newid = newid; else return false;
    while (pre_command != NO_FCMD) update(); // 等之前的命令做完或者 timeout
    
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = _id;
    _commandBuf[3] = 3;
    _commandBuf[4] = FCMD_SET_ID;
    _commandBuf[5] = _newid;
    _commandBuf[6] = getFSCheckSum(_commandBuf, 6);
    _commLen = 7;
    
    pre_command = FCMD_SET_ID;
    _result = false;
    while (update());
    if (_result)
        _id = _newid;
    
    return _result;
}

bool _ForceSensor::setFootSize(unsigned long length, unsigned long width) { // 總是 block funciton
    if (!_begined) return false;
    while (pre_command != NO_FCMD) update(); // 等之前的命令做完或者 timeout
    
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = _id;
    _commandBuf[3] = 10;
    _commandBuf[4] = FCMD_SET_FOOTSIZE;
    _commandBuf[5] = *((unsigned char*)(&length) + 0);
    _commandBuf[6] = *((unsigned char*)(&length) + 1);
    _commandBuf[7] = *((unsigned char*)(&length) + 2);
    _commandBuf[8] = *((unsigned char*)(&length) + 3);
    _commandBuf[9] = *((unsigned char*)(&width) + 0);
    _commandBuf[10] = *((unsigned char*)(&width) + 1);
    _commandBuf[11] = *((unsigned char*)(&width) + 2);
    _commandBuf[12] = *((unsigned char*)(&width) + 3);
    _commandBuf[13] = getFSCheckSum(_commandBuf, 13);
    _commLen = 14;
    
    pre_command = FCMD_SET_FOOTSIZE;
    _result = false;
    while (update());
    
    return _result;
}

bool _ForceSensor::getFootSize(float* length, float* width) { // 總是 block funciton
    if (!_begined) return false;
    while (pre_command != NO_FCMD) update(); // 等之前的命令做完或者 timeout
    
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = _id;
    _commandBuf[3] = 2;
    _commandBuf[4] = FCMD_GET_FOOTSIZE;
    _commandBuf[5] = FCMD_GET_FOOTSIZE;
    _commLen = 6;
    
    pre_command = FCMD_GET_FOOTSIZE;
    _result = false;
    while (update());
    
    if (_result) {
        *length = _footLength;
        *width = _footWidth;
    }
    
    return _result;
}
/*
void _ForceSensor::autoCalibration(void) {
    while (update(FCMD_REFIX));
}

void _ForceSensor::setMasterMode(void) {
    _Mode = MASTER_MODE;
    while (update(FCMD_SET_MODE));
    while (update(FCMD_RESET));
}

void _ForceSensor::setRefreshRate(unsigned char t) {
    if (t < 10 || t > 100) {
        Serial.println("refresh rate is >= 10ms and <= 100ms");
        printf("refresh rate is >= 10ms and <= 100ms");
        return;
    }
    
    _refreshRate = t;
    while (update(FCMD_REF_RATE));
}
*/
static int fsbauds[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}; //, 230400, 460800, 921600, 1000000, 1200000, 1500000, 2000000, 3000000, 6000000};
bool _ForceSensor::setBaudrate(int newBaud) { // 總是 block funciton
    if (!_begined) return false;
    while (pre_command != NO_FCMD) update(); // 等之前的命令做完或者 timeout
    
    int index = -1;
    int bLen = sizeof(fsbauds)/sizeof(int);
    for (int i=0; i<bLen; i++) {
        if (newBaud == fsbauds[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return false;
    if (index != _baudIdx) _baudIdx = index; else return false;
    
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = _id;
    _commandBuf[3] = 3;
    _commandBuf[4] = FCMD_SET_BAUD;
    _commandBuf[5] = _baudIdx;
    _commandBuf[6] = getFSCheckSum(_commandBuf, 6);
    _commLen = 7;
    
    pre_command = FCMD_SET_BAUD;
    _result = false;
    while (update());

    // _commandBuf[0] = 0x55;
    // _commandBuf[1] = 0xAA;
    // _commandBuf[2] = _id;
    // _commandBuf[3] = 2;
    // _commandBuf[4] = FCMD_RESET;
    // _commandBuf[5] = FCMD_RESET;
    // _commLen = 6;
    // pre_command = FCMD_RESET;
    // _result = false;
    // while (update());
    
    return _result;
}

// 需要在背景更新的命令 : FCMD_GET_FORCEDATA 和 FCMD_GET_RAWDATA，兩者會一直交換取得
void _ForceSensor::refresh(bool block) {
    static bool change_get_f012_FzMxMy = false;
    if (pre_command == NO_FCMD) {
        if (change_get_f012_FzMxMy) {
            _commandBuf[0] = 0x55;
            _commandBuf[1] = 0xAA;
            _commandBuf[2] = _id;
            _commandBuf[3] = 2;
            _commandBuf[4] = FCMD_GET_FORCEDATA;
            _commandBuf[5] = FCMD_GET_FORCEDATA;
            _commLen = 6;
            pre_command = FCMD_GET_FORCEDATA;
        } else {
            _commandBuf[0] = 0x55;
            _commandBuf[1] = 0xAA;
            _commandBuf[2] = _id;
            _commandBuf[3] = 2;
            _commandBuf[4] = FCMD_GET_RAWDATA;
            _commandBuf[5] = FCMD_GET_RAWDATA;
            _commLen = 6;
            pre_command = FCMD_GET_RAWDATA;
        }
        change_get_f012_FzMxMy = !change_get_f012_FzMxMy;
        update();
        if (block) while(update());
    } else
        update();
}

double _ForceSensor::getMx(bool block) {
    if (!_begined) return 0;
    
    if (_Mode == MASTER_MODE) {
        _state = FSREADY1;
        _t = millis();
    }
    
    refresh(block);
    
    if (_Mx == -1) return 0.0;
    
    return _Mx;
}

double _ForceSensor::getMy(bool block) {
    if (!_begined) return 0;
    
    if (_Mode == MASTER_MODE) {
        _state = FSREADY1;
        _t = millis();
    }
    
    refresh(block);
    
    if (_My == -1) return 0.0;
    
    return _My;
}

double _ForceSensor::getFz(bool block) {
    if (!_begined) return 0;
    
    if (_Mode == MASTER_MODE) {
        _state = FSREADY1;
        _t = millis();
    }
    
    refresh(block);
    
    if (_Fz == -1) return 0;

    return _Fz;
}

double _ForceSensor::getF0Raw(bool block) {
    if (!_begined) return 0;
    
    if (_Mode == MASTER_MODE) {
        _state = FSREADY1;
        _t = millis();
    }
    
    refresh(block);
    
    if (_F0 == -1) return 0.0;
    
    return _F0;
}

double _ForceSensor::getF1Raw(bool block) {
    if (!_begined) return 0;
    
    if (_Mode == MASTER_MODE) {
        _state = FSREADY1;
        _t = millis();
    }
    
    refresh(block);
    
    if (_F1 == -1) return 0.0;
    
    return _F1;
}

double _ForceSensor::getF2Raw(bool block) {
    if (!_begined) return 0;
    
    if (_Mode == MASTER_MODE) {
        _state = FSREADY1;
        _t = millis();
    }
    
    refresh(block);
    
    if (_F2 == -1) return 0.0;
    
    return _F2;
}

_ForceSensor ForceSensor(1000);



// JoyStick
// 命令 2 : 設置搖桿地址
unsigned char addrCommand[] = {0xAF, 0x0d, 0x00, 0x00, 0x00, 0x01, 0xF5};

// 命令 3 : 復位搖桿 (地址 0 是 reset 全部搖桿)
unsigned char resetCommand[] = {0xAF, 0x15, 0x00, 0x00, 0x00, 0x00, 0xF5};

// 命令 4 : 校正搖桿中心點位置
unsigned char refixCommand[] = {0xAF, 0x09, 0x00, 0x00, 0x00, 0x00, 0xF5};

// 命令 5 : 通信種類選擇
#define CANBUS          (0x00)
#define RS232           (0x01)
#define RS422           (0x02)
#define RS485           (0x03)
#define RS485_MODBUS    (0x04)
unsigned char protocolCommand[] = {0xAF, 0x05, 0x01, 0x00, 0x00, 0x00, 0xF5};

// 命令 6 : 刷新率設置
unsigned char refreshCommand[] = {0xAF, 0x11, 0x00, 0x00, 0x00, 0x14, 0xF5}; // 20ms

// 命令 7 : 設置主從通信模式
#define MASTER_MODE    (0)
#define SLAVE_MODE     (1)
unsigned char modeCommand[] = {0xAF, 0x08, 0x00, 0x00, 0x00, 0x01, 0xF5};

// 命令 8 : 查詢設備訊息
unsigned char infoCommand[] = {0xAF, 0x20, 0x00, 0x00, 0x00, 0x00, 0xF5};

// 命令 9 : 調整 Baudrate
static int jsbauds[] = {9600, 19200, 57600, 115200, 2400, 4800};
unsigned char baudCommand[] = {0xAF, 0x0B, 0x00, 0x00, 0x00, 0x00, 0xF5}; // 9600

// 命令 10 : 取得所有狀態 (只有在 Slave 下這個指令才有效)
unsigned char stateCommand[] = {0xAF, 0x07, 0x00, 0x00, 0x00, 0x00, 0xF5};

_3DJoystick::_3DJoystick(unsigned long timeout) {
    serialport = NULL;
    _Mode = J_NOSETMODE;
    _state = JSIDLE;
    _timeout = timeout;
    _X = -1;
    _Y = -1;
    _Z = -1;
    _addr = -1;
    _btnPressed = _begined = false;
    pre_command = NO_JCMD;
    _JMode = SLAVE_MODE;
    _refreshRate = 0x14; // refresh rate default: 20ms
}

_3DJoystick::_3DJoystick(JoystickDevice dev, SerialInterface &sp, unsigned long timeout) {
    if (dev == SMC30D1) {
        serialport = &sp;
    } else
        serialport = NULL;
    
    _Mode = J_HWSERIAL;
    _state = JSIDLE;
    _timeout = timeout;
    _X = -1;
    _Y = -1;
    _Z = -1;
    _addr = -1;
    _btnPressed = _begined = false;
    pre_command = NO_JCMD;
    _JMode = SLAVE_MODE;
    _refreshRate = 0x14; // refresh rate default: 20ms
}

bool _3DJoystick::_si_update(JoystickCommand command) {
    
    switch (command) {
    case JCMD_SET_ADDR:
        if (_state == JSIDLE) {
            addrCommand[5] = _addr;
            serialport->write(addrCommand, sizeof(addrCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            while (serialport->available() > 0 && serialport->peek() != 0xAA) serialport->read();
            if (serialport->available() >= 3) { // 確保一定是收到真 ACK 3 bytes : 0xAA 0x55 0xAF
                while (serialport->available()) serialport->read(); // 這個指令會回傳一堆 不是標準 3 Bytes ACK，所以收到空為止
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Set Joystick Address Timeout!");
            }                
        }
        break;
    case JCMD_REFIX:
        if (_state == JSIDLE) {
            serialport->write(refixCommand, sizeof(refixCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            while (serialport->available() > 0 && serialport->peek() != 0xAA) serialport->read();
            if (serialport->available() >= 3) {
                if (serialport->read() == 0xAA && serialport->read() == 0x55 && serialport->read() == 0xAF)
                    Serial.println("Set Joystick Calibration OK.");
                else
                    Serial.println("Set Joystick Calibration : NO ACK!");
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Refix Joystick Center Position Timeout!");
            }                
        }
        break;
    case JCMD_RESET:
        if (_state == JSIDLE) {
            resetCommand[5] = (_addr == -1) ? 0 : _addr;
            serialport->write(resetCommand, sizeof(resetCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            while (serialport->available() > 0 && serialport->peek() != 0xAA) serialport->read();
            if (serialport->available() >= 3) {
                if (serialport->read() == 0xAA && serialport->read() == 0x55 && serialport->read() == 0xAF)
                    Serial.println("Reset Joystick OK.");
                else
                    Serial.println("Reset Joystick : NO ACK!");
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Reset Joystick Timeout!");
            }                
        }
        break;
    case JCMD_SEL_PRTCL:
        if (_state == JSIDLE) {
            protocolCommand[2] = 0x01; // RS232
            serialport->write(protocolCommand, sizeof(protocolCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            if (serialport->available() >= 3) {
                // 處理回傳值
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Set Joystick Protocol Timeout!");
            }                
        }
        break;
    case JCMD_REF_RATE:
        if (_state == JSIDLE) {
            refreshCommand[5] = _refreshRate; // default: 20ms
            serialport->write(refreshCommand, sizeof(refreshCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            while (serialport->available() > 0 && serialport->peek() != 0xAA) serialport->read();
            if (serialport->available() >= 3) {
                if (serialport->read() == 0xAA && serialport->read() == 0x55 && serialport->read() == 0xAF)
                    Serial.println("Set Joystick Refresh Rate OK.");
                else
                    Serial.println("Set Joystick Refresh Rate : NO ACK!");
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Set Joystick Refresh Rate Timeout!");
            }                
        }
        break;
    case JCMD_SET_MODE:
        if (_state == JSIDLE) {
            modeCommand[5] = _JMode;
            serialport->write(modeCommand, sizeof(modeCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            while (serialport->available() > 0 && serialport->peek() != 0xAA) serialport->read();
            if (serialport->available() >= 3) {
                if (serialport->read() == 0xAA && serialport->read() == 0x55 && serialport->read() == 0xAF)
                    Serial.println("Set Joystick Mode OK.");
                else
                    Serial.println("Set Joystick Mode : NO ACK!");
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Set Joystick Mode Timeout!");
            }                
        }
        break;
    case JCMD_GET_INFO:
        if (_state == JSIDLE) {
            serialport->write(infoCommand, sizeof(infoCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            while (serialport->available() > 0 && serialport->peek() != 0xFF) serialport->read();
            if (serialport->available() >= 13) {
                unsigned char chksum = 0;
                for (int i=0; i<13; i++)
                    _buf[i] = (unsigned char)serialport->read();

                for (int i=1; i<12; i++)
                    chksum += (unsigned char)_buf[i];
                
                if (chksum == _buf[12]) {
                    for (int i=0; i<13; i++) {
                        Serial.print(_buf[i], HEX);
                        Serial.print(" ");
                    }
                    Serial.println();
                }
                
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Get Joystick Info Timeout!");
            }                
        }
        break;
    case JCMD_SET_BAUD:
        if (_state == JSIDLE) {
            baudCommand[5] = _baudIdx;
            serialport->write(baudCommand, sizeof(baudCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            while (serialport->available() > 0 && serialport->peek() != 0xAA) serialport->read();
            if (serialport->available() >= 3) {
                if (serialport->read() == 0xAA && serialport->read() == 0x55 && serialport->read() == 0xAF)
                    Serial.println("Set Joystick Baudrate OK.");
                else
                    Serial.println("Set Joystick Baudrate : NO ACK!");
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Set Joystick Baudrate Timeout!");
            }
        }
        break;
    case JCMD_GET_STAT:
        if (_state == JSIDLE) {
            stateCommand[5] = _addr;
            serialport->write(stateCommand, sizeof(stateCommand));
            _state = JSRECV;
            _t = millis();
        } else if (_state == JSRECV) {
            while (serialport->available() > 0 && serialport->peek() != 0xFF) serialport->read();
            if (serialport->available() >= 10) {
                unsigned char chksum = 0;
                for (int i=0; i<10; i++)
                    _buf[i] = (unsigned char)serialport->read();

                for (int i=1; i<9; i++)
                    chksum += (unsigned char)_buf[i];
                
                if (_buf[0] == 0xFF) {
                    if (_buf[9] == chksum) {
                        _Y = (((short)_buf[2]) << 8) | _buf[3];
                        _X = (((short)_buf[4]) << 8) | _buf[5];
                        _Z = (((short)_buf[6]) << 8) | _buf[7];
                        _btnPressed = (_buf[8] == 0x20) ? true : false;
                    } else {
                        Serial.println("Check sum ERROR");
                    }
                }
                _state = JSIDLE;
            } else if ((millis() - _t) > _timeout) {
                _state = JSIDLE;
                Serial.println("Get Joystick State Timeout!");
            }                
        }
        break;
    }
    
    if (_state != JSIDLE)
        return true; // Working...

    return false;
}

bool _3DJoystick::update(JoystickCommand command) {
    bool ret;
    
    if (pre_command != NO_JCMD && pre_command != command)
        while (_si_update(pre_command)); // 等待前一個命令做完
    
    pre_command = command;
    ret = _si_update(command);
    
    if(!ret) {
        pre_command = NO_JCMD;
        return false; // Complete
    }
    
    return true; // Working...
}

bool _3DJoystick::begin(JoystickDevice dev, SerialInterface &sp, int addr, unsigned int new_baud) {

    if (dev == SMC30D1) {
        // if (new_baud == DEFAULT_BAUD) sp.begin(115200);
        // else if (new_baud > 50) sp.begin(new_baud);
        
        serialport = &sp;
        
        _Mode = J_ETHERCAT;
        _buf = (unsigned char*)malloc(64);
        
        if (addr != -1)
            _addr = addr;
        _state = JSIDLE;
        
        while (update(JCMD_SET_MODE));
        while (update(JCMD_REFIX));
        
        if (addr != -1)
            while (update(JCMD_SET_ADDR));
        
        unsigned long t = millis();
        while ((millis() - t) < 500) sp.available(); // call update() for 500ms
        while (sp.available()) sp.read(); // clear ALL recv date
    
        _begined = true;
    } else
        return false;
    
    return true;
}

bool _3DJoystick::begin(int addr) {
    if (_Mode == J_NOSETMODE) return false;
    
    if (serialport != NULL) {
        _buf = (unsigned char*)malloc(64);
        
        if (addr != -1)
            _addr = addr;
        _state = JSIDLE;
        
        while (update(JCMD_SET_MODE));
        while (update(JCMD_REFIX));
        
        if (addr != -1)
            while (update(JCMD_SET_ADDR));
        
        unsigned long t = millis();
        while ((millis() - t) < 500) serialport->available(); // call update() for 500ms
        while (serialport->available()) serialport->read(); // clear ALL recv date
    
        _begined = true;
    } else
        return false;

    return true;
}

void _3DJoystick::autoCalibration(void) {
    while (update(JCMD_REFIX));
}

void _3DJoystick::setMasterMode(void) {
    _JMode = MASTER_MODE;
    while (update(JCMD_SET_MODE));
    while (update(JCMD_RESET));
}

void _3DJoystick::setRefreshRate(unsigned char t) {
    if (t < 10 || t > 100) {
        Serial.println("refresh rate is >= 10ms and <= 100ms");
        printf("refresh rate is >= 10ms and <= 100ms");
        return;
    }
    
    _refreshRate = t;
    while (update(JCMD_REF_RATE));
}

void _3DJoystick::setBaudrate(int newBaud) {
    if (!_begined) return;
    
    int index = -1;
    int bLen = sizeof(jsbauds)/sizeof(int);
    for (int i=0; i<bLen; i++) {
        if (newBaud == jsbauds[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    _baudIdx = index;
    while (update(JCMD_SET_BAUD));
    while (update(JCMD_RESET));
}

int _3DJoystick::getX(bool block) {
    if (!_begined) return 0;
    
    if (_JMode == MASTER_MODE) {
        _state = JSRECV;
        _t = millis();
    }
    
    if (block)
        while (update(JCMD_GET_STAT));
    else
        update(JCMD_GET_STAT);
    
    if (_X == -1) return 0;
    
    return _X - 0x200;
}

int _3DJoystick::getY(bool block) {
    if (!_begined) return 0;
    
    if (_JMode == MASTER_MODE) {
        _state = JSRECV;
        _t = millis();
    }
    
    if (block)
        while (update(JCMD_GET_STAT));
    else
        update(JCMD_GET_STAT);
    
    if (_Y == -1) return 0;
    
    return _Y - 0x200;
}

int _3DJoystick::getZ(bool block) {
    if (!_begined) return 0;
    
    if (_JMode == MASTER_MODE) {
        _state = JSRECV;
        _t = millis();
    }
    
    if (block)
        while (update(JCMD_GET_STAT));
    else
        update(JCMD_GET_STAT);
    
    if (_Z == -1) return 0;

    return _Z - 0x200;
}

bool _3DJoystick::btnPressed(bool block) {
    if (!_begined) return false;
    
    if (_JMode == MASTER_MODE) {
        _state = JSRECV;
        _t = millis();
    }
    
    if (block)
        while (update(JCMD_GET_STAT));
    else
        update(JCMD_GET_STAT);
    
    return _btnPressed;
}

_3DJoystick Joystick(1000);



// RFID
// 注意: RFID 的 protocol
// 送出去的封包中，len 這個參數有包含 chksum
// 讀回來的封包中，len 這個參數沒有包含 chksum
// 這是文傑設計上的不一致
#define RFID_DEBUG    (0)

ECRFID::ECRFID(int blockingLevel, unsigned long timeout) {
    myRFID.type = 0;
    myRFID.memsize = 0;
    myRFID.uid_size = 0;
    for (int i=0; i<7; i++) myRFID.uid[i] = 0;
    _timeout = timeout;
    _t = 0;
    _blockingLevel = blockingLevel;
}

static bool isRFIDCheckSumOK(unsigned char* buf, int len) {
    unsigned char chksum = len; // chksum including len, so we set it as len first.
    for (int i=0; i<len; i++)
        chksum += buf[i];
    if (chksum == buf[len]) return true;
    return false;
}

void ECRFID::_handleData() {
    // _recvBuf[0] 總是 command
    int cmd = _recvBuf[0];
    if (cmd == RCMD_SCAN_CARD) {
        myRFID.type = _recvBuf[1];
        myRFID.memsize = ((unsigned short)_recvBuf[3] << 8) | _recvBuf[2];
        myRFID.uid_size = _recvBuf[4];
        for (int i = 0; i < myRFID.uid_size; i++) myRFID.uid[i] = _recvBuf[5+i];
        _result = true;
    } else if (cmd == RCMD_READ_CARD) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = true;
            memset(_readbuf, 0, sizeof(_readbuf));
            _readLen = _recvlen-5; // 要減 5 是因為扣掉 chksum + 0xAA + 0x55 + cmd + result 共 5  個 bytes
            for (int i=0; i<_readLen; i++) 
                _readbuf[i] = _recvBuf[2+i];
        } else {
            _result = false;
            Serial.println("Error: Read Card NG.");
        }
    } else if (cmd == RCMD_SELECT_CARD) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = false; // 不要讓 available() 有反應，當作沒有收到這個回傳封包
            if (RFID_DEBUG) Serial.println("Select card OK.");
        } else {
            _result = false;
            Serial.println("Error: Select card NG.");
        }
    } else if (cmd == RCMD_WRITE_CARD) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = false; // 不要讓 available() 有反應，當作沒有收到這個回傳封包
            if (RFID_DEBUG) Serial.println("Write 4 bytes to card OK.");
        } else {
            _result = false;
            Serial.println("Error: Write 4 bytes to card NG.");
        }
    } else if (cmd == RCMD_COMPATIBILITY_WRITE) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = false; // 不要讓 available() 有反應，當作沒有收到這個回傳封包
            if (RFID_DEBUG) Serial.println("Write 16 bytes to card OK.");
        } else {
            _result = false;
            Serial.println("Error: Write 16 bytes to card NG.");
        }
    } else if (cmd == RCMD_BEEP) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = false; // 不要讓 available() 有反應，當作沒有收到這個回傳封包
            if (RFID_DEBUG) Serial.println("Set beep OK.");
        } else {
            _result = false;
            Serial.println("Error: Set beep NG.");
        }
    } else if (cmd == RCMD_RFLED) {
        if (_recvBuf[1] == 0x01) { // Success
            _result = false; // 不要讓 available() 有反應，當作沒有收到這個回傳封包
            if (RFID_DEBUG) Serial.println("Set LED OK.");
        } else {
            _result = false;
            Serial.println("Error: Set LED NG.");
        }
    }
}

bool ECRFID::_si_update() {
    
    if (_state == RSREADY1) {
        if (serialport->available() >=2) {
            if (serialport->peek() != _commandBuf[0])
                serialport->read(); // 讀掉非 0x55 的值
            else {
                serialport->read(); // 讀掉 0x55
                if (serialport->peek() != _commandBuf[1]) return false; // 重來 (不是讀掉)
                else {
                    serialport->read(); // 讀掉 0xAA
                    _state = RSRECV1;
                    _t = millis();
                }
            }
        }
    } else if (_state == RSRECV1) {
        if (serialport->available() > 0) {
            _recvlen = serialport->read();
            _recvlen += 3; // 實際要接收的長度包含尾部的 0xAA, 0x55 這 2 bytes，另，被我發現，回傳的長度值居然不包含 chksum 這 1 個 byte，所以要再加 1
            _t = millis();
            _state = RSRECV2;
        } else if ((millis() - _t) > _timeout) {
            _state = RSREADY1;
            _result = false;
            Serial.println("state RSRECV1 Timeout!");
        } 
    } else if (_state == RSRECV2) {
        if (serialport->available() >= _recvlen) {
            for (int i=0; i<_recvlen; i++) _recvBuf[i] = serialport->read(); // 先全部放進內部 buf
            if (_recvBuf[_recvlen-2] != 0xAA || _recvBuf[_recvlen-1] != 0x55) {
                _state = RSREADY1;
                _result = false;
                Serial.println("No receive the correct tail data.");
                return _result;
            }
            // 開始處理數據，_recvBuf[0] 是 command，可以跳過
            if (!isRFIDCheckSumOK(_recvBuf, _recvlen - 3)) { // 要扣掉 0xAA, 0x55 尾部這 2 個 bytes 以及 chksum 這 1 個 byte
                _state = RSREADY1;
                _result = false;
                Serial.println("Check sum error\n");
            } else {
                // Check Sum 正確後
                _handleData();
            }
            
            _state = RSREADY1;
        } else if ((millis() - _t) > _timeout) {
            _state = RSREADY1;
            _result = false;
            Serial.println("state RSRECV2 Timeout!");
        }
    }
    
    if (_state != RSREADY1)
        return true; // Working...

    return false;
}

bool ECRFID::update() {
    bool ret = false;
    
    ret = _si_update();
    
    return ret; // false: Complete
               // true: Working...
}

bool ECRFID::begin(RFIDDevice dev, SerialInterface &sp, int id) {
    
    if (dev == RC522 || dev == PN532) {
        // sp.begin(115200);
        
        serialport = &sp;
        
        _RMode = R_ETHERCAT;
        _commandBuf = (unsigned char*)malloc(256);
        _recvBuf = (unsigned char*)malloc(256);
        if (_commandBuf == NULL || _recvBuf == NULL) return false;
        
        if (id != -1)
            _id = id;
        _state = RSREADY1;
        
        unsigned long t = millis();
        while ((millis() - t) < 500) sp.available(); // call update() for 500ms
        while (sp.available()) sp.read(); // clear ALL recv date
    
        _begined = true;
    } else
        return false;
    
    return true;
}

// void ECRFID::update()
// {
//     slave->update();
// }

static unsigned char getRFIDCheckSum(unsigned char* buf, int len) {
    unsigned char chksum = 0;
    for (int i=2; i<len; i++)
        chksum += buf[i];
    return chksum;
}

// 禁止連續呼叫 scan (目前會亂掉)，最好下面接 delay()
bool ECRFID::scan() {
    if (!_begined) return false;
    // if (update()) return false; // 又呼叫相同函式，等之前的命令做完或者 timeout
    
    // 清除曾經讀過的 UID
    myRFID.type = 0;
    myRFID.memsize = 0;
    myRFID.uid_size = 0;
    for (int i = 0; i < 7; i++) myRFID.uid[i] = 0;
        
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = 4; // Length inlcuding checksum
    _commandBuf[3] = RCMD_SCAN_CARD; // Command
    _commandBuf[4] = 0x0A; // Timeout 1
    _commandBuf[5] = 0x00; // Timeout 0
    _commandBuf[6] = getRFIDCheckSum(_commandBuf, 6);
    _commandBuf[7] = 0xAA;
    _commandBuf[8] = 0x55;
    _commLen = 9;
    _result = false;
    
    serialport->write(_commandBuf, _commLen); // 命令會立馬發出

    return _result;
}

bool ECRFID::read(char address) {
    if (!_begined) return false;
    // if (update()) return false; // 又呼叫相同函式，等之前的命令做完或者 timeout
    
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = 3; // Length inlcuding checksum
    _commandBuf[3] = RCMD_READ_CARD; // Command
    _commandBuf[4] = address; // Address Start
    _commandBuf[5] = getRFIDCheckSum(_commandBuf, 5);
    _commandBuf[6] = 0xAA;
    _commandBuf[7] = 0x55;
    _commLen = 8;
    _result = false;
    
    serialport->write(_commandBuf, _commLen); // 命令會立馬發出

    return _result;
}

bool ECRFID::selectCard(char uidSize, char* uid) {
    if (!_begined) return false;
    if (uidSize != 4 && uidSize != 7) return false;
    // if (update()) return false; // 又呼叫相同函式，等之前的命令做完或者 timeout
    
    int i = 0;
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = uidSize + 3; // Length inlcuding checksum
    _commandBuf[3] = RCMD_SELECT_CARD; // Command
    _commandBuf[4] = uidSize;
    for (i=0; i<uidSize; i++)
        _commandBuf[5+i] = uid[i];
    _commandBuf[5+i] = getRFIDCheckSum(_commandBuf, 5+i);
    _commandBuf[6+i] = 0xAA;
    _commandBuf[7+i] = 0x55;
    _commLen = 7+i+1;
    _result = false;
    
    serialport->write(_commandBuf, _commLen); // 命令會立馬發出
    
    return _result;
}

int ECRFID::getType() {
    if (myRFID.type == 0) return -1;
    return myRFID.type;
}

int ECRFID::getMemSize() {
    if (myRFID.memsize == 0) return -1;
    return myRFID.memsize;
}

int ECRFID::getUIDSize() {
    if (myRFID.uid_size == 0) return -1;
    return myRFID.uid_size;
}

unsigned char ECRFID::getUIDByte(int index) {
    if (myRFID.uid_size == 0 || index > 3 || index < 0) return 0;
    return myRFID.uid[index];
}

bool ECRFID::write(CARDCOMMAND cmd, char address, char* userData, int userDataSize) {
    if (!_begined) return false;
    // if (update()) return false; // 又呼叫相同函式，等之前的命令做完或者 timeout
    
    int i = 0;
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = userDataSize + 4; // Length inlcuding checksum
    _commandBuf[3] = cmd;
    _commandBuf[4] = address;
    _commandBuf[5] = userDataSize;
    for (i=0; i<userDataSize; i++)
        _commandBuf[6+i] = userData[i];
    _commandBuf[6+i] = getRFIDCheckSum(_commandBuf, 6+i);
    _commandBuf[7+i] = 0xAA;
    _commandBuf[8+i] = 0x55;
    _commLen = 8+i+1;
    _result = false;
    
    serialport->write(_commandBuf, _commLen); // 命令會立馬發出
    
    return _result;
}

// Block function
bool ECRFID::write4B(char address, char* userData) {
    return write(RCMD_WRITE_CARD, address, userData, 4);
}

// Block function
bool ECRFID::write16B(char address, char* userData) {
    return write(RCMD_COMPATIBILITY_WRITE, address, userData, 16);
}

// Block function
bool ECRFID::MIFR_CA(char sector, char keyNum, char* userData) {

    return true;
}

bool ECRFID::MIFR_UCA(char* userData) {
    
    return true;
}

bool ECRFID::MIFR_21X_AUTHE(char* userData) {
    
    return true;
}

bool ECRFID::Beep(char beepT) {
    if (!_begined) return false;
    
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = 3; // Length inlcuding checksum
    _commandBuf[3] = RCMD_BEEP; // Command
    _commandBuf[4] = beepT; // 次數
    _commandBuf[5] = getRFIDCheckSum(_commandBuf, 5);
    _commandBuf[6] = 0xAA;
    _commandBuf[7] = 0x55;
    _commLen = 8;
    _result = false;
    
    serialport->write(_commandBuf, _commLen); // 命令會立馬發出
    
    return _result;
}

bool ECRFID::Led(char which, bool onoff) {
    if (!_begined) return false;
    
    _commandBuf[0] = 0x55;
    _commandBuf[1] = 0xAA;
    _commandBuf[2] = 4; // Length inlcuding checksum
    _commandBuf[3] = RCMD_RFLED; // Command
    _commandBuf[4] = which; // red:1  green=0
    _commandBuf[5] = onoff;
    _commandBuf[6] = getRFIDCheckSum(_commandBuf, 6);
    _commandBuf[7] = 0xAA;
    _commandBuf[8] = 0x55;
    _commLen = 9;
    _result = false;
    
    serialport->write(_commandBuf, _commLen); // 命令會立馬發出
    
    return _result;
}

void ECRFID::_printResult() {
    int c;
    c = serialport->available();
    if (c > 0) {
      for (;c > 0; c--) fprintf(stderr, "%x ", serialport->read());
    }
}

int ECRFID::_available() {
    return serialport->available();
}

// 目前 available() 用於 RCMD_SCAN_CARD 和 RCMD_READ_CARD 這種需要讀取回傳值的命令用，一個命令配一個 available()，不能兩個以上命令配一個 available()
bool ECRFID::available() {
    int i, c, rlen;
    unsigned long t = 0;
    if (!_begined) return false;
    
    if (_blockingLevel == 2) {
        t = millis();
        do {
            while (update()); // 只處理一個完整封包
            if (_result) break; // 看到 _result 變 true，直接跳出
            if ((millis() - t) > _timeout) {
                Serial.println("RFID available timeout.");
                break;
            }
        } while (_available()); // 假如還有資料，繼續處理所有完整的封包 (外面loop最 lag，適合用 delay)
    } else if (_blockingLevel == 1) {
        while (update()); // 只處理一個完整封包 (外面loop次即時，適合用 millis())
    } else if (_blockingLevel == 0) {
        update(); // 分段處理封包 (外面loop最即時，適合放在多工內)
    }
    
    if (_result) {
        _result = false; // 做完事情了，清除結果
        return true; // user 看到 true 需做該做的事
    }
    
    return false;
}

// Note: you should call pushResultToBuf() after available() return true
int ECRFID::readData(char* data, unsigned int len) {
    int i;
    
    // READ command, len is 16
    for (i=0; i<len; i++) data[i] = _readbuf[i];

    return len;
}

int ECRFID::getDataLen() {
    return _readLen;
}
