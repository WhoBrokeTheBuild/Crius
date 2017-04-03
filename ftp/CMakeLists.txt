
file(GLOB cftp_SOURCES 
    "src/*.h"
    "src/*.c"
)

add_executable (cftp ${cftp_SOURCES})

