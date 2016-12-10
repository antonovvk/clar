#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "src/json.hpp"
#include "arg_base.h"
#include "arg_impl.h"

namespace clar {
    enum Flavours {
        _LongDoubleDash = 0x0001,   // --foo
        _LongSingleDash = 0x0002,   // -foo
        _LongNoDash     = 0x0004,   // foo
        _ShortStacked   = 0x0008,   // -a -b equivalent to -ab
        _ShortSpaceSep  = 0x0010,   // -a val equivalent to -aval
        _ShortNoSep     = 0x0020,   // -a val equivalent to -aval
        _LongSpaceSep   = 0x0040,   // --foo val
        _LongEqualsSep  = 0x0080,   // --foo=val
        _LongNoSep      = 0x0100,   // --foo val aquivalent to --fooval
        _UnixFlavours = _LongDoubleDash | _ShortStacked | _ShortSpaceSep | _ShortNoSep | _LongSpaceSep,
    };

    class Config {
        friend class ArgBase;

    public:
        Config(uint64_t flavours = _UnixFlavours);
        ~Config();

        bool Load(const nlohmann::json& src, std::ostream& err);
        bool Parse(int argc, char* argv[], std::ostream& err);
        bool Parse(const std::vector<std::string>& args, std::ostream& err);
        bool Alias(const std::string& arg, const std::string& alias, std::ostream& err);

        const nlohmann::json& Get() const;

    private:
        bool AddNamed(ArgBase* arg, std::ostream& err);

    private:
        class Impl;
        std::unique_ptr<Impl> Impl_;
    };

    template <typename T, bool Required = false>
    class NamedArg: public ArgBase {
    public:
        NamedArg(Config& config, std::string name, std::string info, T def = T())
            : ArgBase(config, name, info, Required, impl::RequiresValue<T>())
            , Default_(def)
        {
        }

        virtual ~NamedArg() override
        {
        }

        virtual std::string ReportedName() const override {
            return "Argument '--" + ArgBase::Name() + "'";
        }

        virtual bool Check(const nlohmann::json& val, std::ostream& err) const override {
            return impl::Check<T>(*this, val, err);
        }

        virtual bool Parse(nlohmann::json& res, const std::string& val, std::ostream& err) const override {
            return impl::Parse<T>(res, *this, val, err);
        }

        T Get() const {
            return impl::Get<T>(Name_, Config_.Get(), Default_);
        }

    private:
        const T Default_;
    };

    template <typename T, bool Required = false>
    class FreeArg: public NamedArg<T, Required> {
    public:
        FreeArg(Config& config, size_t pos, std::string name, std::string info, T def = T())
            : NamedArg<T, Required>(config, name, info, def)
            , Position_(pos)
        {
        }

        virtual ~FreeArg() override
        {
        }

        virtual std::string ReportedName() const override {
            return "Free argument '" + ArgBase::Name() + "'";
        }

    private:
        size_t Position_;
    };
} // namespace clar
