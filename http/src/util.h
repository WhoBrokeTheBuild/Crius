#ifndef CRIUS_HTTP_UTIL_H
#define CRIUS_HTTP_UTIL_H

#if !defined(NDEBUG)

#define LOG_INFO(MSG, ...) printf("INF[%s:%d] " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR(MSG, ...) fprintf(stderr, "ERR[%s:%d] " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__);

#else 

#define LOG_INFO(MSG, ...) do { } while(false)

#define LOG_ERROR(MSG, ...) do { } while (false)

#endif 

#endif // CRIUS_HTTP_UTIL_H
