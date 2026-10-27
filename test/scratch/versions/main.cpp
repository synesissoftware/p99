
#include <p99/p99.h>

#ifdef HAS_BDUT
# include <bdut/bdut.h>
#endif

#include <iomanip>
#include <iostream>

#include <stdlib.h>


template<
    typename T_stream
,   typename T_integer
>
void
version(
    T_stream&   stm
,   char const* prefix
,   char const* libname
,   char const* macroname
,   T_integer   libver
)
{
    stm
        << prefix
        << libname
        << ": v"
        << ((libver >> 24) & 0xff)
        << '.'
        << ((libver >> 16) & 0xff)
        << '.'
        << ((libver >> 8) & 0xff)
        << '.'
        << ((libver >> 0) & 0xff)
        << " ("
        << macroname
        << " = 0x"
        << std::hex << std::setfill('0') << std::setw(8)
        << static_cast<unsigned>(libver)
        << std::dec
        << ")"
        << std::endl
        ;
}


int main(int /* argc */, char* /* argv */[])
{
    int efferents_count = 0;

    {
        unsigned const libver = P99_VER;

        version(std::cout, "", "p99", "P99_VER", libver);
    }

#ifdef HAS_BDUT

    if (0 == efferents_count++)
    {
        std::cout << "\n" << "efferent dependencies:" << std::endl;
    }

    {
        unsigned const libver = BDUT_VER;

        version(std::cout, "\t", "BDUT", "BDUT_VER", libver);
    }
#endif

    return EXIT_SUCCESS;
}
