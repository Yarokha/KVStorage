find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(kv_storage_tests test.cpp)

target_link_libraries(kv_storage_tests
    PRIVATE
    kv_storage
    GTest::GTest
    GTest::Main
    Threads::Threads
)

enable_testing()
add_test(NAME kv_storage_tests COMMAND kv_storage_tests)