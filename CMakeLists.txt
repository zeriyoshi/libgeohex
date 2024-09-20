cmake_minimum_required(VERSION 3.10)
project(libgeohex VERSION 1.1.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)

option(USE_ASAN "Enable AddressSanitizer" OFF)
option(USE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(USE_MSAN "Enable MemorySanitizer" OFF)
option(USE_COVERAGE "Enable code coverage" OFF)

option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

set(GEOHEX_TARGETS)

if (BUILD_STATIC_LIBS)
    add_library(geohex_static STATIC
        src/geohex.c
    )

    set_target_properties(geohex_static PROPERTIES OUTPUT_NAME geohex)

    target_include_directories(geohex_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(geohex_static PUBLIC m)

    list(APPEND GEOHEX_TARGETS geohex_static)
endif()

if(BUILD_SHARED_LIBS)
    add_library(geohex_shared SHARED
        src/geohex.c
    )

    set_target_properties(geohex_shared PROPERTIES OUTPUT_NAME geohex)

    target_include_directories(geohex_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(geohex_shared PUBLIC m)

    list(APPEND GEOHEX_TARGETS geohex_shared)
endif()

if(USE_ASAN)
    set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(USE_UBSAN)
    set(SANITIZER_FLAGS "${SANITIZER_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
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

install(TARGETS ${GEOHEX_TARGETS}
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
