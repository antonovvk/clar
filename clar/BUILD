cc_library(
    name = "clar",
    hdrs = [
        "clar.h",
    ],
    srcs = [
        "arg_base.h",
        "arg_base.cpp",
        "arg_impl.h",
        "arg_impl.cpp",
        "actions.h",
        "actions.cpp",
        "file.h",
        "config.h",
        "config.cpp",
    ],
    copts = [
        "-std=c++11",
        "-Wall",
    ],
    linkopts = [
        "-lstdc++",
    ],
    deps = [
        "@json//:json",
    ],
    visibility = ["//visibility:public"],
)
