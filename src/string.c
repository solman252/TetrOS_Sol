#include "string.h"

#include "types.h"

void *memset(void *dst, char c, uint n) {
    char *temp = dst;
    for (; n != 0; n--) *temp++ = c; //im not commenting all this shit
    return dst;
}

void *memcpy(void *dst, const void *src, uint n) {
    char *ret = dst;
    char *p = dst;
    const char *q = src;
    while (n--)
        *p++ = *q++;
    return ret;
}

int memcmp(uchar *s1, uchar *s2, uint n) {
    while (n--) {
        if (*s1 != *s2)
            return 0;
        s1++;
        s2++;
    }
    return 1;
}

int str_length(const char *s) {
    int len = 0;
    while (*s++)
        len++;
    return len;
}

int strcmp(const char *s1, char *s2) {
    int i = 0;

    while ((s1[i] == s2[i])) {
        if (s2[i++] == 0)
            return 0;
    }
    return 1;
}

int strcpy(char *dst, const char *src) {
    int i = 0;
    while ((*dst++ = *src++) != 0)
        i++;
    return i;
}

void strcat(char *dest, const char *src) {
    char *end = (char *)dest + str_length(dest);
    memcpy((void *)end, (void *)src, str_length(src));
    end = end + str_length(src);
    *end = '\0';
}

int isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

int isalpha(char c) {
    return (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')));
}

char upper(char c) {
    if ((c >= 'a') && (c <= 'z'))
        return (c - 32);
    return c;
}

char lower(char c) {
    if ((c >= 'A') && (c <= 'Z'))
        return (c + 32);
    return c;
}

void itoa(char *buf, int base, int d) {
    char *p = buf;
    char *p1, *p2;
    unsigned long ud = d;
    int divisor = 10;

    if (base == 'd' && d < 0) {
        *p++ = '-';
        buf++;
        ud = -d;
    } else if (base == 'x')
        divisor = 16;

    do {
        int remainder = ud % divisor;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    } while (ud /= divisor);

    *p = 0;

    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

// Returns the amount of digits in {n}.
// n: int
uint count_digits(int n) {
    uint count = (n <= 0) ? 1 : 0;
    while (n != 0) {
        n /= 10;
        count++;
    }
    return count;
}