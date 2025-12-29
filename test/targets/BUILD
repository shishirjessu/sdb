cc_binary(
    name = "reg_write",
    srcs = ["reg_write.S"],
    linkopts = ["-no-pie", "-fuse-ld=bfd"],
    visibility = ["//visibility:public"],
)


cc_binary(
    name = "reg_read",
    srcs = ["reg_read.S"],
    linkopts = ["-no-pie", "-fuse-ld=bfd"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "run_forever",
    srcs = ["run_forever.cpp"],
    visibility = ["//visibility:public"],
)