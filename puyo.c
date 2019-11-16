#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<sys/time.h>
#include<termios.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<time.h>

#define CCHAR 0

#ifdef CTIME
#undef CTIME
#endif

#define CTIME 1

#define LEFT 0
#define RIGHT 1
#define DOWN 2
#define LROTATE 3
#define RROTATE 4
