#include "util.h"

int cftp_stricmp(char *s1, char *s2)
{
    int i;
    for (; *s1 != '\0'; ++s1, ++s2)
    {
        i = tolower(*s1) - tolower(*s2);
        if (i != 0)
            return i;
    }
    return 0;
}

int cftp_strnicmp(char *s1, char *s2, size_t n)
{
    int i;
    for (; n-- && *s1 != '\0'; ++s1, ++s2)
    {
        i = tolower(*s1) - tolower(*s2);
        if (i != 0)
            return i;
    }
    return 0;
}

