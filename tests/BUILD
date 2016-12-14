cc_test(
    name = "ut",
    srcs = [
        "named_ut.cpp",
        "free_ut.cpp",
        "short_ut.cpp",
        "config_ut.cpp",
    ],
    copts = [
        "-Iexternal/gtest/googletest/include",
    ],
    deps = [
        "@gtest//:main",
        "//clar",
    ],
    linkopts = [
        "-lm",
    ],
)
