cc_test(
    name = "test",
    srcs = glob(["**/*.cpp"]),
    deps = [
        "@googletest//:gtest_main", 
        "//src:libsdb",
    ],
    data = [
        "//test/targets:reg_write",
    ]
)
