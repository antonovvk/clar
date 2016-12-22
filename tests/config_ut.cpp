#include "gtest/gtest.h"
#include "clar/config.h"

using namespace std;
using namespace clar;

TEST(API, NonAddedArg) {
    NamedArg<bool> foo("foo", "FOO");
    try {
        foo.Get();
        FAIL();
    } catch (const exception& e) {
        //~ cerr << e.what() << endl;
        EXPECT_EQ("Option 'foo' wasn't added to config", string(e.what()));
    }
}

TEST(API, DirectCast) {
    Config cfg;
    NamedArg<int, true> foo(cfg, "foo", "FOO");
    NamedArg<uint32_t> bar(cfg, "bar", "BAR", 100500);
    NamedArg<vector<string>> wat(cfg, "wat", "WAT");

    ostringstream err;
    auto ok = cfg.Parse({ "--foo", "-1", "--bar", "1", "--wat",  "A", "--wat", "B" }, err);
    //~ cerr << err.str() << endl;

    auto getInt = [](int val) { return val; };
    auto getUint = [](uint32_t val) { return val; };
    auto getVector = [](vector<string> vec) { return vec; };

    EXPECT_EQ(true, ok);
    EXPECT_EQ(-1, getInt(foo));
    EXPECT_EQ(1u, getUint(bar));
    EXPECT_EQ(2u, getVector(wat).size());
    EXPECT_EQ("A", getVector(wat)[0]);
    EXPECT_EQ("B", getVector(wat)[1]);

    //~ cerr << setw(4) << cfg.Get() << endl;
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

TEST(Config, LoadOverride) {
    Config cfg;
    NamedArg<int, true> foo(cfg, "foo", "FOO");
    NamedArg<vector<int>> bar(cfg, "bar", "BAR");

    ostringstream err;
    auto ok = cfg.Load({ { "foo", 1 }, { "bar", { 2, 3, 4 } } }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(1, foo.Get());
    EXPECT_EQ(3u, bar.Get().size());

    ok = cfg.Parse({ "--foo", "2", "--bar", "5", "--bar", "6" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(2, foo.Get());
    EXPECT_EQ(2u, bar.Get().size());
    EXPECT_EQ(5, bar.Get()[0]);
    EXPECT_EQ(6, bar.Get()[1]);
}

TEST(Config, LoadBooleanRequiredFailure) {
    Config cfg;
    NamedArg<bool, true> foo(cfg, "foo", "FOO");
    NamedArg<bool> bar(cfg, "bar", "BAR", true);

    ostringstream err;
    auto ok = cfg.Load({ { "bar", true } }, err);
    EXPECT_EQ(true, ok);

    ok = cfg.Parse({}, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Option 'foo' is required and was not set", err.str());

    ok = cfg.Parse({ "--foo" }, err);
    EXPECT_EQ(true, ok);
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
    foo.Alias("bar");

    ostringstream err;
    auto ok = cfg.Load({ { "foo", true }, { "bar", true } }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Option 'foo' or one of its aliases was specified in config mulitple times", err.str());
}

TEST(Config, LoadJsonFields) {
    Config cfg;
    NamedArg<json> foo(cfg, "foo", "FOO");
    FreeArg<json> bar(cfg, "bar", "BAR");

    json data = {
        { "foo", { { "A", 1 }, { "B", "C" } } },
        { "bar", { { "E", { 1, 2, 3 } } } }
    };

    ostringstream err;
    auto ok = cfg.Load(data, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(data.dump(), cfg.Get().dump());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(Config, LoadJsonAndIntArray) {
    Config cfg;
    NamedArg<json> foo(cfg, "foo", "FOO");
    FreeArg<vector<int>> bar(cfg, "bar", "BAR");

    json data = {
        { "foo", { { "A", 1 }, { "B", "C" } } },
        { "bar", { 1, 2, 3 } }
    };

    ostringstream err;
    auto ok = cfg.Load(data, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(3u, bar.Get().size());
    EXPECT_EQ(1, bar.Get()[0]);
    EXPECT_EQ(2, bar.Get()[1]);
    EXPECT_EQ(3, bar.Get()[2]);
    EXPECT_EQ(data.dump(), cfg.Get().dump());

    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(Config, LoadIntArrayFailure) {
    Config cfg;
    FreeArg<vector<int>> foo(cfg, "foo", "FOO");

    ostringstream err;
    auto ok = cfg.Load({ { "foo", { "A", "B", "C" } } }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(false, ok);
    EXPECT_EQ("Option 'foo': Expected signed integer array in config", err.str());
}

TEST(ActionArgs, BasicHelp) {
    ostringstream out;
    Config cfg("test", "This is HELP test", out, _UnixFlavours, { { "test-help", true } });
    NamedArg<int, true> foo(cfg, "foo", "FOO");
    FreeArg<string, true> bar(cfg, "bar", "BAR");
    FreeArg<vector<string>> jar(cfg, "jar", "JAR");
    NamedArg<int> wat(cfg, "wat", "WAT");

    ostringstream err;
    cfg.Parse({ "--help" }, err);
    //~ cerr << out.str() << endl;
    //~ cerr << err.str() << endl;

    EXPECT_EQ(
        "test: This is HELP test\n"
        "Usage: test [options] --foo <> <bar> [jar1 ... jarN]\n"
        "\n"
        "Required arguments:\n"
        "  --foo <>\t-- FOO\n"
        "  bar\t-- BAR\n"
        "\n"
        "Optional arguments:\n"
        "  --help\t-- Print help and exit\n"
        "  --config <>\t-- Load config JSON from file\n"
        "  --wat <>\t-- WAT\n"
        "  jar\t-- JAR\n",
        out.str()
    );
}

TEST(ActionArgs, BasicConfig) {
    json data = {
        { "foo", 1 },
        { "bar", "A" },
        { "jar", { "B", "C" } },
        { "wat", 2 },
    };

    ostringstream out;
    Config cfg("test", "This is HELP test", out, _UnixFlavours, { { "test-load", data.dump() } });
    NamedArg<int, true> foo(cfg, "foo", "FOO");
    FreeArg<string, true> bar(cfg, "bar", "BAR");
    FreeArg<vector<string>> jar(cfg, "jar", "JAR");
    NamedArg<int> wat(cfg, "wat", "WAT");

    ostringstream err;
    auto ok = cfg.Parse({ "--config", "test" }, err);
    //~ cerr << err.str() << endl;
    EXPECT_EQ(true, ok);
    EXPECT_EQ(true, (data == cfg.Get()));

    //~ cerr << setw(4) << cfg.Get() << endl;
}
