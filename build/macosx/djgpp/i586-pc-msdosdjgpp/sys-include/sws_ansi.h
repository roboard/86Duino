/****************************************************************************
 *
 * File name   : sws_ansi.h
 * Function    : ANSI terminal control.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 19-Jul-09, LVR, v1.00 - Created
 *
 *
 * Comments:
 * --------
 * This file defines common ANSI terminal control codes
 * See http://ascii-table.com/ansi-escape-sequences.php
 *
 ****************************************************************************/

#ifndef SWS_ANSI_H
#define SWS_ANSI_H 0x0100

#ifndef SWS_ANSI_ENABLE
#define SWS_ANSI_ENABLE 1
#endif
#if SWS_ANSI_ENABLE
#define ANSI_CMD( _s) "\x1b[" _s
#else
#define ANSI_CMD( _s)
#endif

#define ANSI_STRS( _s)  #_s
#define ANSI_STR( _s)  ANSI_STRS( _s)

#define ANSI_CUU( _n) ANSI_CMD( ANSI_STR( _n) "A")
#define ANSI_CUD( _n) ANSI_CMD( ANSI_STR( _n) "B")
#define ANSI_CUF( _n) ANSI_CMD( ANSI_STR( _n) "C")
#define ANSI_CUB( _n) ANSI_CMD( ANSI_STR( _n) "D")
#define ANSI_CNL( _n) ANSI_CMD( ANSI_STR( _n) "E")
#define ANSI_CPL( _n) ANSI_CMD( ANSI_STR( _n) "F")
#define ANSI_CHA( _n) ANSI_CMD( ANSI_STR( _n) "G")
#define ANSI_CUP( _x,_y) ANSI_CMD( ANSI_STR( _y) ";" ANSI_STR( _x) "H")
#define ANSI_ED( _n)  ANSI_CMD( ANSI_STR( _n) "J")
#define ANSI_EL( _n)  ANSI_CMD( ANSI_STR( _n) "K")
#define ANSI_SU( _n)  ANSI_CMD( ANSI_STR( _n) "S")
#define ANSI_SD( _n)  ANSI_CMD( ANSI_STR( _n) "T")
#define ANSI_HVP( _x,_y) ANSI_CMD( ANSI_STR( _y) ";" ANSI_STR( _x) "f")
#define ANSI_SGR( _s) ANSI_CMD( _s "m")
#define ANSI_SCP   ANSI_CMD( "s")
#define ANSI_RCP   ANSI_CMD( "u")

/* ANSI_SGR parameters */

#define ANSI_SGR_RESET  "0"
#define ANSI_SGR_BOLD   "1"
#define ANSI_SGR_FAINT  "2"
#define ANSI_SGR_ITAL1  "3"
#define ANSI_SGR_UNDR1  "4"
#define ANSI_SGR_BLNK1  "5"
#define ANSI_SGR_BLNK2  "6"
#define ANSI_SGR_INVRS  "7"
#define ANSI_SGR_CNCL   "8"
#define ANSI_SGR_UNDR2  "21"
#define ANSI_SGR_NRML   "22"
#define ANSI_SGR_UNDR0  "24"
#define ANSI_SGR_BLNK0  "25"
#define ANSI_SGR_PSTV   "27"
#define ANSI_SGR_RVL    "28"

#define ANSI_SGR_FG_BLK "30"
#define ANSI_SGR_FG_RED "31"
#define ANSI_SGR_FG_GRN "32"
#define ANSI_SGR_FG_YEL "33"
#define ANSI_SGR_FG_BLU "34"
#define ANSI_SGR_FG_MAG "35"
#define ANSI_SGR_FG_CYN "36"
#define ANSI_SGR_FG_WHT "37"
#define ANSI_SGR_FG_RST "39"

#define ANSI_SGR_BG_BLK "40"
#define ANSI_SGR_BG_RED "41"
#define ANSI_SGR_BG_GRN "42"
#define ANSI_SGR_BG_YEL "43"
#define ANSI_SGR_BG_BLU "44"
#define ANSI_SGR_BG_MAG "45"
#define ANSI_SGR_BG_CYN "46"
#define ANSI_SGR_BG_WHT "47"
#define ANSI_SGR_BG_RST "49"

#define ANSI_SGR_FG_HIBLK "90"
#define ANSI_SGR_FG_HIRED "91"
#define ANSI_SGR_FG_HIGRN "92"
#define ANSI_SGR_FG_HIYEL "93"
#define ANSI_SGR_FG_HIBLU "94"
#define ANSI_SGR_FG_HIMAG "95"
#define ANSI_SGR_FG_HICYN "96"
#define ANSI_SGR_FG_HIWHT "97"

#define ANSI_SGR_BG_HIBLK "100"
#define ANSI_SGR_BG_HIRED "101"
#define ANSI_SGR_BG_HIGRN "102"
#define ANSI_SGR_BG_HIYEL "103"
#define ANSI_SGR_BG_HIBLU "104"
#define ANSI_SGR_BG_HIMAG "105"
#define ANSI_SGR_BG_HICYN "106"
#define ANSI_SGR_BG_HIWHT "107"

