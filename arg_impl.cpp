#include <sstream>
#include <type_traits>

#include "arg_impl.h"

using namespace std;

namespace clar {
namespace impl {
    template <>
    ArgBase::Value RequiresValue<bool>() {
        return ArgBase::_None;
    }

    template <>
    bool Get<bool>(const string& name, const nlohmann::json& data, bool def) {
        auto it = data.find(name);
        return it != data.end() ? it.value().get<bool>() : def;
    }

    template <>
    bool Check<bool>(const ArgBase& arg, const nlohmann::json& val, ostream& err) {
        if (!val.is_boolean()) {
            err << arg.ReportedName() << " expects boolean value in config";
            return false;
        }
        return true;
    }

    template <>
    bool Parse<bool>(nlohmann::json& res, const ArgBase& arg, const string&, ostream&) {
        res[arg.Name()] = true;
        return true;
    }

    template <bool S, bool F, bool U>
    bool CheckValue(const ArgBase&, const nlohmann::json&, ostream&);

    template <>
    bool CheckValue<true, false, false>(const ArgBase& arg, const nlohmann::json& val, ostream& err) {
        if (!val.is_string()) {
            err << "Expected string";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, true, false>(const ArgBase& arg, const nlohmann::json& val, ostream& err) {
        if (!val.is_number_float()) {
            err << "Expected floating point";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, false, false>(const ArgBase& arg, const nlohmann::json& val, ostream& err) {
        if (!val.is_number_integer()) {
            err << "Expected signed integer";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, false, true>(const ArgBase& arg, const nlohmann::json& val, ostream& err) {
        if (!val.is_number_unsigned()) {
            err << "Expected unsigned integer";
            return false;
        }
        return true;
    }

    template <typename T>
    bool ParseValue(T& val, const string& str, ostream& err) {
        try {
            // TODO: check T overflow
            val = stoi(str);
        } catch (const exception& e) {
            err << e.what();
            return false;
        }
        return true;
    }

    template <>
    bool ParseValue<string>(string& val, const string& str, ostream&) {
        val = str;
        return true;
    }

    template <>
    bool ParseValue<float>(float& val, const string& str, ostream& err) {
        try {
            val = stof(str);
        } catch (const exception& e) {
            err << e.what();
            return false;
        }
        return true;
    }

    template <>
    bool ParseValue<double>(double& val, const string& str, ostream& err) {
        try {
            val = stod(str);
        } catch (const exception& e) {
            err << e.what();
            return false;
        }
        return true;
    }

    template <typename T>
    struct SingleValueTraits {
        static T Get(const string& name, const nlohmann::json& data, T def) {
            auto it = data.find(name);
            return it != data.end() ? it.value().get<T>() : def;
        }

        static bool Check(const ArgBase& arg, const nlohmann::json& val, ostream& err) {
            ostringstream e;
            if (!CheckValue<is_same<T, string>::value, is_floating_point<T>::value, is_unsigned<T>::value>(arg, val, e)) {
                err << arg.ReportedName() << ": " << e.str() << " value in config";
                return false;
            }
            return true;
        }

        static bool Parse(nlohmann::json& res, const ArgBase& arg, const string& str, ostream& err) {
            T val;
            ostringstream e;
            if (!ParseValue<T>(val, str, e)) {
                err << arg.ReportedName() << " failed to parse value: " << e.str();
                return false;
            }
            res[arg.Name()] = val;
            return true;
        }
    };

    template <typename T>
    struct MultiValueTraits {
        static vector<T> Get(const string& name, const nlohmann::json& data, vector<T> def) {
            auto it = data.find(name);
            return it != data.end() ? it.value().get<vector<T>>() : def;
        }

        static bool Check(const ArgBase& arg, const nlohmann::json& val, ostream& err) {
            if (!val.is_array()) {
                err << arg.ReportedName() << ": Expected array in config";
                return false;
            }
            ostringstream e;
            if (!val.empty() && !CheckValue<is_same<T, string>::value, is_floating_point<T>::value, is_unsigned<T>::value>(arg, val.back(), e)) {
                err << arg.ReportedName() << ": " << e.str() << " array in config";
                return false;
            }
            return true;
        }

        static bool Parse(nlohmann::json& res, const ArgBase& arg, const string& str, ostream& err) {
            T val;
            ostringstream e;
            if (!ParseValue<T>(val, str, e)) {
                err << arg.ReportedName() << " failed to parse value: " << e.str();
                return false;
            }
            res[arg.Name()].push_back(val);
            return true;
        }
    };

#define IMPL_SINGLE(type) \
    template <> \
    ArgBase::Value RequiresValue<type>() { \
        return ArgBase::_Single; \
    } \
    template <> \
    type Get<type>(const string& name, const nlohmann::json& data, type def) { \
        return SingleValueTraits<type>::Get(name, data, def); \
    } \
    template <>\
    bool Check<type>(const ArgBase& arg, const nlohmann::json& val, ostream& err) { \
        return SingleValueTraits<type>::Check(arg, val, err); \
    } \
    template <> \
    bool Parse<type>(nlohmann::json& res, const ArgBase& arg, const string& str, ostream& err) { \
        return SingleValueTraits<type>::Parse(res, arg, str, err); \
    }

    IMPL_SINGLE(char);
    IMPL_SINGLE(unsigned char);

    IMPL_SINGLE(short);
    IMPL_SINGLE(unsigned short);

    IMPL_SINGLE(int);
    IMPL_SINGLE(unsigned int);

    IMPL_SINGLE(long int);
    IMPL_SINGLE(unsigned long int);

    IMPL_SINGLE(float);
    IMPL_SINGLE(double);

    IMPL_SINGLE(string);

#undef IMPL_SINGLE

#define IMPL_MULTI(type) \
    template <> \
    ArgBase::Value RequiresValue<vector<type>>() { \
        return ArgBase::_Multiple; \
    } \
    template <> \
    vector<type> Get<vector<type>>(const string& name, const nlohmann::json& data, vector<type> def) { \
        return MultiValueTraits<type>::Get(name, data, def); \
    } \
    template <>\
    bool Check<vector<type>>(const ArgBase& arg, const nlohmann::json& val, ostream& err) { \
        return MultiValueTraits<type>::Check(arg, val, err); \
    } \
    template <> \
    bool Parse<vector<type>>(nlohmann::json& res, const ArgBase& arg, const string& str, ostream& err) { \
        return MultiValueTraits<type>::Parse(res, arg, str, err); \
    }

    IMPL_MULTI(int);
    IMPL_MULTI(unsigned int);

    IMPL_MULTI(long int);
    IMPL_MULTI(unsigned long int);

    IMPL_MULTI(float);
    IMPL_MULTI(double);

    IMPL_MULTI(string);

#undef IMPL_MULTI

} // namespace impl
} // namespace clar
