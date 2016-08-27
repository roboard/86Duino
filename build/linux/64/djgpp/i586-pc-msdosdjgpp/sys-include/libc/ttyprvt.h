/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_ttyprvt_h__
#define __dj_include_libc_ttyprvt_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#include <dpmi.h>
#include <termios.h>
#include <unistd.h>

#define _TTY_CTRL(x) ((x) & 0x1f)
#define _TTY_QUEUE_SIZE 2048
#define _TTY_EDITLINE_SIZE ((_TTY_QUEUE_SIZE) / 2)
#define _TTY_EDITLINE_CTRL 0
#define _TTY_EDITLINE_SINGLE 1
#define _TTY_EDITLINE_INVALID -1

struct tty_queue
{
  int size;
  unsigned char *top;
  unsigned char *bottom;
  int count;
  unsigned char *rpos;
  unsigned char *wpos;
};

struct tty
{
  struct termios __libc_termios;
  struct tty_queue __libc_tty_queue;
  int __libc_tty_status;
};

struct tty_editline
{
  int col;
  char flag[_TTY_EDITLINE_SIZE];
  unsigned char buf[_TTY_EDITLINE_SIZE];
};

/* Information about the screen.  */
struct tty_screen_info
{
  unsigned char init_attrib;
  unsigned char attrib;
  int max_row;
  int max_col;
  unsigned char active_page;
  unsigned int norm_blink : 1;
  unsigned int cur_blink : 1;
  unsigned short init_cursor_shape;
  void (*set_cursor)(int col, int row);
  void (*get_cursor)(int *col, int *row);
};


/* Pointers to functions that manipulate the screen.  */

struct tty_screen_interface
{
  /* Initialize the interface, if required.  */
  void (*init)(void);
  /* Write a character and update col and row.
     May or may not update the cursor.  */
  void (*write_ch)(unsigned char _ch, int *_col, int *_row);
  /* Write a character and update the cursor.  */
  void (*putch)(unsigned char _ch);
  /* Write a character without updating the cursor.  */
  void (*putch_at)(unsigned char _ch);
  /* Write out a null-terminated string.  */
  void (*puts)(const unsigned char *_s);
  /* Scroll the lines y1 through y2 up to ydst. */
  void (*scroll_up)(int _y1, int _y2, int _ydst);
  /* Scroll the lines y1 through y2 down to ydst. */
  void (*scroll_down)(int _y1, int _y2, int _ydst);
  /* Scroll the characters between x1 and x2 right to xdst on line y.  */
  void (*scroll_left)(int _y, int _x1, int _x2, int _xdst);
  /* Scroll the characters between x1 and x2 left to xdst on line y.  */
  void (*scroll_right)(int _y, int _x1, int _x2, int _xdst);
  /* Clear the space between (x1, y1) and (x2, y2).  */
  void (*clear)(int _x1, int _y1, int _x2, int _y2);
};

#if !defined (_POSIX_VDISABLE) || (_POSIX_VDISABLE == 0)
#error _POSIX_VDISABLE is undefine or zero.
#endif

#define TTYDEFAULT \
{									\
  /* struct termios __libc_termios */					\
  {									\
    /* c_cc[] */							\
    {									\
      (cc_t) 0,               /* pad */ 				\
      (cc_t) _TTY_CTRL ('d'), /* VEOF */				\
      (cc_t) _POSIX_VDISABLE, /* VEOL */				\
      (cc_t) _TTY_CTRL ('h'), /* VERASE */				\
      (cc_t) _TTY_CTRL ('c'), /* VINTR */				\
      (cc_t) _TTY_CTRL ('u'), /* VKILL */				\
      (cc_t) 1,               /* VMIN */				\
      (cc_t) _TTY_CTRL ('\\'),/* VQUIT */				\
      (cc_t) _TTY_CTRL ('q'), /* VSTART */				\
      (cc_t) _TTY_CTRL ('s'), /* VSTOP */				\
      (cc_t) _TTY_CTRL ('z'), /* VSUSP */				\
      (cc_t) 0,               /* VTIME */				\
    },									\
    (tcflag_t) (CS8|CREAD|CLOCAL), /* c_cflag */			\
    (tcflag_t) (BRKINT|ICRNL|IMAXBEL), /* c_iflag */			\
    (tcflag_t) (ISIG|ICANON|ECHO|IEXTEN|ECHOE|ECHOKE|ECHOCTL), /* c_lflag */ \
    (tcflag_t) (OPOST|ONLCR|ONOEOT), /* c_oflag */			\
    (speed_t) (B9600), /* c_ispeed */					\
    (speed_t) (B9600), /* c_ospeed */					\
  },									\
  /* struct tty_queue __libc_tty_queue */				\
  {									\
    _TTY_QUEUE_SIZE,							\
    __libc_tty_queue_buffer,						\
    __libc_tty_queue_buffer + _TTY_QUEUE_SIZE,				\
    0,									\
    __libc_tty_queue_buffer,						\
    __libc_tty_queue_buffer,						\
  },									\
  /* __libc_tty_status */						\
  0,									\
}

#define t_termios __libc_termios
#define t_iflag __libc_termios.c_iflag
#define t_oflag __libc_termios.c_oflag
#define t_cflag __libc_termios.c_cflag
#define t_lflag __libc_termios.c_lflag
#define t_ispeed __libc_termios.c_ispeed
#define t_ospeed __libc_termios.c_ospeed
#define t_cc __libc_termios.c_cc
#define t_status __libc_tty_status

#define t_size __libc_tty_queue.size
#define t_top __libc_tty_queue.top
#define t_bottom __libc_tty_queue.bottom
#define t_count __libc_tty_queue.count
#define t_rpos __libc_tty_queue.rpos
#define t_wpos __libc_tty_queue.wpos

#define _TS_LNCH 0x01 /* next character is literal */
#define _CC_EQU(v,c) (((c) == (unsigned char) __libc_tty_p->t_cc[(v)])	\
		      && ((c) != (unsigned char) _POSIX_VDISABLE))
#define _CC_NEQU(v,c) (((c) != (unsigned char)__libc_tty_p->t_cc[(v)])	\
		       && ((c) != (unsigned char) _POSIX_VDISABLE))

/* internal buffers */
extern unsigned char __libc_tty_queue_buffer[];
extern struct tty __libc_tty_internal;
extern struct tty *__libc_tty_p;
extern struct tty_editline __libc_tty_editline;
extern struct tty_screen_interface __tty_direct_intface;
extern struct tty_screen_interface __tty_vbios_intface;
extern struct tty_screen_interface *__tty_screen_intface;
extern struct tty_screen_info __tty_screen;


/* termios hooks */
extern ssize_t (*__libc_read_termios_hook)(int handle, void *buffer, size_t count,
	   			           ssize_t *rv);
extern ssize_t (*__libc_write_termios_hook)(int handle, const void *buffer, size_t count,
					    ssize_t *rv);
extern int __libc_termios_hook_common_count;

/* functions */
void __libc_termios_init(void);
void __libc_termios_init_read(void);
void __libc_termios_init_write(void);
extern int __libc_termios_exist_queue (void);
int __libc_termios_puts_queue (const unsigned char *_str);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_libc_ttyprvt_h__ */
