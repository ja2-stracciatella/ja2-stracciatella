cmake_minimum_required(VERSION 2.8)

project(utf8cpp)

install(DIRECTORY source/ DESTINATION include/utf8cpp FILES_MATCHING PATTERN "*.h")
