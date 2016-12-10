#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "src/json.hpp"

namespace clar {
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

        //~ ArgBase& Short(char c);
        ArgBase& Long(std::string name);

        const std::string& Name() const;
        const std::string& Info() const;

        bool IsRequired() const;
        Value RequiresValue() const;

        const std::vector<char>& ShortNames() const;
        const std::vector<std::string>& LongNames() const;

        virtual std::string ReportedName() const;

    protected:
        friend class Config;
        ArgBase(Config& config, std::string name, std::string info, bool required, Value value);

        virtual bool Check(const nlohmann::json& val, std::ostream& err) const = 0;
        virtual bool Parse(nlohmann::json& res, const std::string& val, std::ostream& err) const = 0;

    protected:
        const std::string Name_;
        const std::string Info_;
        const bool Required_;
        const Value Value_;

        std::vector<char> ShortNames_;
        std::vector<std::string> LongNames_;

        Config& Config_;
    };
} // namespace clar
