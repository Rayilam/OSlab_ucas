#ifndef __INCLUDE_CONSOLE_H__
#define __INCLUDE_CONSOLE_H__

#include <os/bios.h>
#include <os/string.h>
#include <type.h>

char console_getchar(void);//GETCHAR
char CONSOLE_GETPRINT(int bs);//显回
int CONSOLE_GETLINE(char buf[], int bufsize);
int format(char *fmt, char *buf, int len, char *val);//格式化
void CONSOLE_PRINT(char *fmt, char *val);//print

#endif
