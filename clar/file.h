#pragma once

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <type_traits>

#include "config.h"

namespace clar {
    template <bool Output, bool Free, bool Required, char Short = 0>
    class FileArg: public NamedArg<std::string, Required, Short> {
        typedef NamedArg<std::string, Required, Short> Base;

    public:
        typedef typename std::conditional<Output, std::ostream, std::istream>::type Stream;
        typedef typename std::conditional<Output, std::ofstream, std::ifstream>::type FileStream;

    public:
        FileArg(std::string name, std::string info, std::string def = std::string())
            : Base(Free, nullptr, name, info, def)
        {
        }

        FileArg(Config& config, std::string name, std::string info, std::string def = std::string())
            : Base(Free, &config, name, info, def)
        {
        }

        virtual ~FileArg() override
        {
        }

        std::unique_ptr<Stream> Open() const {
            auto name = Base::Get();
            if (name == "-") {
                return std::unique_ptr<Stream>(new Stream(Output ? std::cout.rdbuf() : std::cin.rdbuf()));
            }
            std::unique_ptr<FileStream> s(new FileStream(name));
            if (!*s) {
                return nullptr;
            }
            return std::unique_ptr<Stream>(s.release());
        }
    };

    template <bool Required, char Short = 0>
    using InputFileArg = FileArg<false, false, Required, Short>;

    template <bool Required, char Short = 0>
    using OutputFileArg = FileArg<true, false, Required, Short>;

    template <char Short = 0>
    using InputFileOpt = InputFileArg<false, Short>;

    template <char Short = 0>
    using OutputFileOpt = OutputFileArg<false, Short>;

    template <bool Required>
    using FreeInputFileArg = FileArg<false, true, Required>;

    template <bool Required>
    using FreeOutputFileArg = FileArg<true, true, Required>;

} // namespace clar
