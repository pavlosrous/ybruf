#ifndef YBRUF_H
#define YBRUF_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

//#define DEBUG

#ifdef DEBUG
#define APP_WD "../run/"
#else
#define APP_WD "/tmp/"
#endif

#define APP_NAME "ybruf"
#define APP_PIDFILE APP_NAME ".pid"

extern const int MIN_SERVERS, MAX_SERVERS;

#endif // YBRUF_H
