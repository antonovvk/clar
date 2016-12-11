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
    T Get(const std::string&, const json&, T def);

    template <typename T>
    bool Check(const ArgBase&, const json&, std::ostream&);

    template <typename T>
    bool Parse(json&, const ArgBase&, const std::string&, std::ostream&);

} // namespace impl
} // namespace clar
