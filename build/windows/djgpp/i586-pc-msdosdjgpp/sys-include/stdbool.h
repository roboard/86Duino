/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_stdbool__h_
#define __dj_stdbool__h_

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#endif /* !__dj_stdbool__h_ */
