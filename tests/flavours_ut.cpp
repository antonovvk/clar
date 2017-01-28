#include "gtest/gtest.h"
#include "clar/config.h"

using namespace std;
using namespace clar;

TEST(Separators, EqOrSpaceSucces) {
    struct TestConfig: public Config {
        NamedArg<int, true, 'f'> Foo_ = { *this, "foo", "FOO" };
        NamedOpt<uint32_t, 'b'> Bar_ = { *this, "bar", "BAR", 100500 };
        NamedOpt<vector<string>, 'w'> Wat_ = { *this, "wat", "WAT" };
    };

    TestConfig cfg;
    ostringstream err;
    auto ok = cfg.Parse({ "-f", "-1", "-b=1", "-w", "WAT IS", "-w=WAT" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ(-1, cfg.Foo_.Get());
    EXPECT_EQ(1u, cfg.Bar_.Get());
    EXPECT_EQ(2u, cfg.Wat_.Get().size());
    EXPECT_EQ("WAT IS", cfg.Wat_.Get()[0]);
    EXPECT_EQ("WAT", cfg.Wat_.Get()[1]);
    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(Separators, EqSeparatorSpaceInStr) {
    Config cfg;
    NamedArg<string> foo(cfg, "f", "FOO");
    FreeArg<string, false> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-f=FOO BAR", "JAR" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(true, ok);
    EXPECT_EQ("FOO BAR",foo.Get());
    EXPECT_EQ("JAR", bar.Get());
    //~ cerr << setw(4) << cfg.Get() << endl;
}

TEST(Separators, EqSeparatorFailures) {
    Config cfg("", "", "", cout, (_UnixFlavours & ~(_EqualsSep | _ShortNoSep)));
    NamedArg<int> foo(cfg, "f", "FOO");
    Switch<> bar(cfg, "b", "BAR");

    ostringstream err;
    auto ok = cfg.Parse({ "-bf=100" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    // TODO: fix message
    EXPECT_EQ("Option 'f' shortcut 'f' is followed by unexpected symbol, value is expected", err.str());

    ostringstream().swap(err);
    ok = cfg.Parse({ "-b=WAT" }, err);
    //~ cerr << err.str() << endl;
    ASSERT_EQ(false, ok);
    EXPECT_EQ("Unknown argument '-b=WAT' at position 1", err.str());
}
