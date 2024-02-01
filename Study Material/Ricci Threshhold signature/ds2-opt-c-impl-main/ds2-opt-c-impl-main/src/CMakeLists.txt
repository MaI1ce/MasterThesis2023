add_library(ds2 STATIC
    verify.c
    sign.c
    gen.c
    party.c
    commit.c
    poly.c
    ntt.c
    reduce.c
    fips202.c
    rand.c
    socket.c
    util.c
    benchmark.c)

target_include_directories(ds2 PUBLIC ${CMAKE_SOURCE_DIR}/include)

if (IS_IOS)
else ()
    add_executable(test_file test_file.c)
    target_link_libraries(test_file PRIVATE ds2)

    add_executable(test_network test_network.c)
    target_link_libraries(test_network PRIVATE ds2)
endif (IS_IOS)