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
    ASSERT_EQ(true, ok);
    EXPECT_EQ(false, !foo);
    EXPECT_EQ(true, foo.Get());
    EXPECT_EQ(true, !bar);
    EXPECT_EQ(true, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, IntegerRequiredSucces) {
    // Another way to organize config
    struct TestConfig: public Config {
        NamedArg<int, true, 'f'> Foo_ = { *this, "foo", "FOO" };
        NamedOpt<uint32_t, 'b'> Bar_ = { *this, "bar", "BAR", 100500 };
    };

    TestConfig cfg;
    ostringstream err;
    auto ok = cfg.Parse({ "-f", "-1", "-b1" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(-1, cfg.Foo_.Get());
    EXPECT_EQ(1u, cfg.Bar_.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, CharacterRequiredSucces) {
    // Another way to organize config
    struct TestConfig: public Config {
        NamedArg<char, true, 'f'> Foo_ = { *this, "foo", "FOO" };
        NamedOpt<unsigned, 'b'> Bar_ = { *this, "bar", "BAR", 255 };
        NamedOpt<vector<char>, 'w'> Wat_ { *this, "wat", "WAT" };
    };

    TestConfig cfg;
    ostringstream err;
    auto ok = cfg.Parse({ "-fa", "-b1", "-w1", "-w", "W" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ('a', cfg.Foo_.Get());
    EXPECT_EQ(1u, cfg.Bar_.Get());
    EXPECT_EQ(2u, cfg.Wat_.Get().size());
    EXPECT_EQ('1', cfg.Wat_.Get()[0]);
    EXPECT_EQ('W', cfg.Wat_.Get()[1]);

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, StringStackedSucces) {
    Config cfg;
    NamedArg<bool> foo(cfg, "f", "FOO");
    NamedArg<string> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fbWAT" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(true, foo.Get());
    EXPECT_EQ("WAT", bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, StringLastValueMissing) {
    Config cfg;
    NamedArg<string> foo(cfg, "f", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "-f" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Option 'f' shortcut 'f' required value is missing", err.str());
}

TEST(ShortArgs, MultipleStringStackedSucces) {
    Config cfg;
    NamedArg<vector<string>> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-bf", "-fb", "-f", "", "-fFOO", "-f", "BAR" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(true, bar.Get());
    EXPECT_EQ(4u, foo.Get().size());
    EXPECT_EQ("-fb", foo.Get()[0]);
    EXPECT_EQ("", foo.Get()[1]);
    EXPECT_EQ("FOO", foo.Get()[2]);
    EXPECT_EQ("BAR", foo.Get()[3]);

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, MultipleStringWithSpacesSucces) {
    struct TestConfig: public Config {
        NamedArg<int, true, 'f'> Foo_ = { *this, "foo", "FOO" };
        NamedOpt<uint32_t, 'b'> Bar_ = { *this, "bar", "BAR", 100500 };
        NamedOpt<vector<string>, 'w'> Wat_ = { *this, "wat", "WAT" };
    };

    TestConfig cfg;
    ostringstream err;
    auto ok = cfg.Parse({ "-f", "-1", "-b", "1", "-w", "WAT", "-w", "WAT THE" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(-1, cfg.Foo_.Get());
    EXPECT_EQ(1u, cfg.Bar_.Get());
    EXPECT_EQ(2u, cfg.Wat_.Get().size());
    EXPECT_EQ("WAT", cfg.Wat_.Get()[0]);
    EXPECT_EQ("WAT THE", cfg.Wat_.Get()[1]);
    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, IntegerRequiredFailure) {
    Config cfg;
    NamedArg<int> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-f", "-b" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    // TODO: fix message
    EXPECT_EQ("Option 'f': Failed to parse value: stoi", err.str());
}

TEST(ShortArgs, CharacterRequiredFailure) {
    Config cfg;
    NamedArg<char> foo(cfg, "f", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "-f", "foo" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Option 'f': Failed to parse value: Expected one character string", err.str());
}

TEST(ShortArgs, EqSeparatorFailures) {
    Config cfg("", "", "", cout, (_UnixFlavours & ~_ShortNoSep));
    NamedArg<int> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fb=100" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    // TODO: fix message
    EXPECT_EQ("Option 'f' shortcut 'f' is followed by unexpected symbol, value is expected", err.str());

    ostringstream().swap(err);
    ok = cfg.Parse({ "-b=WAT" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Option 'b' is a switch, value can not be specified", err.str());
}

TEST(ShortArgs, StackedSwitchesSuccess) {
    Config cfg;
    NamedArg<bool> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fb" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(true, foo.Get());
    EXPECT_EQ(true, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, ValueStringShortcutChars) {
    Config cfg;
    NamedOpt<bool, 'f'> foo(cfg, "foo", "FOO");
    FreeArg<string> bar(cfg, "bar", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "bf" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(true, !foo);
    EXPECT_EQ("bf", bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(ShortArgs, StackedUnknownArg) {
    Config cfg;
    NamedArg<bool> foo(cfg, "f", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "-fb" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Unknown argument '-fb' at position 1", err.str());
}

TEST(ShortArgs, AliasNameClash) {
    Config cfg;
    try {
        Switch<'f'> foo(cfg, "f", "FOO");
        FAIL();
    } catch (const exception& e) {
        //~ cerr << e.what() << endl;
        EXPECT_EQ("Option 'f': Failed to add alias: Alias is same as the option name", string(e.what()));
    } catch (...) {
        FAIL();
    }
}

TEST(ShortArgs, StackedValueNoSepError) {
    Config cfg;
    NamedArg<int> foo(cfg, "f", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "-fFOO" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Option 'f': Failed to parse value: stoi", err.str());
}

TEST(ShortArgs, BooleanDuplicateCall) {
    Config cfg;
    NamedArg<bool> foo(cfg, "f", "FOO");
    NamedArg<bool> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-fbf" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Option 'f' was specified multiple times", err.str());
}
