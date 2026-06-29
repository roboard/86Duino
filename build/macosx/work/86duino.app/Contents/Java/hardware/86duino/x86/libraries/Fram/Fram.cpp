#include "io.h"
#include "Fram.h"

#define FUJITSU_CHIP

#if defined (CYPRESS_CHIP)

    // In BIOS Setting:
    // BASE : 80C00000
    // MasK: 0FFFFC00
    #define GPCS0BASE_ADDR (0x90)
    #define FRAM_SIZE      (0x40000L)

#elif defined (FUJITSU_CHIP)

    // In BIOS Setting:
    // BASE : 80C00000
    // MasK: 0FFFF800
    #define GPCS0BASE_ADDR (0x90)
    #define FRAM_SIZE      (0x80000L)
    #define FUJITSU_MAX    (128 * 2) // 128 x 16bits
    
#endif

Fram::Fram() {
    io_handle = pci_p = NULL;
    MMIO_baseAddr = 0L;
    _addr = 0L;
    maxCapacity = FUJITSU_MAX * 1024;
}

bool Fram::begin() {
    if ((pci_p = pci_Alloc(0x00, 0x07, 0x00)) != NULL) {
        MMIO_baseAddr = pci_In32(pci_p, GPCS0BASE_ADDR) & 0xFFFFFFFEL;
        if ((io_handle = io_Alloc(IO_USE_MMIO, MMIO_baseAddr, FRAM_SIZE)) == NULL) {
            printf("Memory allocation fail\n");
            return false;
        }
    } else
        return false;
    
    // printf("MMIO_baseAddr=%08X\n", MMIO_baseAddr);
    return true;
}

int Fram::_write(unsigned long idx, unsigned char val) {
    unsigned short data;
    
    if (io_handle == NULL) return 0;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        idx = idx >> 1; // Don't put it out of if ((idx ...
        data = io_In16(io_handle, idx << 1); // Don't put it out of if ((idx ...
        data = (data & 0x00FF) | ((unsigned short)val << 8);
    } else { // 0, 2, 4, 6, 8, 10, ...
        idx = idx >> 1; // Don't put it out of if ((idx ...
        data = io_In16(io_handle, idx << 1); // Don't put it out of if ((idx ...
        data = (data & 0xFF00) | val;
    }
    
    io_Out16(io_handle, idx << 1, data);

    return 1;
}

// Note: The max address can be write is half space of byte version.
int Fram::_write16(unsigned long idx, unsigned short val) {
    if (io_handle == NULL) return 0;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        _write((idx + 0), val & 0x00FF);
        _write((idx + 1), (val & 0xFF00) >> 8);
    } else { // 0, 2, 4, 6, 8, 10, ...
        io_Out16(io_handle, idx, val); // we don't use SA0 but SA1 for strating, so shift left 1 bit
    }
    
    return 2;
}

int Fram::_write32(unsigned long idx, unsigned long val) {
    if (io_handle == NULL) return 0;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        _write((idx + 0), val & 0x000000FFL);
        io_Out16(io_handle, (idx + 1), (val & 0x00FFFF00L) >> 8);
        _write((idx + 3), (val & 0xFF000000L) >> 24);
    } else { // 0, 2, 4, 6, 8, 10, ...
        io_Out16(io_handle, (idx + 0), val & 0x0000FFFFL);
        io_Out16(io_handle, (idx + 2), (val & 0xFFFF0000L) >> 16);
    }
    
    return 4;
}

int Fram::_write64(unsigned long idx, unsigned long long val) {
    if (io_handle == NULL) return 0;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        _write(idx, val & 0x00000000000000FFLL);
        io_Out16(io_handle, (idx + 1), (val & 0x0000000000FFFF00LL) >> 8);
        io_Out16(io_handle, (idx + 3), (val & 0x000000FFFF000000LL) >> 24);
        io_Out16(io_handle, (idx + 5), (val & 0x00FFFF0000000000LL) >> 40);
        _write(idx + 7, (val & 0xFF00000000000000LL) >> 56);
    } else { // 0, 2, 4, 6, 8, 10, ...
        io_Out16(io_handle, (idx + 0), val & 0x000000000000FFFFLL);
        io_Out16(io_handle, (idx + 2), (val & 0x00000000FFFF0000LL) >> 16);
        io_Out16(io_handle, (idx + 4), (val & 0x0000FFFF00000000LL) >> 32);
        io_Out16(io_handle, (idx + 6), (val & 0xFFFF000000000000LL) >> 48);
    }
    
    return 8;
}

