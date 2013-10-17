# Makefile for DX2/EX MCM

IFALGS    = -I./
INIFILES  = common.o io.o irq.o usb.o usb_desc.o vortex86.o queue.o
EXEFILES  = v86boot.exe 
OBJFILES  = v86boot.o 
LIBFILES  = -lstdcxx
TARGETPATH= 

.PHONY : everything all clean

everything : $(INIFILES) $(EXEFILES)

all : clean everything

clean :
	-rm -f $(INIFILES) $(EXEFILES) $(OBJFILES)

common.o : common.cpp
	gcc -w -c $< $(IFALGS)
io.o : io.cpp
	gcc -w -c $< $(IFALGS)
irq.o : irq.cpp
	gcc -w -c $< $(IFALGS)
uart.o : uart.cpp
	gcc -w -c $< $(IFALGS)
usb.o : usb.cpp
	gcc -w -c $< $(IFALGS)
usb_desc.o : usb_desc.cpp
	gcc -w -c $< $(IFALGS)
vortex86.o : vortex86.cpp
	gcc -w -c $< $(IFALGS)
queue.o : queue.cpp



v86boot.exe : v86boot.o $(INIFILES)
	gcc -o $@ $< $(INIFILES) $(LIBFILES)
v86boot.o : v86boot.cpp
	gcc -w -c $< $(IFALGS)
