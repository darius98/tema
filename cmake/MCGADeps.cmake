include(FetchContent)

FetchContent_Declare(mcga_cli URL https://github.com/darius98/mcga-cli/archive/refs/heads/master.zip)
FetchContent_Populate(mcga_cli)
add_subdirectory(${mcga_cli_SOURCE_DIR} ${mcga_cli_BINARY_DIR} EXCLUDE_FROM_ALL)

FetchContent_Declare(mcga_matchers URL https://github.com/darius98/mcga-matchers/archive/refs/heads/master.zip)
FetchContent_Populate(mcga_matchers)
add_subdirectory(${mcga_matchers_SOURCE_DIR} ${mcga_matchers_BINARY_DIR} EXCLUDE_FROM_ALL)

FetchContent_Declare(mcga_test URL https://github.com/darius98/mcga-test/archive/refs/heads/master.zip)
FetchContent_Populate(mcga_test)
add_subdirectory(${mcga_test_SOURCE_DIR} ${mcga_test_BINARY_DIR} EXCLUDE_FROM_ALL)
