cmake_minimum_required(VERSION 3.10)
project(libgeohex VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(USE_VALGRIND "Enable Valgrind memory checks" OFF)

option(USE_ASAN "Enable AddressSanitizer" OFF)
option(USE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(USE_MSAN "Enable MemorySanitizer" OFF)
option(USE_COVERAGE "Enable code coverage with gcov" OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

add_library(geohex_static STATIC
    src/geohex.c
)

add_library(geohex_shared SHARED
    src/geohex.c
)

set_target_properties(geohex_static PROPERTIES OUTPUT_NAME geohex)
set_target_properties(geohex_shared PROPERTIES OUTPUT_NAME geohex)

target_include_directories(geohex_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(geohex_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(geohex_static PUBLIC m)
target_link_libraries(geohex_shared PUBLIC m)

if(USE_ASAN)
    set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(USE_UBSAN)
    set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=undefined")
endif()

if(USE_MSAN)
    set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=memory -fno-omit-frame-pointer")
endif()

if(USE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

if(SANITIZER_FLAGS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()

install(TARGETS geohex_static geohex_shared
    EXPORT geohexTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

find_program(VALGRIND "valgrind")
if(NOT VALGRIND)
    message(STATUS "Valgrind not found, memory check target will not be available")
endif()

if(USE_COVERAGE)
    find_program(GCOV "gcov")
    if(NOT GCOV)
        message(FATAL_ERROR "gcov not found, but code coverage is enabled")
    endif()

    find_program(LCOV "lcov")
    find_program(GENHTML "genhtml")
    if(LCOV AND GENHTML)
        add_custom_target(coverage
            COMMAND ${LCOV} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${GENHTML} coverage.info --output-directory coverage_report
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
    else()
        message(STATUS "lcov or genhtml not found, HTML coverage report generation will not be available")
    endif()
endif()
