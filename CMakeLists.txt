cmake_minimum_required(VERSION 3.14)
project(FTCD_UnitTests C)

set(CMAKE_C_STANDARD 11)

add_executable(FTCD_UnitTests main.c encoder.h decoder.h rs.h transfer.h)