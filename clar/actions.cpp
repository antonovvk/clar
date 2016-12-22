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
                    out << " " << arg->Meta() << ">";
                } else if (!arg->IsSwitch()) {
                    out << " <" << arg->Meta() << ">";
                }
                req.push_back(arg);
            } else {
                if (arg->IsFree()) {
                    out << " [" << arg->Name() << " " << arg->Meta();
                    if (arg->IsMultiple()) {
                        out << " 1 ... " << arg->Name() << " " << arg->Meta() << " N";
                    }
                    out << "]";
                }
                opt.push_back(arg);
            }
        }
        out << endl;

        auto dump = [](ostream& out, const ArgBase* arg, size_t maxLen) {
            size_t start = out.tellp();
            out << "  ";
            if (arg->IsFree()) {
                out << (arg->IsRequired() ? "<" : "[");
            } else {
                for (auto n: arg->Names()) {
                    if (n.size() == 1) {
                        out << "-" << n << ", ";
                    }
                }
                for (auto n: arg->Names()) {
                    if (n.size() != 1 && n != arg->Name()) {
                        out << "--" << n << ", ";
                    }
                }
                out << "--";
            }
            out << arg->Name();
            if (arg->IsFree()) {
                out << " " << arg->Meta();
                out << (arg->IsRequired() ? ">" : "]");
            } else if (!arg->IsSwitch()) {
                out << " <" << arg->Meta() << ">";
            }
            size_t finish = out.tellp();
            if (maxLen) {
                out << string(maxLen - (finish - start), ' ');
            }
            out << " -- ";
            if (arg->IsMultiple()) {
                out << "(multiple) ";
            }
            out << arg->Info() << endl;
            return finish - start;
        };

        size_t maxLen = 0;
        for (auto arg: config.Args()) {
            stringstream tmp;
            maxLen = max(maxLen, dump(tmp, arg, 0));
        }

        ostringstream o;
        o << endl << "Required arguments:" << endl;
        for (auto arg: req) {
            dump(o, arg, maxLen);
        }
        o << endl << "Optional arguments:" << endl;
        for (auto arg: opt) {
            dump(o, arg, maxLen);
        }
        out << o.str();
        if (!testing) {
            exit(0);
        }
        return true;
    });
}

ArgPtr CreateLoadAction(Config& config, string testData) {
    auto a = CreateActionArg(config, "config", "Load config JSON from file", ArgBase::_Single, [&config, testData] (const string& val, ostream& err) {
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
    a->Meta("file name");
    return a;
}

} // namespace clar
