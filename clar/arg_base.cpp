#include <sstream>

#include "config.h"
#include "arg_base.h"

using namespace std;

namespace clar {

ArgBase::ArgBase(string name, string info, bool required, bool free, Value value)
    : Name_(name)
    , Info_(info)
    , Required_(required)
    , Free_(free)
    , Value_(value)
    , Config_(nullptr)
{
}

ArgBase::~ArgBase()
{
}

bool ArgBase::Add(Config& config, ostream& err) {
    Config_ = &config;
    if (!Config_->Add(this, err)) {
        return false;
    }
    if (!IsFree()) {
        for (auto n: Names_) {
            if (!Config_->Alias(Name_, n, err)) {
                return false;
            }
        }
        Names_.push_back(Name_);
    }
    return true;
}

ArgBase& ArgBase::Alias(string name) {
    if (Config_) {
        ostringstream err;
        if (!Config_->Alias(Name_, name, err)) {
            throw domain_error(err.str());
        }
    }
    Names_.push_back(name);
    return *this;
}

ArgBase& ArgBase::Meta(string meta) {
    Meta_ = meta;
    return *this;
}

const string& ArgBase::Name() const {
    return Name_;
}

const string& ArgBase::Info() const {
    return Info_;
}

const string& ArgBase::Meta() const {
    return Meta_;
}

bool ArgBase::IsFree() const {
    return Free_;
}

bool ArgBase::IsRequired() const {
    return Required_;
}

bool ArgBase::IsSwitch() const {
    return Value_ == _None;
}

bool ArgBase::IsMultiple() const {
    return Value_ == _Multiple;
}

const vector<string>& ArgBase::Names() const {
    return Names_;
}

string ArgBase::ReportedName() const {
    return "Option '" + Name() + "'";
}

string ArgBase::ReportedDefault() const {
    if (!IsRequired()) {
        json def;
        Save(def);
        if (def.count(Name())) {
            return " (default value: " + def[Name()].dump() + ")";
        }
    }
    return "";
}

} // namespace clar
