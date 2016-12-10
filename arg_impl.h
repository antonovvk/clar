#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "src/json.hpp"
#include "arg_base.h"

namespace clar {
namespace impl {
    template <typename T>
    ArgBase::Value RequiresValue();

    template <typename T>
    T Get(const std::string&, const nlohmann::json&, T def);

    template <typename T>
    bool Check(const ArgBase&, const nlohmann::json&, std::ostream&);

    template <typename T>
    bool Parse(nlohmann::json&, const ArgBase&, const std::string&, std::ostream&);

} // namespace impl
} // namespace clar
