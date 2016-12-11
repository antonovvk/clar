#pragma once

#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "clar.h"

namespace clar {
    template <typename Action>
    class ActionArg: public ArgBase {
    public:
        ActionArg(std::string name, std::string info, Action action)
            : ArgBase(name, info, false, false, ArgBase::_None)
            , Action_(action)
        {
        }

        ActionArg(Config& config, std::string name, std::string info, Action action)
            : ActionArg(name, info, action)
        {
            std::ostringstream err;
            if (!Add(config, err)) {
                throw std::domain_error(err.str());
            }
        }

        virtual ~ActionArg() override
        {
        }

        virtual bool Check(const json&, std::ostream& err) const override {
            err << ReportedName() << " is an action argument and can not be loaded from config";
            return false;
        }

        virtual bool Parse(json&, const std::string&, std::ostream&) const override {
            Action_();
            return true;
        }

    private:
        Action Action_;
    };

    template <typename Action>
    ArgPtr CreateActionArg(std::string name, std::string info, Action action) {
        return ArgPtr(new ActionArg<Action>(name, info, action));
    }

    template <typename Action>
    ArgPtr CreateActionArg(Config& config, std::string name, std::string info, Action action) {
        return ArgPtr(new ActionArg<Action>(config, name, info, action));
    }

    ArgPtr CreateHelpAction(Config& config, std::string name, std::string info, std::ostream& out, bool testing = false);

} // namespace clar
