#pragma once

#ifdef _WIN32

struct timezone;

int gettimeofday(struct timeval * tp, struct timezone * tzp);

#endif

