cmake_minimum_required(VERSION 3.23)
project(test_work_client)

set(CMAKE_CXX_STANDARD 17)
set(path_to_conan_directory "./conan")

include(${path_to_conan_directory}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(test_job_client
        src/main.cpp
        src/source/config.cpp src/source/tcp_session.cpp src/include/config_structs.hpp)

target_link_libraries(test_job_client ${CONAN_LIBS})