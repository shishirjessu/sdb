cc_library(
    name = "test",
    hdrs = ["TestUtil.hpp", "BreakpointSetTestHelpers.hpp"],
    srcs = glob(["BreakpointSetTestHelpers.cpp"]),
    deps = ["//src:libsdb"],
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "sdb_test",
    srcs = glob(["**/*.cpp"]),
    deps = [
        "@googletest//:gtest_main", 
        "//src:libsdb",
        "//test",
    ],
    data = [
        "//test/targets:reg_write",
        "//test/targets:reg_read",
        "//test/targets:run_forever",
        "//test/targets:hello_sdb",
        "//test/targets:memory",
    ]
)
