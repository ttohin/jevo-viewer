

#pragma once

#include <ostream>
#include "cocos2d.h"

#define KOMORKI_DEBUG_UI
#ifdef KOMORKI_DEBUG_UI
#define KOMORKI_LOG(...) cocos2d::log(__VA_ARGS__)
#else
#define KOMORKI_LOG(...) {}
#endif

//#define LOG_UPDATES

//#define DEBUG_PARTIAL_MAP
//#define DEBUG_VIEWPORT

#ifdef DEBUG_PARTIAL_MAP
#define LOG_W(...) KOMORKI_LOG(__VA_ARGS__)
#else
#define LOG_W(...) {}
#endif


#ifndef _COLORS_
#define _COLORS_

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

#endif  /* _COLORS_ */
