cmake_minimum_required(VERSION 3.24)

project(radioss_file_converter
  VERSION 1.0
  LANGUAGES CXX
)

#############################

add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)
add_subdirectory(src)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

find_package(Boost REQUIRED COMPONENTS program_options)
target_include_directories(${PROJECT_NAME} PRIVATE Boost::program_options)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost Boost::program_options)

find_package(VTK REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${VTK_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)