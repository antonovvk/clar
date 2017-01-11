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

        _SpaceSep       = 0x0010,   // --foo val
        _EqualsSep      = 0x0020,   // --foo=val

        _ShortDash      = 0x0100,   // -a
        _ShortNoDash    = 0x0200,   // a
        _ShortStacked   = 0x0400,   // -a -b equivalent to -ab
        _ShortNoSep     = 0x0800,   // -a val equivalent to -aval

        _HelpAction     = 0x001000, // --help for help
        _VersionAction  = 0x002000, // --version for version
        _ConfigAction   = 0x004000, // --config for config
        _CommonActions  = _HelpAction | _VersionAction | _ConfigAction,

        _HelpShort      = 0x010000, // -h alias for --help
        _VersionShort   = 0x020000, // -v alias for --version
        _ConfigShort    = 0x040000, // -c alias for --config
        _CommonActShort = _HelpShort | _VersionShort | _ConfigShort,

        _UnixFlavours =
            _LongDoubleDash |
            _SpaceSep |
            _ShortDash |
            _ShortStacked |
            _ShortNoSep |
            _CommonActions |
            _CommonActShort
        ,
    };

    class Config {
        friend class ArgBase;

    public:
        Config(
            std::string name = "",
            std::string info = "",
            std::ostream& infoOutput = std::cout,
            uint64_t flavours = _UnixFlavours,
            const json& testing = json::object()
        );

        ~Config();

        bool Load(const json& src, std::ostream& err);
        bool Parse(int argc, char* argv[], std::ostream& err);
        bool Parse(const std::vector<std::string>& args, std::ostream& err);
        bool Alias(const std::string& arg, const std::string& alias, std::ostream& err);

        std::vector<const ArgBase*> Args() const;
        const json& Get() const;
        void Save(json& res) const;
        void Dump(std::ostream& out, int indent) const;

    private:
        bool Add(const ArgBase* arg, std::ostream& err);

    private:
        class Impl;
        std::unique_ptr<Impl> Impl_;
    };

    template <typename T, bool Required = false, char Short = 0>
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

        virtual void Save(json& res) const override {
            impl::Save(res[Name_], Get());
        }

        virtual bool Check(const json& val, std::ostream& err) const override {
            std::ostringstream e;
            if (!impl::Check<T>(val, e)) {
                err << ArgBase::ReportedName() << ": Failed to load value: " << e.str();
                return false;
            }
            return true;
        }

        virtual bool Parse(json& res, const std::string& val, std::ostream& err) const override {
            std::ostringstream e;
            if (!impl::Parse<T>(res[Name()], val, e)) {
                err << ArgBase::ReportedName() << ": Failed to parse value: " << e.str();
                return false;
            }
            return true;
        }

        operator T () const {
            return Get();
        }

        bool operator! () const {
            if (!Config_) {
                throw std::domain_error(ArgBase::ReportedName() + " wasn't added to config");
            }
            return Config_->Get().count(Name_) == 0;
        }

        T Get() const {
            if (!Config_) {
                throw std::domain_error(ArgBase::ReportedName() + " wasn't added to config");
            }
            auto it = Config_->Get().find(Name_);
            if (it != Config_->Get().end()) {
                return impl::Get<T>(it.value());
            }
            return Default_;
        }

        const T& Default() const {
            return Default_;
        }

        void Default(T val) {
            Default_ = val;
        }

    protected:
        NamedArg(bool free, std::string name, std::string info, T def)
            : ArgBase(name, info, Required, free, impl::RequiresValue<T>())
            , Default_(def)
        {
            if (Short) {
                Alias(std::string(1, Short));
            }
            if (ArgBase::Meta().empty()) {
                ArgBase::Meta(impl::Meta<T>());
            }
        }

    private:
        T Default_;
    };

    template <typename T, char Short = 0>
    using NamedOpt = NamedArg<T, false, Short>;

    template <char Short = 0>
    using Switch = NamedArg<bool, false, Short>;

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
