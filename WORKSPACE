load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")


# ----------------------------
# libedit
# ----------------------------
http_archive(
    name = "libedit",
    urls = ["https://thrysoee.dk/editline/libedit-20210910-3.1.tar.gz"],  # update version if needed
    strip_prefix = "libedit-20210910-3.1",
    build_file_content = """
cc_library(
    name = "libedit",
    srcs = glob(["*.c"]),
    hdrs = glob(["*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)
""",
)

# ----------------------------
# Google Test
# ----------------------------
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "gtest",
    sha256 = "53de8c75150430c217550ec6bb413029300120407f2de02ea8e20e89675f5e94",
    strip_prefix = "googletest-912db742531bf82efb01194bc08140416e3b3467",
    urls = [
        "https://github.com/google/googletest/archive/912db742531bf82efb01194bc08140416e3b3467.tar.gz",
    ],
)