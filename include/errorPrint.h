#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef __ERRORPRINT_H
#define __ERRORPRINT_H

// 这一行绝对不能放在某个报错函数里面，因为这里面的宏只能检测当前的环境

// 为了方便在str添加%d %s类型的参数，可以借助sprintf和snrpintf等函数
#define PRINTERROR(str_format)     \
    char errorBuf[BUFSIZ];         \
    sprintf(errorBuf, str_format); \
    fprintf(stderr, "\033[31mFunction  %s() error at file %s line %d Info: %s\033[0m\n", __func__, __FILE__, __LINE__, errorBuf);

#define PRINTERROREXIT(str_format)                                                                                           \
    char errorBuf[BUFSIZ];                                                                                                   \
    sprintf(errorBuf, str_format);                                                                                           \
    fprintf(stderr, "\033[31mFunction  %s() error at file %s line %d Info: %s\033[0m\n", __func__, __FILE__, __LINE__, str); \
    fprintf(stderr, "exiting");                                                                                              \
    EXIT(EXIT_FAILURE);

void DieWithUserMessage(const char *msg);

void DieWithSystemMessage(const char *msg);

#endif
