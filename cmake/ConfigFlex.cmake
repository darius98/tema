# On macOS, search Homebrew for keg-only versions of Flex. Xcode does
# not provide new enough versions for us to use.
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
            COMMAND brew --prefix flex
            RESULT_VARIABLE BREW_FLEX
            OUTPUT_VARIABLE BREW_FLEX_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
        message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
        set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
        include_directories(${BREW_FLEX_PREFIX}/include)
        link_directories(${BREW_FLEX_PREFIX}/lib)
    endif ()
endif ()

find_package(FLEX REQUIRED)

function(AddFlexLibrary NAME FLEX_FILE)
    flex_target(${NAME}_config compiler/flex_lexer.l ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.flex.cpp)
    add_library(${NAME} ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.flex.cpp)
endfunction()