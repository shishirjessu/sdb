cc_test(
    name = "test",
    srcs = glob(["**/*.cpp"]),
    deps = [
        "@googletest//:gtest_main", 
        "//src:libsdb",
    ],
    data = [
        "//test/targets:reg_write",
        "//test/targets:reg_read",
        "//test/targets:run_forever",
    ]
)
