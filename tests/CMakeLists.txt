include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.0
)
FetchContent_MakeAvailable(unity)

target_compile_definitions(unity PUBLIC UNITY_INCLUDE_DOUBLE)

add_executable(test_geohex_static test_geohex.c)
target_link_libraries(test_geohex_static
    PRIVATE
    geohex_static
    unity
)
target_compile_definitions(test_geohex_static PRIVATE UNITY_INCLUDE_DOUBLE)

add_executable(test_geohex_shared test_geohex.c)
target_link_libraries(test_geohex_shared
    PRIVATE
    geohex_shared
    unity
)
target_compile_definitions(test_geohex_shared PRIVATE UNITY_INCLUDE_DOUBLE)

add_test(NAME test_geohex_static COMMAND test_geohex_static)
add_test(NAME test_geohex_shared COMMAND test_geohex_shared)

if(USE_VALGRIND AND VALGRIND)
    add_custom_target(memcheck_static
        COMMAND ${VALGRIND} --leak-check=full --error-exitcode=1 $<TARGET_FILE:test_geohex_static>
        DEPENDS test_geohex_static
    )
    add_custom_target(memcheck_shared
        COMMAND ${VALGRIND} --leak-check=full --error-exitcode=1 $<TARGET_FILE:test_geohex_shared>
        DEPENDS test_geohex_shared
    )
    add_custom_target(memcheck
        DEPENDS memcheck_static memcheck_shared
    )
endif()
