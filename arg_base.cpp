#include <sstream>

#include "clar.h"
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
        for (auto n: LongNames_) {
            if (!Config_->Alias(Name_, n, err)) {
                return false;
            }
        }
        LongNames_.push_back(Name_);
    }
    return true;
}

//~ ArgBase& ArgBase::Short(char c) {
    //~ if (!Config_.Alias()) {
    //~ }
    //~ ShortNames_.push_back(c);
    //~ return *this;
//~ }

ArgBase& ArgBase::Long(string name) {
    if (Config_) {
        ostringstream err;
        if (!Config_->Alias(Name_, name, err)) {
            throw domain_error(err.str());
        }
    }
    LongNames_.push_back(name);
    return *this;
}

const string& ArgBase::Name() const {
    return Name_;
}

const string& ArgBase::Info() const {
    return Info_;
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

const vector<char>& ArgBase::ShortNames() const {
    return ShortNames_;
}

const vector<string>& ArgBase::LongNames() const {
    return LongNames_;
}

string ArgBase::ReportedName() const {
    return "Option '" + Name() + "'";
}

} // namespace clar
