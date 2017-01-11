#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "src/json.hpp"

namespace clar {
    using nlohmann::json;

    class Config;
    class ArgBase {
    public:
        enum Value {
            _None,
            _Single,
            _Multiple,
        };

    public:
        virtual ~ArgBase();

        bool Add(Config& config, std::ostream& err);
        ArgBase& Alias(std::string name);
        ArgBase& Meta(std::string meta);

        const std::string& Name() const;
        const std::string& Info() const;
        const std::string& Meta() const;

        bool IsFree() const;
        bool IsRequired() const;
        bool IsSwitch() const;
        bool IsMultiple() const;

        const std::vector<std::string>& Names() const;

        std::string ReportedName() const;
        std::string ReportedDefault() const;

    protected:
        friend class Config;
        ArgBase(std::string name, std::string info, bool required, bool free, Value value);

        virtual void Save(json& res)  const = 0;
        virtual bool Check(const json& val, std::ostream& err) const = 0;
        virtual bool Parse(json& res, const std::string& val, std::ostream& err) const = 0;

    protected:
        const std::string Name_;
        const std::string Info_;
        const bool Required_;
        const bool Free_;
        const Value Value_;
        std::string Meta_;
        std::vector<std::string> Names_;

        Config* Config_;
    };

    typedef std::unique_ptr<ArgBase> ArgPtr;
} // namespace clar
