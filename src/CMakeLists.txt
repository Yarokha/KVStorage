add_library(kv_storage)

target_sources(kv_storage
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/kv_storage.h
)

target_include_directories(kv_storage
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
