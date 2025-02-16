cmake_minimum_required(VERSION 3.28)
project(fastcsv C)

set(CMAKE_C_STANDARD 99)

add_library(fastcsv STATIC
        src/indexing.c
        src/csv.c)
