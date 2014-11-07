#ifndef MsTimer2_h
#define MsTimer2_h

namespace MsTimer2 {
	extern void (*func)();
	
	void set(unsigned long ms, void (*f)());
	void start();
	void stop();
	void _overflow();
}

#endif
