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
    string Meta<bool>() {
        return "";
    }

    template <>
    bool Get<bool>(const string& name, const json& data, bool def) {
        auto it = data.find(name);
        return it != data.end() ? it.value().get<bool>() : def;
    }

    template <>
    bool Check<bool>(const ArgBase& arg, const json& val, ostream& err) {
        if (!val.is_boolean()) {
            err << arg.ReportedName() << " expects boolean value in config";
            return false;
        }
        return true;
    }

    template <>
    bool Parse<bool>(json& res, const ArgBase& arg, const string&, ostream&) {
        res[arg.Name()] = true;
        return true;
    }

    template <>
    ArgBase::Value RequiresValue<json>() {
        return ArgBase::_Single;
    }

    template <>
    string Meta<json>() {
        return "JSON";
    }

    template <>
    json Get<json>(const string& name, const json& data, json def) {
        auto it = data.find(name);
        return it != data.end() ? it.value() : def;
    }

    template <>
    bool Check<json>(const ArgBase&, const json&, ostream&) {
        return true;
    }

    template <>
    bool Parse<json>(json& res, const ArgBase& arg, const string& str, ostream& err) {
        try {
            res[arg.Name()] = json::parse(str);
        } catch (const exception& e) {
            err << "Failed to parse JSON: " << e.what();
            return false;
        }
        return true;
    }

    template <bool S, bool F, bool U>
    bool CheckValue(const ArgBase&, const json&, ostream&);

    template <>
    bool CheckValue<true, false, false>(const ArgBase& arg, const json& val, ostream& err) {
        if (!val.is_string()) {
            err << "Expected string";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, true, false>(const ArgBase& arg, const json& val, ostream& err) {
        if (!val.is_number_float()) {
            err << "Expected floating point";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, false, false>(const ArgBase& arg, const json& val, ostream& err) {
        if (!val.is_number_integer()) {
            err << "Expected signed integer";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, false, true>(const ArgBase& arg, const json& val, ostream& err) {
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
        static T Get(const string& name, const json& data, T def) {
            auto it = data.find(name);
            return it != data.end() ? it.value().get<T>() : def;
        }

        static bool Check(const ArgBase& arg, const json& val, ostream& err) {
            ostringstream e;
            if (!CheckValue<is_same<T, string>::value, is_floating_point<T>::value, is_unsigned<T>::value>(arg, val, e)) {
                err << arg.ReportedName() << ": " << e.str() << " value in config";
                return false;
            }
            return true;
        }

        static bool Parse(json& res, const ArgBase& arg, const string& str, ostream& err) {
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
        static vector<T> Get(const string& name, const json& data, vector<T> def) {
            auto it = data.find(name);
            return it != data.end() ? it.value().get<vector<T>>() : def;
        }

        static bool Check(const ArgBase& arg, const json& val, ostream& err) {
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

        static bool Parse(json& res, const ArgBase& arg, const string& str, ostream& err) {
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

#define IMPL_SINGLE(type, meta) \
    template <> \
    ArgBase::Value RequiresValue<type>() { \
        return ArgBase::_Single; \
    } \
    template <>\
    string Meta<type>() { \
        return meta; \
    } \
    template <> \
    type Get<type>(const string& name, const json& data, type def) { \
        return SingleValueTraits<type>::Get(name, data, def); \
    } \
    template <>\
    bool Check<type>(const ArgBase& arg, const json& val, ostream& err) { \
        return SingleValueTraits<type>::Check(arg, val, err); \
    } \
    template <> \
    bool Parse<type>(json& res, const ArgBase& arg, const string& str, ostream& err) { \
        return SingleValueTraits<type>::Parse(res, arg, str, err); \
    }

    IMPL_SINGLE(char, "char");
    IMPL_SINGLE(unsigned char, "char");

    IMPL_SINGLE(short, "short");
    IMPL_SINGLE(unsigned short, "short");

    IMPL_SINGLE(int, "int");
    IMPL_SINGLE(unsigned int, "uint");

    IMPL_SINGLE(long int, "int");
    IMPL_SINGLE(unsigned long int, "uint");

    IMPL_SINGLE(float, "float");
    IMPL_SINGLE(double, "double");

    IMPL_SINGLE(string, "string");

#undef IMPL_SINGLE

#define IMPL_MULTI(type, meta) \
    template <> \
    ArgBase::Value RequiresValue<vector<type>>() { \
        return ArgBase::_Multiple; \
    } \
    template <>\
    string Meta<vector<type>>() { \
        return meta; \
    } \
    template <> \
    vector<type> Get<vector<type>>(const string& name, const json& data, vector<type> def) { \
        return MultiValueTraits<type>::Get(name, data, def); \
    } \
    template <>\
    bool Check<vector<type>>(const ArgBase& arg, const json& val, ostream& err) { \
        return MultiValueTraits<type>::Check(arg, val, err); \
    } \
    template <> \
    bool Parse<vector<type>>(json& res, const ArgBase& arg, const string& str, ostream& err) { \
        return MultiValueTraits<type>::Parse(res, arg, str, err); \
    }

    IMPL_MULTI(int, "int");
    IMPL_MULTI(unsigned int, "uint");

    IMPL_MULTI(long int, "int");
    IMPL_MULTI(unsigned long int, "uint");

    IMPL_MULTI(float, "float");
    IMPL_MULTI(double, "double");

    IMPL_MULTI(string, "string");

#undef IMPL_MULTI

} // namespace impl
} // namespace clar
