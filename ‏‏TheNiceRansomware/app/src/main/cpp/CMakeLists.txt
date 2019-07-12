cmake_minimum_required(VERSION 3.6)


include_directories(src/main/cpp/)

add_library(crypto
        SHARED
        crypto.cpp
        )


target_link_libraries(crypto
                      android
                      log
        )