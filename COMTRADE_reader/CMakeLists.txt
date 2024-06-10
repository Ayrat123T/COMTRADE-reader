cmake_minimum_required(VERSION 3.11)

project(ComtradeReader CXX)
set(CMAKE_CXX_STANDARD 20)

set(SOURCE_DIR src)
set(SOURCES_FILES ${SOURCE_DIR}/main.cpp ${SOURCE_DIR}/Comtrade.cpp ${SOURCE_DIR}/DirectoryMonitor.cpp ${SOURCE_DIR}/InputReader.cpp)
set(HEAD_FILES ${SOURCE_DIR}/Comtrade.h ${SOURCE_DIR}/DirectoryMonitor.h ${SOURCE_DIR}/InputReader.h)

add_executable(comtrade ${SOURCES_FILES} ${HEAD_FILES})

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif() 

target_link_libraries(comtrade ${SYSTEM_LIBS})