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
        "clar.cpp",
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

cc_test(
    name = "clar_ut",
    srcs = ["clar_ut.cpp"],
    copts = [
        "-Iexternal/gtest/googletest/include",
    ],
    deps = [
        "@gtest//:main",
        ":clar",
    ],
    linkopts = [
        "-lm",
    ],
)
