#include "actions.h"

using namespace std;

namespace clar {

ArgPtr CreateHelpAction(Config& config, string name, string info, ostream& out, bool testing) {
    return CreateActionArg(config, "help", "Print help and exit", [&config, name, info, &out, testing]() {
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
    });
}

} // namespace clar
