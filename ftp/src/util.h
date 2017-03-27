#ifndef CFTP_UTIL_H
#define CFTP_UTIL_H

#include "common.h"

int cftp_stricmp(char *s1, char *s2);
int cftp_strnicmp(char *s1, char *s2, size_t n);

#define STRICMP     cftp_stricmp
#define STRNICMP    cftp_strnicmp
#define CSTRICMP(S1, S2) STRNICMP(S1, S2, sizeof(S2) - 1)

static inline char * skip_whitespace(char * ptr)
{
    while (isspace(*ptr)) 
        ++ptr;
    return ptr;
}

static inline char * skip_word(char * ptr)
{
    while (!isspace(*ptr))
        ++ptr;
    return ptr;
}

#define SAFE_FREE(PTR) do { if ((PTR) != NULL) { free(PTR); PTR = NULL; } } while(0)

#if !defined(NDEBUG)

#define LOG_INFO(MSG, ...) \
    do { printf("INF[%s:%d] " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(0)

#define LOG_ERROR(MSG, ...) \
    do { fprintf(stderr, "ERR[%s:%d] " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(0)

#else 

#define LOG_INFO(MSG, ...) do { } while(0)
#define LOG_ERROR(MSG, ...) do { } while(0)

#endif 

#endif // CFTP_UTIL_H
