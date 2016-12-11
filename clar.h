#pragma once

#include <memory>
#include <string>
#include <vector>
#include <sstream>

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
        _HelpAction     = 0x0100,   // --help for help
        _VersionAction  = 0x0200,   // --version for version
        _ConfigAction   = 0x0400,   // --config for config
        _CommonActions  = _HelpAction | _VersionAction | _ConfigAction,
        _HelpShort      = 0x1000,   // -h alias for --help
        _VersionShort   = 0x2000,   // -v alias for --version
        _ConfigShort    = 0x4000,   // -c alias for --config
        _CommonActShort = _HelpShort | _VersionShort | _ConfigShort,
        _UnixFlavours =
            _LongDoubleDash |
            _ShortStacked |
            _ShortSpaceSep |
            _ShortNoSep |
            _LongSpaceSep |
            _CommonActions |
            _CommonActShort
        ,
        _DoNotExitOnHelp    = 0x00010000,
    };

    class Config {
        friend class ArgBase;

    public:
        Config(
            std::string name = "",
            std::string info = "",
            std::ostream& infoOutput = std::cout,
            uint64_t flavours = _UnixFlavours
        );

        ~Config();

        bool Load(const json& src, std::ostream& err);
        bool Parse(int argc, char* argv[], std::ostream& err);
        bool Parse(const std::vector<std::string>& args, std::ostream& err);
        bool Alias(const std::string& arg, const std::string& alias, std::ostream& err);

        std::vector<const ArgBase*> Args() const;
        const json& Get() const;

    private:
        bool Add(const ArgBase* arg, std::ostream& err);

    private:
        class Impl;
        std::unique_ptr<Impl> Impl_;
    };

    template <typename T, bool Required = false>
    class NamedArg: public ArgBase {
    public:
        NamedArg(std::string name, std::string info, T def = T())
            : NamedArg(false, name, info, def)
        {
        }

        NamedArg(Config& config, std::string name, std::string info, T def = T())
            : NamedArg(name, info, def)
        {
            std::ostringstream err;
            if (!ArgBase::Add(config, err)) {
                throw std::domain_error(err.str());
            }
        }

        virtual ~NamedArg() override
        {
        }

        virtual bool Check(const json& val, std::ostream& err) const override {
            return impl::Check<T>(*this, val, err);
        }

        virtual bool Parse(json& res, const std::string& val, std::ostream& err) const override {
            return impl::Parse<T>(res, *this, val, err);
        }

        T Get() const {
            return Config_ ? impl::Get<T>(Name_, Config_->Get(), Default_) : Default_;
        }

    protected:
        NamedArg(bool free, std::string name, std::string info, T def)
            : ArgBase(name, info, Required, free, impl::RequiresValue<T>())
            , Default_(def)
        {
        }

    private:
        const T Default_;
    };

    template <typename T, bool Required = false>
    class FreeArg: public NamedArg<T, Required> {
    public:
        FreeArg(std::string name, std::string info, T def = T())
            : NamedArg<T, Required>(true, name, info, def)
        {
        }

        FreeArg(Config& config, std::string name, std::string info, T def = T())
            : FreeArg(name, info, def)
        {
            std::ostringstream err;
            if (!ArgBase::Add(config, err)) {
                throw std::domain_error(err.str());
            }
        }

        virtual ~FreeArg() override
        {
        }
    };
} // namespace clar