int Fram::_write64(unsigned long idx, double *val) {
    unsigned char* _val = (unsigned char*)val;
    
    if (io_handle == NULL) return 0;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        _write(idx, *_val);
        io_Out16(io_handle, (idx + 1), *(unsigned short*)(_val + 1));
        io_Out16(io_handle, (idx + 3), *(unsigned short*)(_val + 3));
        io_Out16(io_handle, (idx + 5), *(unsigned short*)(_val + 5));
        _write(idx + 7, *(_val + 7));
    } else { // 0, 2, 4, 6, 8, 10, ...
        io_Out16(io_handle, (idx + 0), *(unsigned short*)(_val));
        io_Out16(io_handle, (idx + 2), *(unsigned short*)(_val + 2));
        io_Out16(io_handle, (idx + 4), *(unsigned short*)(_val + 4));
        io_Out16(io_handle, (idx + 6), *(unsigned short*)(_val + 6));
    }
    
    return 8;
}

unsigned char Fram::_read(unsigned long idx) {
    if (io_handle == NULL) return 0;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        idx = idx >> 1;
        return (io_In16(io_handle, idx << 1) >> 8) & 0xFF;
    } else { // 0, 2, 4, 6, 8, 10, ...
        return io_In16(io_handle, idx) & 0xFF;
    }
    
    return 0;
}

// Note: The max address can be read is half space of byte version.
unsigned short Fram::_read16(unsigned long idx) {
    unsigned short data = 0;
    
    if (io_handle == NULL) return 0;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        data = _read(idx);
        data |= (((unsigned short)_read(idx + 1)) << 8);
        return data;
    } else { // 0, 2, 4, 6, 8, 10, ...
        return io_In16(io_handle, idx);
    }
    
    return 0;
}

unsigned long Fram::_read32(unsigned long idx) {
    unsigned long data = 0L;
    
    if (io_handle == NULL) return 0L;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        data = _read(idx);
        data |= ((unsigned long)io_In16(io_handle, idx + 1)) << 8;
        data |= (((unsigned long)_read(idx + 3)) << 24);
        return data;
    } else { // 0, 2, 4, 6, 8, 10, ...
        data = io_In16(io_handle, idx);
        data |= ((unsigned long)io_In16(io_handle, idx + 2)) << 16;
        return data;
    }
    
    return 0L;
}

unsigned long long Fram::_read64(unsigned long idx) {
    unsigned long long data = 0LL;
    
    if (io_handle == NULL) return 0LL;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        data = _read(idx);
        data |= ((unsigned long long)io_In16(io_handle, idx + 1)) << 8;
        data |= ((unsigned long long)io_In16(io_handle, idx + 3)) << 24;
        data |= ((unsigned long long)io_In16(io_handle, idx + 5)) << 40;
        data |= (((unsigned long long)_read(idx + 7)) << 56);
        return data;
    } else { // 0, 2, 4, 6, 8, 10, ...
        data = io_In16(io_handle, idx);
        data |= ((unsigned long long)io_In16(io_handle, idx + 2)) << 16;
        data |= ((unsigned long long)io_In16(io_handle, idx + 4)) << 32;
        data |= ((unsigned long long)io_In16(io_handle, idx + 6)) << 48;
        return data;
    }
    
    return 0LL;
}

