#pragma once

#include <IRMC_Macro.hpp>

namespace IRMC {

    template <typename T>
    class Defer {
    public:
        Defer() = default;
        
        Defer(T func) : m_Func(func) {}

        ~Defer() { m_Func(); }

    private:
        T m_Func;
    };

}

#define IRMC_DEFER(_block) IRMC::Defer IRMC_UNIQUE(__irmc_defer)([&]() _block )