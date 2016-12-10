#include "gtest/gtest.h"
#include "clar.h"

using namespace std;
using namespace clar;

TEST(NamedArgs, BooleanRequiredSucces) {
    Config cfg;
    NamedArg<bool, true> foo(cfg, "foo", "FOO");
    NamedArg<bool> bar(cfg, "bar", "BAR", true);

    ostringstream err;
    auto ok = cfg.Parse({ "--foo" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(true, foo.Get());
    EXPECT_EQ(true, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(NamedArgs, IntegerRequiredSucces) {
    Config cfg;
    NamedArg<int, true> foo(cfg, "foo", "FOO");
    NamedArg<uint32_t> bar(cfg, "bar", "BAR", 100500);

    ostringstream err;
    auto ok = cfg.Parse({ "--foo", "-1", "--bar", "1" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(-1, foo.Get());
    EXPECT_EQ(1u, bar.Get());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(NamedArgs, IntegerRequiredFailure) {
    Config cfg;
    NamedArg<int, true> foo(cfg, "foo", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "--foo", "FOO" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    // TODO: fix message
    EXPECT_EQ("Argument '--foo' failed to parse value: stoi", err.str());
}

TEST(NamedArgs, IntegerArrayRequiredSucces) {
    Config cfg;
    NamedArg<vector<int>, true> foo(cfg, "foo", "FOO");
    NamedArg<vector<uint32_t>> bar(cfg, "bar", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "--foo", "-1", "--bar", "1", "--bar", "2", "--foo", "-2" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(2u, foo.Get().size());
    EXPECT_EQ(2u, bar.Get().size());
    EXPECT_EQ(-1, foo.Get()[0]);
    EXPECT_EQ(-2, foo.Get()[1]);
    EXPECT_EQ(1u, bar.Get()[0]);
    EXPECT_EQ(2u, bar.Get()[1]);

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(NamedArgs, BooleanUnknownArg) {
    Config cfg;
    NamedArg<bool> foo(cfg, "foo", "FOO");

    ostringstream err;
    auto ok = cfg.Parse({ "--bar" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Unknown argument '--bar' at position 1", err.str());
}

TEST(NamedArgs, BooleanRequiredMissing) {
    Config cfg;
    NamedArg<bool, true> foo(cfg, "foo", "FOO");
    NamedArg<bool> bar(cfg, "bar", "BAR", true);

    ostringstream err;
    auto ok = cfg.Parse({ "--bar" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Argument '--foo' is required and was not set", err.str());
}

TEST(NamedArgs, BooleanDuplicateName) {
    Config cfg;
    NamedArg<bool> foo(cfg, "foo", "FOO");
    try {
        NamedArg<bool> bar(cfg, "foo", "BAR");
        FAIL();
    } catch (const exception& e) {
        //~ cerr << e.what() << endl;
        EXPECT_EQ("Option 'foo' failed to add to config: Option name 'foo' is already used", string(e.what()));
    } catch (...) {
        FAIL();
    }
}

TEST(NamedArgs, BooleanAliasFailure) {
    Config cfg;
    NamedArg<bool> foo(cfg, "foo", "FOO");
    NamedArg<bool> bar(cfg, "bar", "BAR");
    try {
        foo.Long("bar");
        FAIL();
    } catch (const exception& e) {
        //~ cerr << e.what() << endl;
        EXPECT_EQ("Argument '--foo' failed to add alias: Alias 'bar' is already used by option 'bar'", string(e.what()));
    } catch (...) {
        FAIL();
    }
}

TEST(Config, LoadBooleanRequiredSuccess) {
    Config cfg;
    NamedArg<bool, true> foo(cfg, "foo", "FOO");
    NamedArg<bool> bar(cfg, "bar", "BAR", true);

    ostringstream err;
    auto ok = cfg.Load({ { "foo", true } }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(true, foo.Get());
    EXPECT_EQ(true, bar.Get());
}

TEST(Config, LoadBooleanRequiredFailure) {
    Config cfg;
    NamedArg<bool, true> foo(cfg, "foo", "FOO");
    NamedArg<bool> bar(cfg, "bar", "BAR", true);

    ostringstream err;
    auto ok = cfg.Load({ { "bar", true } }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Required option 'foo' was not specified in config", err.str());
}

TEST(Config, LoadBooleanUnknownOption) {
    Config cfg;
    NamedArg<bool> foo(cfg, "foo", "FOO");

    ostringstream err;
    auto ok = cfg.Load({ { "foo", true }, { "bar", true } }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Unknown option 'bar' was specified in config", err.str());
}

TEST(Config, LoadBooleanMultipleEntries) {
    Config cfg;
    NamedArg<bool> foo(cfg, "foo", "FOO");
    foo.Long("bar");

    ostringstream err;
    auto ok = cfg.Load({ { "foo", true }, { "bar", true } }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Option 'foo' or one of its aliases was specified in config mulitple times", err.str());
}
