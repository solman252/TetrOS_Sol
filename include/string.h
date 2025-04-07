#ifndef STRING_H
#define STRING_H

#include "types.h"

void *memset(void *dst, char c, uint n);

void *memcpy(void *dst, const void *src, uint n);

int memcmp(uchar *s1, uchar *s2, uint n);

int str_length(const char *s);

int strcmp(const char *s1, char *s2);

int strcpy(char *dst, const char *src);

void strcat(char *dest, const char *src);

int isspace(char c);

int isalpha(char c);
char upper(char c);
char lower(char c);

void itoa(char *buf, int base, int d);

uint count_digits(int n);

#endif