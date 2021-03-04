cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_library(ad-block SHARED
        src/main/cpp/ad_block_client_wrap.cpp
        ../../ad_block_client.cc
        ../../context_domain.cc
        ../../cosmetic_filter.cc
        ../../filter.cc
        ../../filter_list.cc
        ../../no_fingerprint_domain.cc
        ../../protocol.cc
        ../../node_modules/bloom-filter-cpp/BloomFilter.cpp
        ../../node_modules/bloom-filter-cpp/hashFn.cpp
        ../../node_modules/hashset-cpp/hash_set.cc)

include_directories(${CMAKE_SOURCE_DIR}
        src/main/cpp/
        ../../
        ../../node_modules/bloom-filter-cpp/
        ../../node_modules/hashset-cpp/)

target_link_libraries(ad-block
        android
        log
        m)