#ifndef YBRUF_H
#define YBRUF_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <syslog.h>

//#define DEBUG

#ifdef DEBUG
#define APP_WD "../run/"
#else
#define APP_WD "/tmp/"
#endif

#define APP_NAME "ybruf"
#define APP_PIDFILE APP_NAME ".pid"

extern const int MIN_SERVERS, MAX_SERVERS;
bool process_request(int sock_id);

#define PROTO "HTTP/1.1 "
#define DEFAULT_DOC "index.html"

#endif // YBRUF_H
