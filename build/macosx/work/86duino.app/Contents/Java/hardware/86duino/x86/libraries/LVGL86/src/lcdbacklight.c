#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pc.h>

#define SEQ_INDEX  0x3C4
#define SEQ_DATA   0x3C5

static unsigned char get_sr(int index)
{
	outportb(SEQ_INDEX, index);
	return inportb(SEQ_DATA);
}

static void set_sr(int index, unsigned char val, unsigned char mask)
{
	unsigned char regv;

	outportb(SEQ_INDEX, index);
	regv = inportb(SEQ_DATA);
	regv &= ~(mask);
	regv |= (val & mask);
	outportb(SEQ_DATA, regv);
}

static unsigned char get_pwm_level(void)
{
	return get_sr(0x30);
}

void set_pwm_level(unsigned char level)
{
	set_sr(0x2f, 0x7, 0x7);
	set_sr(0x30, level, 0xff);
}
