#include "gtest/gtest.h"
#include "clar/config.h"

using namespace std;
using namespace clar;

TEST(EqualsSep, EqOrSpaceSucces) {
    struct TestConfig: public ConfigWith<_UnixFlavours | _EqualsSep> {
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
