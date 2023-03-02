#ifndef __INCLUDE_TURN_H__
#define __INCLUDE_TURN_H__

#include <os/bios.h>
#include <type.h>

int is_space(char c);
int st_z(char *a); // string-number
// number-string
void z_st(int i, int radix, char *a, int len, int zero_pad);
// delete extra white space characters
int lstrip(char *s);
int rstrip(char *s);
int strip(char *s);

#endif