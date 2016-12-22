#include <fstream>
#include <sstream>

#include "actions.h"

using namespace std;

namespace clar {

ArgPtr CreateHelpAction(Config& config, string name, string info, ostream& out, bool testing) {
    return CreateActionSwitch(config, "help", "Print help and exit", [
        &config,
        name,
        info,
        &out,
        testing
    ] (const string&, ostream&) {
        out << name << ": " << info << endl;
        out << "Usage: " << name << " [options]";
        vector<const ArgBase*> req;
        vector<const ArgBase*> opt;
        for (auto arg: config.Args()) {
            if (arg->IsRequired()) {
                out << " ";
                if (arg->IsFree()) {
                    out << "<";
                } else {
                    out << "--";
                }
                out << arg->Name();
                if (arg->IsFree()) {
                    out << ">";
                } else if (!arg->IsSwitch()) {
                    out << " <>";
                }
                req.push_back(arg);
            } else {
                if (arg->IsFree()) {
                    out << " [" << arg->Name();
                    if (arg->IsMultiple()) {
                        out << "1 ... " << arg->Name() << "N";
                    }
                    out << "]";
                }
                opt.push_back(arg);
            }
        }
        out << endl;

        auto dump = [&out](const ArgBase* arg) {
            out << "  ";
            if (!arg->IsFree()) {
                out << "--";
            }
            out << arg->Name();
            if (!arg->IsFree() && !arg->IsSwitch()) {
                out << " <>";
            }
            out << "\t-- " << arg->Info() << endl;
        };
        out << endl << "Required arguments:" << endl;
        for (auto arg: req) {
            dump(arg);
        }
        out << endl << "Optional arguments:" << endl;
        for (auto arg: opt) {
            dump(arg);
        }
        if (!testing) {
            exit(0);
        }
        return true;
    });
}

ArgPtr CreateLoadAction(Config& config, string testData) {
    return CreateActionArg(config, "config", "Load config JSON from file", ArgBase::_Single, [&config, testData] (const string& val, ostream& err) {
        if (val.empty()) {
            err << "Empty config file name";
            return false;
        }
        ostringstream content;
        if (!testData.empty()) {
            content << testData;
        } else {
            ifstream file(val);
            if (!file) {
                err << "Failed to open config file: " << val;
                return false;
            }
            content << file.rdbuf();
        }
        json data;
        try {
            data = json::parse(content.str());
        } catch (const exception& e) {
            err << "Failed to parse JSON config: " << e.what();
            return false;
        }
        if (!config.Load(data, err)) {
            return false;
        }
        return true;
    });
}

} // namespace clar
