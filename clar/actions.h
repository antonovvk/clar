#pragma once

#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "config.h"

namespace clar {
    template <typename Action>
    class ActionArg: public ArgBase {
    public:
        ActionArg(std::string name, std::string info, ArgBase::Value value, Action action)
            : ArgBase(name, info, false, false, value)
            , Action_(action)
        {
        }

        ActionArg(Config& config, std::string name, std::string info, ArgBase::Value value, Action action)
            : ActionArg(name, info, value, action)
        {
            std::ostringstream err;
            if (!Add(config, err)) {
                throw std::domain_error(err.str());
            }
        }

        virtual ~ActionArg() override
        {
        }

        virtual void Save(json&) const override {
            return;
        }

        virtual bool Check(const json&, std::ostream& err) const override {
            err << ReportedName() << " is an action argument and can not be loaded from config";
            return false;
        }

        virtual bool Parse(json&, const std::string& val, std::ostream& err) const override {
            return Action_(val, err);
        }

    private:
        Action Action_;
    };

    template <typename Action>
    ArgPtr CreateActionArg(std::string name, std::string info, ArgBase::Value value, Action action) {
        return ArgPtr(new ActionArg<Action>(name, info, value, action));
    }

    template <typename Action>
    ArgPtr CreateActionArg(Config& config, std::string name, std::string info, ArgBase::Value value, Action action) {
        return ArgPtr(new ActionArg<Action>(config, name, info, value, action));
    }

    template <typename Action>
    ArgPtr CreateActionSwitch(Config& config, std::string name, std::string info, Action action) {
        return ArgPtr(new ActionArg<Action>(config, name, info, ArgBase::_None, action));
    }

    ArgPtr CreateHelpAction(Config& config, std::string name, std::string info, std::ostream& out, bool testing = false);
    ArgPtr CreateLoadAction(Config& config, std::string testData = std::string());
} // namespace clar
