cc_library(
    name = "main",
    srcs = glob(
        [
            "googletest/include/gtest/internal/*/*.h",
            "googletest/include/gtest/internal/*.h",
            "googletest/src/*.cc",
            "googletest/src/*.h",
        ],
        exclude = ["googletest/src/gtest-all.cc"]
    ),
    hdrs = glob([
        "googletest/include/gtest/*.h",
    ]),
    copts = [
        "-Iexternal/gtest/googletest/",
        "-Iexternal/gtest/googletest/include/",
    ],
    linkopts = [
        "-pthread",
    ],
    visibility = ["//visibility:public"],
)
