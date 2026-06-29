#ifndef __FRAM_H
#define __FRAM_H

class Fram {
    public:
        Fram();
        bool begin();
        
        int writeByte(unsigned long idx, unsigned char val);
        int writeByte(unsigned char val) { return writeByte(-1, val); }
        int writeBytes(unsigned long idx, unsigned char *buf, int size);
        int writeWord(unsigned long idx, unsigned short val);
        int writeWord(unsigned short val) { return writeWord(-1, val); }
        int writeWords(unsigned long idx, unsigned short *buf, int size);
        int writeDWord(unsigned long idx, unsigned long val);
        int writeDWord(unsigned long val) { return writeDWord(-1, val); }
        int writeDWords(unsigned long idx, unsigned long *buf, int size);
        int writeDecimal(unsigned long idx, double val);
        int writeDecimal(double val) { return writeDecimal(-1, val); }
        int writeDecimals(unsigned long idx, double *buf, int size);
        unsigned char readByte(unsigned long idx);
        unsigned char readByte() { return readByte(-1); }
        int readBytes(unsigned long idx, unsigned char *buf, int size);
        unsigned short readWord(unsigned long idx);
        unsigned short readWord() { return readWord(-1); }
        int readWords(unsigned long idx, unsigned short *buf, int size);
        unsigned long readDWord(unsigned long idx);
        unsigned long readDWord() { return readDWord(-1); }
        int readDWords(unsigned long idx, unsigned long *buf, int size);
        double readDecimal(unsigned long idx);
        double readDecimal() { return readDecimal(-1); }
        int readDecimals(unsigned long idx, double *buf, int size);
        
    private:
        int _write(unsigned long idx, unsigned char val);
        int _write16(unsigned long idx, unsigned short val);
        int _write32(unsigned long idx, unsigned long val);
        int _write64(unsigned long idx, unsigned long long val);
        int _write64(unsigned long idx, double *val);
        unsigned char _read(unsigned long idx);
        unsigned short _read16(unsigned long idx);
        unsigned long _read32(unsigned long idx);
        unsigned long long _read64(unsigned long idx);
        double _read64_double(unsigned long idx);
        
        unsigned long MMIO_baseAddr;
        unsigned long _addr;
        unsigned long maxCapacity;
        void* io_handle;
        void* pci_p;
};

extern Fram FRAM;
#endif