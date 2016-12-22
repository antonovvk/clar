#include "gtest/gtest.h"
#include "clar/config.h"

using namespace std;
using namespace clar;

TEST(ShortArgs, BooleanRequiredSucces) {
    Config cfg;
    NamedArg<bool, true> foo(cfg, "f", "FOO");
    Switch<'b'> bar(cfg, "bar", "BAR", true);

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
    NamedArg<int, true, 'f'> foo(cfg, "foo", "FOO");
    NamedOpt<uint32_t, 'b'> bar(cfg, "bar", "BAR", 100500);

    ostringstream err;
    auto ok = cfg.Parse({ "-f", "-1", "-b1" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(-1, foo.Get());
    EXPECT_EQ(1u, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, StringStackedSucces) {
    Config cfg;
    NamedArg<bool> foo(cfg, "f", "FOO");
    NamedArg<string> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fbWAT" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(true, foo.Get());
    EXPECT_EQ("WAT", bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, MultipleStringStackedSucces) {
    Config cfg;
    NamedArg<vector<string>> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-bf", "-fb", "-f", "", "-fFOO", "-f", "BAR", "-f" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(true, bar.Get());
    EXPECT_EQ(5u, foo.Get().size());
    EXPECT_EQ("-fb", foo.Get()[0]);
    EXPECT_EQ("", foo.Get()[1]);
    EXPECT_EQ("FOO", foo.Get()[2]);
    EXPECT_EQ("BAR", foo.Get()[3]);
    EXPECT_EQ("", foo.Get()[4]);

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, IntegerRequiredFailure) {
    Config cfg;
    NamedArg<int> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-f", "-b" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    // TODO: fix message
    EXPECT_EQ("Option 'f' failed to parse value: stoi", err.str());
}

TEST(ShortArgs, EqSeparatorFailures) {
    Config cfg("", "", cout, (_UnixFlavours & ~_ShortNoSep) | _EqualsSep);
    NamedArg<int> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fb=100" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    // TODO: fix message
    EXPECT_EQ("Option 'f' shortcut 'f' is followed by unexpected symbol, value is expected", err.str());

    ostringstream().swap(err);
    ok = cfg.Parse({ "-b=WAT" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Option 'b' is a switch, value can not be specified", err.str());
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
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fbf" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Option 'f' was specified multiple times", err.str());
}
