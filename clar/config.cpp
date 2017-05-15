#include <unordered_map>

#include "config.h"
#include "actions.h"

using namespace std;

namespace clar {

class Config::Impl {
public:
    Impl(uint64_t flavours)
        : Flavours_(flavours)
    {
    }

    bool Load(const json& src, ostream& err) {
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
            for (auto name: arg->Names()) {
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
        vector<string> args;
        for (int i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        return Parse(args, err);
    }

    bool Parse(const vector<string>& args, ostream& err) {
        json data;
        unordered_map<string, size_t> count;

        auto matched = [&data, &count, &err](const ArgBase* arg, const string& val) {
            if (arg->IsSwitch()) {
                if (!val.empty()) {
                    err << arg->ReportedName() << " is a switch, value can not be specified";
                    return false;
                }
            }
            ++count[arg->Name()];
            if (!arg->Parse(data, val, err)) {
                return false;
            }
            return true;
        };

        size_t idx = 0;
        size_t pos = 0;
        while (idx < args.size()) {
            string val;
            bool haveVal = false;
            auto a = args[idx];
            if (Flavours_ & _EqualsSep) {
                auto p = a.find('=');
                if (p != string::npos) {
                    val = a.substr(p + 1);
                    haveVal = true;
                }
                a = a.substr(0, p);
            }

            size_t inc = 0;
            for (auto arg: NamedArgs_) {
                if (MatchLong(*arg, a)) {
                    ++inc;
                    if (!arg->IsSwitch() && (Flavours_ & _SpaceSep) && !haveVal) {
                        if (idx + 1 >= args.size()) {
                            err << arg->ReportedName() << " required value is missing";
                            return false;
                        }
                        ++inc;
                        val = args[idx + 1];
                        haveVal = true;
                    }
                    if (!matched(arg, val)) {
                        return false;
                    }
                    break;
                }
            }

            bool checkShort = false;
            if ((Flavours_ & _ShortDash) && !a.empty() && a[0] == '-') {
                a = a.substr(1);
                checkShort = true;
            }
            for (size_t sym = 0; checkShort && !inc && sym < a.size(); ++sym) {
                bool match = false;
                for (auto arg: NamedArgs_) {
                    if (MatchShort(sym, *arg, a)) {
                        match = true;
                        if (arg->IsSwitch()) {
                            if (sym + 1 == a.size()) {
                                ++inc;
                            } else if (!(Flavours_ & _ShortStacked)) {
                                err << arg->ReportedName() << " shortcut '" << a[sym] << "' is followed by unexpected symbol";
                                return false;
                            }
                        } else {
                            ++inc;
                            if (sym + 1 == a.size()) {
                                if (Flavours_ & _SpaceSep) {
                                    if (!haveVal) {
                                        if (idx + 1 >= args.size()) {
                                            err << arg->ReportedName() << " shortcut '" << a[sym] << "' required value is missing";
                                            return false;
                                        }
                                        ++inc;
                                        val = args[idx + 1];
                                        haveVal = true;
                                    }
                                }
                            } else if (Flavours_ & _ShortNoSep) {
                                val = a.substr(sym + 1);
                            } else {
                                err << arg->ReportedName() << " shortcut '" << a[sym] << "' is followed by unexpected symbol, value is expected";
                                return false;
                            }
                        }
                        if (!matched(arg, val)) {
                            return false;
                        }
                        break;
                    }
                }
                if (!match) {
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
        auto tmp = Data_;
        for (auto it = data.begin(); it != data.end(); ++it) {
            tmp[it.key()] = it.value();
        }

        for (auto arg: Args()) {
            size_t c = tmp.count(arg->Name());
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
        Data_ = tmp;
        return true;
    }

    bool Alias(const string& arg, const string& alias, ostream& err) {
        auto info = "Option '" + arg + "': Failed to add alias: ";
        if (alias == arg) {
            err << info << "Alias is same as the option name";
            return false;
        }

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

    const json& Get() const {
        return Data_;
    }

    void Save(json& res) const {
        for (auto arg: Args()) {
            arg->Save(res);
        }
    }

    void Dump(ostream& out, int indent) const {
        json saved;
        Save(saved);
        out << saved.dump(indent);
    }

    bool Add(const ArgBase* arg, ostream& err) {
        if (!arg) {
            throw runtime_error("Config::Impl::Add(): Null argument pointer");
        }

        auto info = arg->ReportedName() + " failed to add to config: ";
        if (!(ArgMap_.insert({ arg->Name(), arg })).second) {
            err << info << "Option name '" << arg->Name() << "' is already used";
            return false;
        }
        if (arg->Name().empty()) {
            err << info << "Option name can not be empty";
            return false;
        }
        if (arg->Name().find(' ') != string::npos) {
            err << info << "Option name can not contain space";
            return false;
        }
        if (((Flavours_ & _LongDoubleDash) || (Flavours_ & _LongSingleDash)) && arg->Name()[0] == '-') {
            err << info << "Option name can not start with dash";
            return false;
        }
        if ((Flavours_ & _EqualsSep) && arg->Name().find('=') != string::npos) {
            err << info << "Option name can not contain equals sign '=' since it is value separator";
            return false;
        }

        if (arg->IsFree()) {
            if (!FreeArgs_.empty() && (!FreeArgs_.back()->IsRequired() || FreeArgs_.back()->IsMultiple())) {
                err << info << "Only the last free arg is allowed to be optional or accept multiple values";
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
    bool MatchLong(const ArgBase& arg, const string& a) const {
        for (auto& name: arg.Names()) {
            if (
                ((Flavours_ & _LongDoubleDash) && "--" + name == a) ||
                ((Flavours_ & _LongSingleDash) && "-" + name == a) ||
                ((Flavours_ & _LongNoDash) && name == a)
            ) {
                return true;
            }
        }
        return false;
    }

    bool MatchShort(size_t sym, const ArgBase& arg, const string& a) const {
        for (auto& name: arg.Names()) {
            if (name.size() == 1 && name[0] == a[sym]) {
                return true;
            }
        }
        return false;
    }

private:
    const uint64_t Flavours_;
    json Data_;
    vector<ArgPtr> HeldArgs_;
    vector<const ArgBase*> NamedArgs_;
    vector<const ArgBase*> FreeArgs_;
    unordered_map<string, const ArgBase*> ArgMap_;
};

Config::Config(string name, string info, string version, ostream& infoOutput, uint64_t flavours, const json& testing)
    : Impl_(new Impl(flavours))
{
    if (flavours & _HelpAction) {
        auto a = CreateHelpAction(*this, name, info, infoOutput, testing.count("test-help"));
        if (flavours & _HelpShort) {
            a->Alias("h");
        }
        Impl_->Hold(move(a));
    }
    if (flavours & _VersionAction) {
        auto a = CreateVersionAction(*this, name, version, infoOutput, testing.count("test-version"));
        if (flavours & _VersionShort) {
            a->Alias("v");
        }
        Impl_->Hold(move(a));
    }
    if (flavours & _ConfigAction) {
        auto a = CreateLoadAction(*this, testing.count("test-load") ? testing["test-load"] : "");
        if (flavours & _ConfigShort) {
            a->Alias("c");
        }
        Impl_->Hold(move(a));
    }
}

Config::~Config() = default;

bool Config::Load(const json& src, ostream& err) {
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

const json& Config::Get() const {
    return Impl_->Get();
}

void Config::Save(json& res) const {
    return Impl_->Save(res);
}

void Config::Dump(ostream& out, int indent) const {
    return Impl_->Dump(out, indent);
}

bool Config::Add(const ArgBase* arg, ostream& err) {
    return Impl_->Add(arg, err);
}

} // namespace clar