double Fram::_read64_double(unsigned long idx) {
    double data = 0.0;
    unsigned char* _val = (unsigned char*)&data;
    
    if (io_handle == NULL) return 0LL;
    
    if ((idx & 0x01) != 0) { // 1, 3, 5, 7, 9, ...
        *_val = _read(idx);
        *(unsigned short*)(_val + 1) = io_In16(io_handle, idx + 1);
        *(unsigned short*)(_val + 3) = io_In16(io_handle, idx + 3);
        *(unsigned short*)(_val + 5) = io_In16(io_handle, idx + 5);
        *(_val + 7) = _read(idx + 7);
        return data;
    } else { // 0, 2, 4, 6, 8, 10, ...
        *(unsigned short*)(_val) = io_In16(io_handle, idx);
        *(unsigned short*)(_val + 2) = io_In16(io_handle, idx + 2);
        *(unsigned short*)(_val + 4) = io_In16(io_handle, idx + 4);
        *(unsigned short*)(_val + 6) = io_In16(io_handle, idx + 6);
        return data;
    }
    
    return 0LL;
}

// User-App
int Fram::writeByte(unsigned long idx, unsigned char val) {
    long _idx = (long)idx;
    
    if (_idx == -1) {
        if ((_addr + 1) > maxCapacity) return 0;
    } else {
        if (idx > maxCapacity) return 0;
        if ((idx + 1) > maxCapacity) return 0;
    }
        
    if (_idx == -1)
        _addr += _write(_addr, val);
    else {
        _addr = idx;
        _addr += _write(idx, val);
    }
    
    return 1;
}

int Fram::writeBytes(unsigned long idx, unsigned char *buf, int size) {
    int bytes = 1;
    long _idx = (long)idx;
    
    if (_idx == -1 || buf == NULL || size == 0) return 0; // 陣列式 API 一定要傳 idx
    
    for (int i=0; i<size; i++) {
        if (writeByte(idx + (i * bytes), buf[i]) < bytes) return i;
    }
    
    return size;
}

int Fram::writeWord(unsigned long idx, unsigned short val) {
    bool over = false;
    long _idx = (long)idx;
    int bytes = 2;
    
    if (_idx == -1) {
        if ((_addr + bytes) > maxCapacity) over = true;
    } else {
        if (idx > maxCapacity) return 0; // idx 本身就是一個極大值，就不用做 1 次寫 1 Byte 的動作了
        if ((idx + bytes) > maxCapacity) over = true;
    }
    
    if (over) {
        if (_idx == -1) {
            for (int i=0; i<bytes; i++) {
                if ((_addr + 1) > maxCapacity) // 1 次寫 1 Byte，所以只檢查 +1 是否超過
                    return i;
                _addr += _write(_addr, (val >> (8 * i)) & 0x00FF); // 1 次寫 1 Byte
            }
        } else {
            _addr = idx;
            for (int i=0; i<bytes; i++) {
                if ((_addr + 1) > maxCapacity)
                    return i;
                _addr += _write(idx + i, (val >> (8 * i)) & 0x00FF);
            }
        }
    }
    
    if (_idx == -1)
        _addr += _write16(_addr, val); // 1 次寫 2 Byte
    else {
        _addr = idx;
        _addr += _write16(idx, val); // 1 次寫 2 Byte
    }
    
    return bytes;
}

int Fram::writeWords(unsigned long idx, unsigned short *buf, int size) {
    int bytes = 2;
    long _idx = (long)idx;
    
    if (_idx == -1 || buf == NULL || size == 0) return 0;
    
    for (int i=0; i<size; i++) {
        if (writeWord(idx + (i * bytes), buf[i]) < bytes) return i;
    }
    
    return size;
}

int Fram::writeDWord(unsigned long idx, unsigned long val) {
    bool over = false;
    long _idx = (long)idx;
    int bytes = 4;
    
    if (_idx == -1) {
        if ((_addr + bytes) > maxCapacity) over = true;
    } else {
        if (idx > maxCapacity) return 0;
        if ((idx + bytes) > maxCapacity) over = true;
    }
    
    if (over) {
        if (_idx == -1) {
            for (int i=0; i<bytes; i++) {
                if ((_addr + 1) > maxCapacity) // 1 次寫 1 Byte，所以只檢查 +1 是否超過
                    return i;
                _addr += _write(_addr, (val >> (8 * i)) & 0x000000FFL); // 1 次寫 1 Byte
            }
        } else {
            _addr = idx;
            for (int i=0; i<bytes; i++) {
                if ((_addr + 1) > maxCapacity)
                    return i;
                _addr += _write(idx + i, (val >> (8 * i)) & 0x000000FFL);
            }
        }
    }
    
    if (_idx == -1)
        _addr += _write32(_addr, val); // 1 次寫 4 Byte
    else {
        _addr = idx;
        _addr += _write32(idx, val); // 1 次寫 4 Byte
    }
    
    return bytes;
}

