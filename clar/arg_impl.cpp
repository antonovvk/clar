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
    bool Get<bool>(const json& data) {
        return data.get<bool>();
    }

    template <>
    void Save<bool>(json& res, const bool& val) {
        res = val;
    }

    template <>
    bool Check<bool>(const json& val, ostream& err) {
        if (!val.is_boolean()) {
            err << "Expected boolean";
            return false;
        }
        return true;
    }

    template <>
    bool Parse<bool>(json& res, const string&, ostream&) {
        res = true;
        return true;
    }

    template <>
    ArgBase::Value RequiresValue<char>() {
        return ArgBase::_Single;
    }

    template <>
    string Meta<char>() {
        return "char";
    }

    template <>
    char Get<char>(const json& data) {
        return data.get<string>()[0];
    }

    template <>
    void Save<char>(json& res, const char& val) {
        res = string(1, val);
    }

    template <>
    bool Check<char>(const json& val, ostream& err) {
        if (!val.is_string() || val.get<string>().size() != 1) {
            err << "Expected one character string";
            return false;
        }
        return true;
    }

    template <>
    bool Parse<char>(json& res, const string& str, ostream& err) {
        if (str.size() != 1) {
            err << "Expected one character string";
            return false;
        }
        res = str;
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
    json Get<json>(const json& data) {
        return data;
    }

    template <>
    void Save<json>(json& res, const json& val) {
        res = val;
    }

    template <>
    bool Check<json>(const json&, ostream&) {
        return true;
    }

    template <>
    bool Parse<json>(json& res, const string& str, ostream& err) {
        try {
            res = json::parse(str);
        } catch (const exception& e) {
            err << "Failed to parse JSON: " << e.what();
            return false;
        }
        return true;
    }

    template <bool S, bool F, bool U>
    bool CheckValue(const json&, ostream&);

    template <>
    bool CheckValue<true, false, false>(const json& val, ostream& err) {
        if (!val.is_string()) {
            err << "Expected string";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, true, false>(const json& val, ostream& err) {
        if (!val.is_number_float()) {
            err << "Expected floating point";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, false, false>(const json& val, ostream& err) {
        if (!val.is_number_integer()) {
            err << "Expected signed integer";
            return false;
        }
        return true;
    }

    template <>
    bool CheckValue<false, false, true>(const json& val, ostream& err) {
        if (!val.is_number_unsigned()) {
            err << "Expected unsigned integer";
            return false;
        }
        return true;
    }

    template <typename T>
    bool ParseValue(T& val, const string& str, ostream& err) {
        try {
            val = stoi(str);
        } catch (const exception& e) {
            err << e.what();
            return false;
        }
        return true;
    }

    template <>
    bool ParseValue(uint32_t& val, const string& str, ostream& err) {
        try {
            val = stoul(str);
        } catch (const exception& e) {
            err << e.what();
            return false;
        }
        return true;
    }

    template <>
    bool ParseValue(int64_t& val, const string& str, ostream& err) {
        try {
            val = stol(str);
        } catch (const exception& e) {
            err << e.what();
            return false;
        }
        return true;
    }

    template <>
    bool ParseValue(uint64_t& val, const string& str, ostream& err) {
        try {
            val = stoul(str);
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
        static T Get(const json& data) {
            return data.get<T>();
        }

        static void Save(json& res, const T& val) {
            res = val;
        }

        static bool Check(const json& val, ostream& err) {
            if (!CheckValue<is_same<T, string>::value, is_floating_point<T>::value, is_unsigned<T>::value>(val, err)) {
                return false;
            }
            return true;
        }

        static bool Parse(json& res, const string& str, ostream& err) {
            T val;
            if (!ParseValue<T>(val, str, err)) {
                return false;
            }
            res = val;
            return true;
        }
    };

    template <typename T>
    struct MultiValueTraits {
        static vector<T> Get(const json& data) {
            vector<T> res;
            for (auto& val: data) {
                res.push_back(impl::Get<T>(val));
            }
            return res;
        }

        static void Save(json& res, const vector<T>& val) {
            res = json::array();
            for (auto& v: val) {
                res.push_back(v);
            }
        }

        static bool Check(const json& val, ostream& err) {
            if (!val.is_array()) {
                err << "Expected array";
                return false;
            }
            if (!val.empty() && !impl::Check<T>(val.back(), err)) {
                err << " array";
                return false;
            }
            return true;
        }

        static bool Parse(json& res, const string& str, ostream& err) {
            json val;
            if (!impl::Parse<T>(val, str, err)) {
                err << " array";
                return false;
            }
            res.push_back(val);
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
    type Get<type>(const json& data) { \
        return SingleValueTraits<type>::Get(data); \
    } \
    template <> \
    void Save<type>(json& res, const type& val) { \
        return SingleValueTraits<type>::Save(res, val); \
    } \
    template <>\
    bool Check<type>(const json& val, ostream& err) { \
        return SingleValueTraits<type>::Check(val, err); \
    } \
    template <> \
    bool Parse<type>(json& res, const string& str, ostream& err) { \
        return SingleValueTraits<type>::Parse(res, str, err); \
    }

    IMPL_SINGLE(unsigned char, "uchar");

    IMPL_SINGLE(int16_t, "int16");
    IMPL_SINGLE(uint16_t, "uint16");

    IMPL_SINGLE(int32_t, "int32");
    IMPL_SINGLE(uint32_t, "uint32");

    IMPL_SINGLE(int64_t, "int64");
    IMPL_SINGLE(uint64_t, "uint64");

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
    vector<type> Get<vector<type>>(const json& data) { \
        return MultiValueTraits<type>::Get(data); \
    } \
    template <> \
    void Save<vector<type>>(json& res, const vector<type>& val) { \
        return MultiValueTraits<type>::Save(res, val); \
    } \
    template <>\
    bool Check<vector<type>>(const json& val, ostream& err) { \
        return MultiValueTraits<type>::Check(val, err); \
    } \
    template <> \
    bool Parse<vector<type>>(json& res, const string& str, ostream& err) { \
        return MultiValueTraits<type>::Parse(res, str, err); \
    }

    IMPL_MULTI(char, "char");

    IMPL_MULTI(int32_t, "int32");
    IMPL_MULTI(uint32_t, "uint32");

    IMPL_MULTI(int64_t, "int64");
    IMPL_MULTI(uint64_t, "uint64");

    IMPL_MULTI(float, "float");
    IMPL_MULTI(double, "double");

    IMPL_MULTI(string, "string");

#undef IMPL_MULTI

} // namespace impl
} // namespace clar
