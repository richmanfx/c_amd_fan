cmake_minimum_required(VERSION 3.10)
project(c_amd_fan C)

set(CMAKE_C_STANDARD 11)

# Название проекта
project(c_amd_fan)

# Исходники
set(SOURCE_EXE c_amd_fan.c)

# Собрать бинарник из исходников
add_executable( c_amd_fan ${SOURCE_EXE})

# Линковка программы с библиотекой
set ( PROJECT_LINK_LIBS libconfig.a )
#set ( PROJECT_LINK_LIBS libconfig.so )
link_directories( /usr/lib/x86_64-linux-gnu )
target_link_libraries( c_amd_fan ${PROJECT_LINK_LIBS} )
