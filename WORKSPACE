new_git_repository(
    name = "gtest",
    remote = "https://github.com/google/googletest",
    tag = "release-1.8.0",
    build_file = "contrib/gtest.BUILD",
)

new_http_archive(
    name = "json",
    url = "https://github.com/nlohmann/json/releases/download/v3.1.2/include.zip",
    sha256 = "495362ee1b9d03d9526ba9ccf1b4a9c37691abe3a642ddbced13e5778c16660c",
    build_file = "contrib/json.BUILD",
    strip_prefix = "include",
)
