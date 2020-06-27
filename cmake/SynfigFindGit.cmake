#[=============================================================[

Finds git and sets GIT_DATE, GIT_BRANCH and GIT_HASH
for future use.

#]=============================================================]

find_package(Git)

if (GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} show -s --format=%ad --date=format:%Y-%m-%d HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(GIT_DATE    "unknown")
    set(GIT_BRANCH  "unknown")
    set(GIT_HASH    "unknown")
endif()
