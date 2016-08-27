/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_conio_h_
#define __dj_include_conio_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE


extern int directvideo; /* ignored by gppconio */
extern int _wscroll;

#define _NOCURSOR      0
#define _SOLIDCURSOR   1
#define _NORMALCURSOR  2

struct text_info {
    unsigned char winleft;
    unsigned char wintop;
    unsigned char winright;
    unsigned char winbottom;
    unsigned char attribute;
    unsigned char normattr;
    unsigned char currmode;
    unsigned char screenheight;
    unsigned char screenwidth;
    unsigned char curx;
    unsigned char cury;
};

enum text_modes { LASTMODE=-1, BW40=0, C40, BW80, C80, MONO=7, C4350=64 };

enum COLORS {
    /*  dark colors     */
    BLACK,          
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHTGRAY,
    /*  light colors    */
    DARKGRAY, /* "light black" */
    LIGHTBLUE,
    LIGHTGREEN,
    LIGHTCYAN,
    LIGHTRED,
    LIGHTMAGENTA,
    YELLOW,
    WHITE
};

#define BLINK   0x80    /*  blink bit   */

void    blinkvideo(void);
char *  cgets(char *_str); 
void    clreol(void);
void    clrscr(void);
int     _conio_kbhit(void); /* checks for ungetch char */
int     cprintf(const char *_format, ...) __attribute__((format(printf,1,2)));
int     cputs(const char *_str);
int     cscanf(const char *_format, ...) __attribute__((format(scanf,1,2)));
void    delline(void);
int     getch(void);
int     getche(void);
int     _conio_gettext(int _left, int _top, int _right, int _bottom, void *_destin);
void    gettextinfo(struct text_info *_r);
void    gotoxy(int _x, int _y);
void    gppconio_init(void);
void    highvideo(void);
void    insline(void);
void	intensevideo(void);
void    lowvideo(void);
int     movetext(int _left, int _top, int _right, int _bottom, int _destleft, int _desttop);
void    normvideo(void);
int     putch(int _c);
int     puttext(int _left, int _top, int _right, int _bottom, void *_source);
void    _setcursortype(int _type);
void    _set_screen_lines(int _nlines);
void    textattr(int _attr);
void    textbackground(int _color);
void    textcolor(int _color);
void    textmode(int _mode);
int     ungetch(int); 
int     wherex(void);
int     wherey(void);
void    window(int _left, int _top, int _right, int _bottom);

#define kbhit _conio_kbhit /* Who ever includes gppconio.h probably
                              also wants _conio_kbhit and not kbhit
                              from libc */

/* This is to resolve the name clash between
   gettext from conio.h and gettext from libintl.h.
   IMPORTANT:
   If both headers are included, the gettext keyword will always
   make reference to the GNU gettext function declared in libintl.h
   and never to the BORLAND-compatibility gettext function declared
   in conio.h. In this case, BORLAND-compatibility gettext function
   will only be available as _conio_gettext. */
#ifndef __USE_GNU_GETTEXT
# undef  gettext
# define gettext _conio_gettext
#endif

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_conio_h_ */
