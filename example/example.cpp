#include "clar/config.h"

using namespace std;
using namespace clar;

int main(int argc, char* argv[]) {
    Config cfg(
        "example",
        "Example CLAR app"
    );

    NamedArg<int, true, 'f'> foo = { cfg, "foo", "FOO" };
    NamedOpt<uint32_t, 'b'> bar = { cfg, "bar", "BAR", 100500 };
    NamedOpt<vector<string>, 'w'> wat = { cfg, "wat", "WAT" };
    FreeArg<string, true> a = { cfg, "A", "AAA" };
    FreeArg<vector<uint32_t>> b = { cfg, "B", "BBB" };

    if (!cfg.Parse(argc, argv, cerr)) {
        cerr << endl;
        return 1;
    }

    cout << setw(4) << cfg.Get() << endl;

    return 0;
}
