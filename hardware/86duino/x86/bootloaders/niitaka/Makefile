# Makefile for DX2/EX MCM

IFALGS    = -I./include
INIFILES  = io.o irq.o err.o uart.o usbcdc.o usbcom.o usbdesc.o usbdev.o usbhid.o usbmsc.o vortex86.o queue.o com.o can.o 
EXEFILES  = v86boot.exe 
OBJFILES  = v86boot.o 
LIBFILES  = -lstdc++ -lm
TARGETPATH= 

.PHONY : everything all clean

everything : $(INIFILES) $(EXEFILES)

all : clean everything

clean :
	-rm -f $(INIFILES) $(EXEFILES) $(OBJFILES)

io.o : src/io.cpp
	gcc -w -c $< $(IFALGS)
irq.o : src/irq.cpp
	gcc -w -c $< $(IFALGS)
uart.o : src/uart.cpp
	gcc -w -c $< $(IFALGS)
err.o : src/err.cpp
	gcc -w -c $< $(IFALGS)
usbcdc.o : src/usbcdc.cpp
	gcc -w -c $< $(IFALGS)
usbcom.o : src/usbcom.cpp
	gcc -w -c $< $(IFALGS)
usbdesc.o : src/usbdesc.cpp
	gcc -w -c $< $(IFALGS) 
usbdev.o : src/usbdev.cpp
	gcc -w -c $< $(IFALGS)  
usbhid.o : src/usbhid.cpp
	gcc -w -c $< $(IFALGS)
usbmsc.o : src/usbmsc.cpp
	gcc -w -c $< $(IFALGS)
vortex86.o : src/vortex86.cpp
	gcc -w -c $< $(IFALGS)
queue.o : src/queue.cpp
	gcc -w -c $< $(IFALGS)
com.o : src/com.cpp
	gcc -w -c $< $(IFALGS)
can.o : src/can.cpp
	gcc -w -c $< $(IFALGS)



v86boot.exe : v86boot.o $(INIFILES)
	gcc -o $@ $< $(INIFILES) $(LIBFILES)
v86boot.o : v86boot.cpp
	gcc -w -c $< $(IFALGS)
