#ifndef ___ENCODER_INTERFACE_H
#define ___ENCODER_INTERFACE_H

class _Encoder_Interface {
    
    public:
        virtual void write(unsigned long cnt) { write(cnt); };
        virtual unsigned long read(void) { return read(); };
        virtual int directionRead(void) { return directionRead(); };
    
};

#endif