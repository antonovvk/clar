#include <unordered_map>

#include "clar.h"

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
        for (auto arg: NamedArgs_) {
            size_t count = 0;
            for (auto name: arg->LongNames()) {
                count += src.count(name);
            }
            if (count == 0 && arg->IsRequired()) {
                err << "Required option '" << arg->Name() << "' was not specified in config";
                return false;
            }
            if (count > 1) {
                err << "Option '" << arg->Name() << "' or one of its aliases was specified in config mulitple times";
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
        unordered_map<string, size_t> count;
        size_t idx = 0;
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
                    if (!arg->Parse(Data_, val, err)) {
                        return false;
                    }
                    break;
                }
            }
            if (!inc) {
                err << "Unknown argument '" << args[idx] << "' at position " << idx + 1;
                return false;
            }
            idx += inc;
        }

        for (auto arg: NamedArgs_) {
            auto c = count.find(arg->Name());
            if (c == count.end()) {
                if (arg->IsRequired()) {
                    err << arg->ReportedName() << " is required and was not set";
                    return false;
                }
            } else if (c->second > 1) {
                if (arg->RequiresValue() != ArgBase::_Multiple) {
                    err << arg->ReportedName() << " was specified multiple times";
                    return false;
                }
            }
        }

        return true;
    }

    bool Alias(const string& arg, const string& alias, ostream& err) {
        auto it = ArgMap_.find(arg);
        if (it == ArgMap_.end()) {
            err << "Unknown option '" << arg << "'";
            return false;
        }
        auto res = ArgMap_.insert({ alias, it->second });
        if (!res.second) {
            err << "Alias '" << alias << "' is already used by option '" << res.first->second->Name() << "'";
            return false;
        }
        return true;
    }

    const nlohmann::json& Get() const {
        return Data_;
    }

    bool AddNamed(ArgBase* arg, ostream& err) {
        if (!arg) {
            err << "Null argument pointer";
            return false;
        }
        if (!(ArgMap_.insert({ arg->Name(), arg })).second) {
            err << "Option name '" << arg->Name() << "' is already used";
            return false;
        }
        NamedArgs_.push_back(arg);
        return true;
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
        if (res && arg.RequiresValue() != ArgBase::_None) {
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
    vector<ArgBase*> NamedArgs_;
    unordered_map<string, ArgBase*> ArgMap_;
};

Config::Config(uint64_t flavours)
    : Impl_(new Impl(flavours))
{
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

const nlohmann::json& Config::Get() const {
    return Impl_->Get();
}

bool Config::AddNamed(ArgBase* arg, ostream& err) {
    return Impl_->AddNamed(arg, err);
}

} // namespace clar
