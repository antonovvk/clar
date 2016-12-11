#include <unordered_map>

#include "clar.h"
#include "actions.h"

using namespace std;

namespace clar {

class Config::Impl {
public:
    Impl(uint64_t flavours)
        : Flavours_(flavours)
    {
    }

    bool Load(const nlohmann::json& src, ostream& err) {
        if (!src.is_object()) {
            err << "Expecting json object as config source";
            return false;
        }
        for (auto it = src.begin(); it != src.end(); ++it) {
            auto arg = ArgMap_.find(it.key());
            if (arg == ArgMap_.end()) {
                err << "Unknown option '" << it.key() << "' was specified in config";
                return false;
            }
            if (!arg->second->Check(it.value(), err)) {
                return false;
            }
        }

        for (auto arg: Args()) {
            size_t c = 0;
            for (auto name: arg->LongNames()) {
                c += src.count(name);
            }
            if (c > 1) {
                err << arg->ReportedName() << " or one of its aliases was specified in config mulitple times";
                return false;
            }
        }
        Data_ = src;
        return true;
    }

    bool Parse(int argc, char* argv[], ostream& err) {
        return false;
    }

    bool Parse(const vector<string>& args, ostream& err) {
        auto data = Data_;

        unordered_map<string, size_t> count;
        size_t idx = 0;
        size_t pos = 0;
        while (idx < args.size()) {
            size_t inc = 0;
            for (auto arg: NamedArgs_) {
                string val;
                inc = Match(*arg, val, idx, args);
                if (inc) {
                    if (idx + inc > args.size()) {
                        err << arg->ReportedName() << " requires value at position " << idx + 1;
                        return false;
                    }
                    ++count[arg->Name()];
                    if (!arg->Parse(data, val, err)) {
                        return false;
                    }
                    break;
                }
            }
            if (!inc && pos < FreeArgs_.size()) {
                auto arg = FreeArgs_[pos];
                ++count[arg->Name()];
                if (!arg->Parse(data, args[idx], err)) {
                    return false;
                }
                if (!arg->IsMultiple()) {
                    ++pos;
                }
                inc = 1;
            }
            if (!inc) {
                err << "Unknown argument '" << args[idx] << "' at position " << idx + 1;
                return false;
            }
            idx += inc;
        }

        for (auto arg: Args()) {
            size_t c = data.count(arg->Name());
            auto it = count.find(arg->Name());
            if (it != count.end()) {
                if (it->second > 1 && !arg->IsMultiple()) {
                    err << arg->ReportedName() << " was specified multiple times";
                    return false;
                }
                c += it->second;
            }
            if (arg->IsRequired() && !c) {
                err << arg->ReportedName() << " is required and was not set";
                return false;
            }
        }

        Data_ = data;
        return true;
    }

    bool Alias(const string& arg, const string& alias, ostream& err) {
        auto info = "Option '" + arg + "': Failed to add alias: ";
        auto it = ArgMap_.find(arg);
        if (it == ArgMap_.end()) {
            err << info << "Unknown option '" << arg << "'";
            return false;
        }
        auto res = ArgMap_.insert({ alias, it->second });
        if (!res.second) {
            err << info << "Alias '" << alias << "' is already used by option '" << res.first->second->Name() << "'";
            return false;
        }
        return true;
    }

    vector<const ArgBase*> Args() const {
        auto args = NamedArgs_;
        args.insert(args.end(), FreeArgs_.begin(), FreeArgs_.end());
        return args;
    }

    const nlohmann::json& Get() const {
        return Data_;
    }

    bool Add(const ArgBase* arg, ostream& err) {
        if (!arg) {
            throw domain_error("Config::Impl::Add(): Null argument pointer");
        }

        auto info = arg->ReportedName() + " failed to add to config: ";
        if (!(ArgMap_.insert({ arg->Name(), arg })).second) {
            err << info << "Option name '" << arg->Name() << "' is already used";
            return false;
        }

        if (arg->IsFree()) {
            if (!FreeArgs_.empty() && FreeArgs_.back()->IsMultiple()) {
                err << info << "Only the last free arg is allowed to accept multiple values";
                return false;
            }
            FreeArgs_.push_back(arg);
        } else {
            NamedArgs_.push_back(arg);
        }
        return true;
    }

    void Hold(ArgPtr arg) {
        HeldArgs_.emplace_back(move(arg));
    }

private:
    size_t Match(const ArgBase& arg, string& val, size_t idx, const vector<string>& args) const {
        size_t res = 0;
        for (auto& name: arg.LongNames()) {
            if ((Flavours_ & _LongDoubleDash) && "--" + name == args[idx]) {
                res = 1;
                break;
            }
        }
        if (res && !arg.IsSwitch()) {
            if (Flavours_ & _LongSpaceSep) {
                if (idx + 1 < args.size()) {
                    val = args[idx + 1];
                }
                ++res;
            }
        }
        return res;
    }

private:
    const uint64_t Flavours_;
    nlohmann::json Data_;
    vector<ArgPtr> HeldArgs_;
    vector<const ArgBase*> NamedArgs_;
    vector<const ArgBase*> FreeArgs_;
    unordered_map<string, const ArgBase*> ArgMap_;
};

Config::Config(string name, string info, ostream& infoOutput, uint64_t flavours)
    : Impl_(new Impl(flavours))
{
    if (flavours & _HelpAction) {
        Impl_->Hold(CreateHelpAction(*this, name, info, infoOutput, flavours & _DoNotExitOnHelp));
    }
}

Config::~Config() = default;

bool Config::Load(const nlohmann::json& src, ostream& err) {
    return Impl_->Load(src, err);
}

bool Config::Parse(int argc, char* argv[], ostream& err) {
    return Impl_->Parse(argc, argv, err);
}

bool Config::Parse(const vector<string>& args, ostream& err) {
    return Impl_->Parse(args, err);
}

bool Config::Alias(const string& arg, const string& alias, ostream& err) {
    return Impl_->Alias(arg, alias, err);
}

vector<const ArgBase*> Config::Args() const {
    return Impl_->Args();
}

const nlohmann::json& Config::Get() const {
    return Impl_->Get();
}

bool Config::Add(const ArgBase* arg, ostream& err) {
    return Impl_->Add(arg, err);
}

} // namespace clar
