# cmake_minimum_required(VERSION 3.0.0)
# project(DUET_CODE VERSION 0.1.0)

# include(CTest)
# enable_testing()

# add_executable(DUET_CODE main.cpp)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

cmake_minimum_required(VERSION 3.0.0)
project(DUET_CODE VERSION 0.1.0)

IF(CMAKE_COMPILER_IS_GNUCXX)
    ADD_COMPILE_OPTIONS(-O3)  
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

include_directories("${PROJECT_BINARY_DIR}")
 
# Find all main*.cpp files and store in list mains
file(GLOB_RECURSE mains RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/main*.cpp")
foreach(mainfile IN LISTS mains)
    # Get file name without directory
    get_filename_component(mainname ${mainfile} NAME_WE)
    add_executable(${mainname} ${mainfile})
endforeach()