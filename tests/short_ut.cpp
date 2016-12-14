#include "gtest/gtest.h"
#include "clar/config.h"

using namespace std;
using namespace clar;

TEST(ShortArgs, BooleanRequiredSucces) {
    Config cfg;
    NamedArg<bool, true> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR", true);

    ostringstream err;
    auto ok = cfg.Parse({ "-f" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(true, foo.Get());
    EXPECT_EQ(true, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, IntegerRequiredSucces) {
    Config cfg;
    NamedArg<int, true> foo(cfg, "f", "FOO");
    NamedArg<uint32_t> bar(cfg, "b", "BAR", 100500);

    ostringstream err;
    auto ok = cfg.Parse({ "-f", "-1", "-b", "1" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(-1, foo.Get());
    EXPECT_EQ(1u, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, IntegerRequiredFailure) {
    Config cfg;
    NamedArg<int> foo(cfg, "f", "FOO");
    NamedArg<int, true> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-f", "1" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    // TODO: fix message
    EXPECT_EQ("Option 'f' requires value", err.str());
}

TEST(ShortArgs, StackedSwitchesSuccess) {
    Config cfg;
    NamedArg<bool> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fb" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(true, foo.Get());
    EXPECT_EQ(true, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, StackedUnknownArg) {
    Config cfg;
    NamedArg<bool> foo(cfg, "f", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "-fb" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Unknown argument '-fb' at position 1", err.str());
}

TEST(ShortArgs, StackedValueNoSepError) {
    Config cfg;
    NamedArg<int> foo(cfg, "f", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "-fFOO" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Option 'f' failed to parse value: stoi", err.str());
}

TEST(ShortArgs, BooleanDuplicateCall) {
    Config cfg;
    NamedArg<bool> foo(cfg, "f", "FOO");
    NamedArg<bool> bar("b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fbf" }, err);
    cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    //~ EXPECT_EQ("Option 'foo' failed to add to config: Option name 'foo' is already used", err.str());
}