int Fram::writeDWords(unsigned long idx, unsigned long *buf, int size) {
    int bytes = 4;
    long _idx = (long)idx;
    
    if (_idx == -1 || buf == NULL || size == 0) return 0;
    
    for (int i=0; i<size; i++) {
        if (writeDWord(idx + (i * bytes), buf[i]) < bytes) return i;
    }
    
    return size;
}

int Fram::writeDecimal(unsigned long idx, double val) {
    bool over = false;
    long _idx = (long)idx;
    int bytes = 8;
    
    // double 浮點數是特殊 case，位址一旦超過什麼都不做 (不像其他的會 1 次寫 1 個 Byte)
    if (_idx == -1) {
        if ((_addr + bytes) > maxCapacity) return 0;
    } else {
        if (idx > maxCapacity) return 0;
        if ((idx + bytes) > maxCapacity) return 0;
    }
    
    if (_idx == -1)
        _addr += _write64(_addr, &val); // 1 次寫 8 Byte
    else {
        _addr = idx;
        _addr += _write64(idx, &val); // 1 次寫 8 Byte
    }
    
    return bytes;
}

int Fram::writeDecimals(unsigned long idx, double *buf, int size) {
    int bytes = 8;
    long _idx = (long)idx;
    
    if (_idx == -1 || buf == NULL || size == 0) return 0;
    
    for (int i=0; i<size; i++) {
        if (writeDecimal(idx + (i * bytes), buf[i]) < bytes) return i;
    }
    
    return size;
}

unsigned char Fram::readByte(unsigned long idx) {
    unsigned char data = 0;
    long _idx = (long)idx;
    
    if (_idx == -1) {
        if ((_addr + 1) > maxCapacity) return 0xFF;
    } else {
        if (idx > maxCapacity) return 0xFF;
        if ((idx + 1) > maxCapacity) return 0xFF;
    }
        
    if (_idx == -1) {
        data = _read(_addr);
    } else {
        _addr = idx;
        data = _read(idx);
    }
    
    _addr += 1;
    
    return data;
}

int Fram::readBytes(unsigned long idx, unsigned char *buf, int size) {
    long _idx = (long)idx;
    
    if (_idx == -1 || buf == NULL || size == 0) return 0;
    
    for (int i=0; i<size; i++) {
        buf[i] = readByte(idx + i);
    }
    
    return (_addr > maxCapacity) ? 1 : 0;
}

unsigned short Fram::readWord(unsigned long idx) {
    unsigned short data = 0;
    bool over = false;
    long _idx = (long)idx;
    int bytes = 2;
    
    if (_idx == -1) {
        if ((_addr + bytes) > maxCapacity) over = true;
    } else {
        if (idx > maxCapacity) return 0; // idx 本身就是一個極大值，就不用做 1 次讀 1 Byte 的動作了
        if ((idx + bytes) > maxCapacity) over = true;
    }
    
    if (over) {
        if (_idx == -1) {
            for (int i=0; i<bytes; i++) {
                if ((_addr + 1) > maxCapacity) // 1 次讀 1 Byte，所以只檢查 +1 是否超過
                    data |= (((unsigned short)0xFF) << (8 *i)); // 超過邊界，全部填 0xFF
                else
                    data |= (((unsigned short)_read(_addr)) << (8 *i)); // 1 次讀 1 Byte
                _addr += 1;
            }
        } else {
            _addr = idx;
            for (int i=0; i<bytes; i++) {
                if ((_addr + 1) > maxCapacity)
                    data |= (((unsigned short)0xFF) << (8 *i));
                else
                    data |= (((unsigned short)_read(idx + i)) << (8 *i)); // 1 次讀 1 Byte
                _addr += 1;
            }
        }
        return data;
    }
    
    if (_idx == -1)
        data = _read16(_addr); // 1 次讀 2 Byte
    else {
        _addr = idx;
        data = _read16(idx); // 1 次讀 2 Byte
    }
    
    _addr += bytes;
    
    return data;
}

