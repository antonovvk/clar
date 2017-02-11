#include "gtest/gtest.h"
#include "clar/config.h"

using namespace std;
using namespace clar;

TEST(FreeArgs, StringRequiredSucces) {
    Config cfg;
    NamedArg<int, true> foo(cfg, "foo", "FOO");
    FreeArg<string, true> bar(cfg, "bar", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "--foo", "-1", "bar" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(-1, foo.Get());
    EXPECT_EQ("bar", bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(FreeArgs, StringRequiredFailure) {
    Config cfg;
    NamedArg<int, true> foo(cfg, "foo", "FOO");
    FreeArg<string, true> bar(cfg, "bar", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "--foo", "1" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    // TODO: fix message
    EXPECT_EQ("Option 'bar' is required and was not set", err.str());
}

TEST(FreeArgs, UndeclaredFreeArgFailure) {
    Config cfg;
    NamedArg<int, true> foo(cfg, "foo", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "--foo", "1", "BAR" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Unknown argument 'BAR' at position 3", err.str());
}

TEST(FreeArgs, IntegerArrayRequiredSucces) {
    Config cfg;
    NamedArg<vector<int>, true> foo(cfg, "foo", "FOO");
    FreeArg<vector<uint32_t>> bar(cfg, "bar", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "--foo", "-1", "1", "2", "--foo", "-2" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(2u, foo.Get().size());
    EXPECT_EQ(2u, bar.Get().size());
    EXPECT_EQ(-1, foo.Get()[0]);
    EXPECT_EQ(-2, foo.Get()[1]);
    EXPECT_EQ(1u, bar.Get()[0]);
    EXPECT_EQ(2u, bar.Get()[1]);

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(FreeArgs, ManySingularFreeArgs) {
    Config cfg;
    FreeArg<int, true> foo(cfg, "foo", "FOO");
    FreeArg<uint32_t> bar(cfg, "bar", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-1", "1" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(-1, foo.Get());
    EXPECT_EQ(1u, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(FreeArgs, OptionalFreeArgInTheMiddle) {
    Config cfg;
    FreeArg<int, true> foo(cfg, "foo", "FOO");
    FreeArg<uint32_t> bar(cfg, "bar", "BAR");
    FreeArg<int, true> jar("jar", "JAR");

    ostringstream err;
    auto ok = jar.Add(cfg, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Option 'jar' failed to add to config: Only the last free arg is allowed to be optional or accept multiple values", err.str());
}

TEST(FreeArgs, ManyMultipleFreeArgs) {
    Config cfg;
    FreeArg<vector<int>, true> foo(cfg, "foo", "FOO");
    FreeArg<uint32_t> bar("bar", "BAR");

    ostringstream err;
    auto ok = bar.Add(cfg, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Option 'bar' failed to add to config: Only the last free arg is allowed to be optional or accept multiple values", err.str());
}
