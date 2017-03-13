#ifndef CRIUS_HTTP_UTIL_H
#define CRIUS_HTTP_UTIL_H

#define SAFE_FREE(PTR) do { if ((PTR) != NULL) { free(PTR); PTR = NULL; } } while(0)

#if !defined(NDEBUG)

#define LOG_INFO(MSG, ...) do { printf("INF[%s:%d] " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(0)

#define LOG_ERROR(MSG, ...) do { fprintf(stderr, "ERR[%s:%d] " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while(0)

#else 

#define LOG_INFO(MSG, ...) do { } while(0)

#define LOG_ERROR(MSG, ...) do { } while(0)

#endif 

#endif // CRIUS_HTTP_UTIL_H
