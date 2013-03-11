# executable
add_executable(example example.cpp)

# link
target_link_libraries(example MicroIni)

# install rules
install(TARGETS example DESTINATION example)
install(FILES input.ini DESTINATION example)
