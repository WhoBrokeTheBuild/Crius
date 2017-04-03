
file(GLOB chttp_SOURCES 
    "src/*.h"
    "src/*.c"
)

add_executable (chttp ${chttp_SOURCES})
target_link_libraries(chttp pthread ${CMAKE_DL_LIBS})
