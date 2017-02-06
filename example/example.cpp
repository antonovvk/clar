#include "clar/config.h"
#include "clar/file.h"

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
    OutputFileOpt<> cfgOut = { cfg, "cfg-out", "Save startup config in file", "-" };
    InputFileOpt<> inFile = { cfg, "input-file", "Read file" };

    if (!cfg.Parse(argc, argv, cerr)) {
        cerr << endl;
        return 1;
    }

    auto out = cfgOut.Open();
    if (!out) {
        cerr << "Failed to write config to '" << cfgOut.Get() << "'" << endl;
        return 1;
    }

    *out << setw(4) << cfg.Get() << endl;

    if (!!inFile) {
        auto in = inFile.Open();
        if (!in) {
            cerr << "Failed to open file: '" << inFile.Get() << "'" << endl;
            return 1;
        }
        string line;
        while (getline(*in, line)) {
            cout << ">\t" << line << endl;
        }
    }

    return 0;
}