int Fram::readWords(unsigned long idx, unsigned short *buf, int size) {
    long _idx = (long)idx;
    int bytes = 2;
    
    if (_idx == -1 || buf == NULL || size == 0) return 0;
    
    for (int i=0; i<size; i++) {
        buf[i] = readWord(idx + (i * bytes));
    }
    
    return ((idx + (bytes * size)) > maxCapacity) ? 1 : 0;
}

unsigned long Fram::readDWord(unsigned long idx) {
    unsigned long data = 0;
    bool over = false;
    long _idx = (long)idx;
    int bytes = 4;
    
    if (_idx == -1) {
        if ((_addr + bytes) > maxCapacity) over = true;
    } else {
        if (idx > maxCapacity) return 0; // idx 本身就是一個極大值，就不用做 1 次讀 1 Byte 的動作了
        if ((idx + bytes) > maxCapacity) over = true;
    }
    
    if (over) {
        if (_idx == -1) {
            for (int i=0; i<bytes; i++) {
                if ((_addr + 1) > maxCapacity) // 1 次讀 1 Byte，所以只檢查 +1 是否超過
                    data |= (((unsigned long)0xFF) << (8 *i)); // 超過邊界，全部填 0xFF
                else
                    data |= (((unsigned long)_read(_addr)) << (8 *i)); // 1 次讀 1 Byte
                _addr += 1;
            }
        } else {
            _addr = idx;
            for (int i=0; i<bytes; i++) {
                if ((_addr + 1) > maxCapacity)
                    data |= (((unsigned long)0xFF) << (8 *i));
                else
                    data |= (((unsigned long)_read(idx + i)) << (8 *i)); // 1 次讀 1 Byte
                _addr += 1;
            }
        }
        return data;
    }
    
    if (_idx == -1)
        data = _read32(_addr); // 1 次讀 4 Byte
    else {
        _addr = idx;
        data = _read32(idx); // 1 次讀 4 Byte
    }
    
    _addr += bytes;
    
    return data;
}

int Fram::readDWords(unsigned long idx, unsigned long *buf, int size) {
    long _idx = (long)idx;
    int bytes = 4;
    
    if (_idx == -1 || buf == NULL || size == 0) return 0;
    
    for (int i=0; i<size; i++) {
        buf[i] = readDWord(idx + (i * bytes));
    }
    
    return ((idx + (bytes * size)) > maxCapacity) ? 1 : 0;
}

double Fram::readDecimal(unsigned long idx) {
    double data = 0;
    bool over = false;
    long _idx = (long)idx;
    int bytes = 8;
    
    if (_idx == -1) {
        if ((_addr + bytes) > maxCapacity) return 0.0;
    } else {
        if (idx > maxCapacity) return 0.0; // idx 本身就是一個極大值，就不用做 1 次讀 1 Byte 的動作了
        if ((idx + bytes) > maxCapacity) return 0.0;
    }
    
    if (_idx == -1)
        data = _read64_double(_addr); // 1 次讀 8 Byte
    else {
        _addr = idx;
        data = _read64_double(idx); // 1 次讀 8 Byte
    }
    
    _addr += bytes;
    
    return data;
}

int Fram::readDecimals(unsigned long idx, double *buf, int size) {
    long _idx = (long)idx;
    int bytes = 8;
    
    if (_idx == -1 || buf == NULL || size == 0) return 0;
    
    for (int i=0; i<size; i++) {
        buf[i] = readDecimal(idx + (i * bytes));
    }
    
    return ((idx + (bytes * size)) > maxCapacity) ? 1 : 0;
}


Fram FRAM;