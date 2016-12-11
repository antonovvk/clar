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

        //~ ArgBase& Short(char c);
        ArgBase& Long(std::string name);

        const std::string& Name() const;
        const std::string& Info() const;

        bool IsFree() const;
        bool IsRequired() const;
        bool IsSwitch() const;
        bool IsMultiple() const;

        const std::vector<char>& ShortNames() const;
        const std::vector<std::string>& LongNames() const;

        std::string ReportedName() const;

    protected:
        friend class Config;
        ArgBase(std::string name, std::string info, bool required, bool free, Value value);

        virtual bool Check(const json& val, std::ostream& err) const = 0;
        virtual bool Parse(json& res, const std::string& val, std::ostream& err) const = 0;

    protected:
        const std::string Name_;
        const std::string Info_;
        const bool Required_;
        const bool Free_;
        const Value Value_;

        std::vector<char> ShortNames_;
        std::vector<std::string> LongNames_;

        Config* Config_;
    };

    typedef std::unique_ptr<ArgBase> ArgPtr;
} // namespace clar
