#include <sstream>

#include "clar.h"
#include "arg_base.h"

using namespace std;

namespace clar {

ArgBase::ArgBase(Config& config, string name, string info, bool required, Value value)
    : Name_(name)
    , Info_(info)
    , Required_(required)
    , Value_(value)
    , Config_(config)
{
    ostringstream err;
    err << ReportedName() << " failed to add to config: ";
    if (!Config_.AddNamed(this, err)) {
        throw domain_error(err.str());
    }
    LongNames_.push_back(Name_);
}

ArgBase::~ArgBase()
{
}

//~ ArgBase& ArgBase::Short(char c) {
    //~ if (!Config_.Alias()) {
    //~ }
    //~ ShortNames_.push_back(c);
    //~ return *this;
//~ }

ArgBase& ArgBase::Long(string name) {
    ostringstream err;
    err << ReportedName() << " failed to add alias: ";
    if (!Config_.Alias(Name_, name, err)) {
        throw domain_error(err.str());
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
