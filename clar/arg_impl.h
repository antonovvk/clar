#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "nlohmann/json.hpp"
#include "arg_base.h"

namespace clar {
namespace impl {
    template <typename T>
    ArgBase::Value RequiresValue();

    template <typename T>
    std::string Meta();

    template <typename T>
    T Get(const json&);

    template <typename T>
    void Save(json&, const T&);

    template <typename T>
    bool Check(const json&, std::ostream&);

    template <typename T>
    bool Parse(json&, const std::string&, std::ostream&);

} // namespace impl
} // namespace clar