/* Graphic convenience macros */

#define ANSI_RST   ANSI_SGR( ANSI_SGR_RESET)
#define ANSI_BOLD  ANSI_SGR( ANSI_SGR_BOLD)
#define ANSI_FAINT ANSI_SGR( ANSI_SGR_FAINT)
#define ANSI_ITAL1 ANSI_SGR( ANSI_SGR_ITAL1)
#define ANSI_UNDR1 ANSI_SGR( ANSI_SGR_UNDR1)
#define ANSI_BLNK1 ANSI_SGR( ANSI_SGR_BLNK1)
#define ANSI_BLNK2 ANSI_SGR( ANSI_SGR_BLNK2)
#define ANSI_INVRS ANSI_SGR( ANSI_SGR_INVRS)
#define ANSI_CNCL  ANSI_SGR( ANSI_SGR_CNCL)
#define ANSI_UNDR2 ANSI_SGR( ANSI_SGR_UNDR2)
#define ANSI_NRML  ANSI_SGR( ANSI_SGR_NRML)
#define ANSI_UNDR0 ANSI_SGR( ANSI_SGR_UNDR0)
#define ANSI_BLNK0 ANSI_SGR( ANSI_SGR_BLNK0)
#define ANSI_PSTV  ANSI_SGR( ANSI_SGR_PSTV)
#define ANSI_RVL   ANSI_SGR( ANSI_SGR_RVL)

#define ANSI_FG_BLK ANSI_SGR( ANSI_SGR_FG_BLK)
#define ANSI_FG_RED ANSI_SGR( ANSI_SGR_FG_RED)
#define ANSI_FG_GRN ANSI_SGR( ANSI_SGR_FG_GRN)
#define ANSI_FG_YEL ANSI_SGR( ANSI_SGR_FG_YEL)
#define ANSI_FG_BLU ANSI_SGR( ANSI_SGR_FG_BLU)
#define ANSI_FG_MAG ANSI_SGR( ANSI_SGR_FG_MAG)
#define ANSI_FG_CYN ANSI_SGR( ANSI_SGR_FG_CYN)
#define ANSI_FG_WHT ANSI_SGR( ANSI_SGR_FG_WHT)
#define ANSI_FG_RST ANSI_SGR( ANSI_SGR_FG_RST)

#define ANSI_BG_BLK ANSI_SGR( ANSI_SGR_BG_BLK)
#define ANSI_BG_RED ANSI_SGR( ANSI_SGR_BG_RED)
#define ANSI_BG_GRN ANSI_SGR( ANSI_SGR_BG_GRN)
#define ANSI_BG_YEL ANSI_SGR( ANSI_SGR_BG_YEL)
#define ANSI_BG_BLU ANSI_SGR( ANSI_SGR_BG_BLU)
#define ANSI_BG_MAG ANSI_SGR( ANSI_SGR_BG_MAG)
#define ANSI_BG_CYN ANSI_SGR( ANSI_SGR_BG_CYN)
#define ANSI_BG_WHT ANSI_SGR( ANSI_SGR_BG_WHT)
#define ANSI_BG_RST ANSI_SGR( ANSI_SGR_BG_RST)

#define ANSI_FG_HIBLK ANSI_SGR( ANSI_SGR_FG_HIBLK)
#define ANSI_FG_HIRED ANSI_SGR( ANSI_SGR_FG_HIRED)
#define ANSI_FG_HIGRN ANSI_SGR( ANSI_SGR_FG_HIGRN)
#define ANSI_FG_HIYEL ANSI_SGR( ANSI_SGR_FG_HIYEL)
#define ANSI_FG_HIBLU ANSI_SGR( ANSI_SGR_FG_HIBLU)
#define ANSI_FG_HIMAG ANSI_SGR( ANSI_SGR_FG_HIMAG)
#define ANSI_FG_HICYN ANSI_SGR( ANSI_SGR_FG_HICYN)
#define ANSI_FG_HIWHT ANSI_SGR( ANSI_SGR_FG_HIWHT)

#define ANSI_BG_HIBLK ANSI_SGR( ANSI_SGR_BG_HIBLK)
#define ANSI_BG_HIRED ANSI_SGR( ANSI_SGR_BG_HIRED)
#define ANSI_BG_HIGRN ANSI_SGR( ANSI_SGR_BG_HIGRN)
#define ANSI_BG_HIYEL ANSI_SGR( ANSI_SGR_BG_HIYEL)
#define ANSI_BG_HIBLU ANSI_SGR( ANSI_SGR_BG_HIBLU)
#define ANSI_BG_HIMAG ANSI_SGR( ANSI_SGR_BG_HIMAG)
#define ANSI_BG_HICYN ANSI_SGR( ANSI_SGR_BG_HICYN)
#define ANSI_BG_HIWHT ANSI_SGR( ANSI_SGR_BG_HIWHT)

#endif /* ndef SWS_ANSI_H */
/* End of file */
