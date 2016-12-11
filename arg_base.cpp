#include <sstream>

#include "clar.h"
#include "arg_base.h"

using namespace std;

namespace clar {

ArgBase::ArgBase(string name, string info, bool required, Value value)
    : Name_(name)
    , Info_(info)
    , Required_(required)
    , Value_(value)
    , Config_(nullptr)
{
}

ArgBase::~ArgBase()
{
}

bool ArgBase::AddNamed(Config* config, ostream& err) {
    Config_ = config;
    if (!Config_) {
        err << "Null config pointer";
        return false;
    }
    if (!Config_->AddNamed(this, err)) {
        return false;
    }
    for (auto n: LongNames_) {
        if (!AddAlias(n, err)) {
            return false;
        }
    }
    LongNames_.push_back(Name_);
    return true;
}

bool ArgBase::AddFree(Config* config, ostream& err) {
    Config_ = config;
    if (!Config_) {
        err << "Null config pointer";
        return false;
    }
    if (!Config_->AddFree(this, err)) {
        return false;
    }
    return true;
}

bool ArgBase::AddAlias(const string& name, ostream& err) {
    assert(Config_);
    ostringstream e;
    if (!Config_->Alias(Name_, name, e)) {
        err << ReportedName() << " failed to add alias: " << e.str();
        return false;
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
        if (!AddAlias(name, err)) {
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

bool ArgBase::IsRequired() const {
    return Required_;
}

bool ArgBase::IsMultiple() const {
    return Value_ == _Multiple;
}

ArgBase::Value ArgBase::RequiresValue() const {
    return Value_;
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
