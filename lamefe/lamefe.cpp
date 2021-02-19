#include <iostream>
#include "encoder.h"

#ifdef LAMEFE_WIDE
#   define LAMEFE_COUT std::wcout
#   define LAMEFE_MAIN wmain
#else
#   define LAMEFE_COUT std::cout
#   define LAMEFE_MAIN main
#endif

std::optional<unsigned> number(lamefe::Char *a)
{
    unsigned l_value;
    lamefe::SStream l_ss(a);
    if (l_ss >> l_value) return l_value;
    return std::optional<unsigned>();
}

lamefe::Settings settings(int a_argc, lamefe::Char *a_argv[])
{
    lamefe::Settings l_settings;
    if (a_argc < 3) return l_settings;
    l_settings.quality = number(a_argv[2]);
    if (a_argc < 4 || a_argv[3] == nullptr) return l_settings;
    if (a_argv[3][0] == 'c') l_settings.mode = lamefe::Settings::SMCBR;
    else if (a_argv[3][0] == 'a') l_settings.mode = lamefe::Settings::SMABR;
    else if (a_argv[3][0] == 'v') l_settings.mode = lamefe::Settings::SMVBR;
    if (a_argc < 5) return l_settings;
    l_settings.value = number(a_argv[4]);

    return l_settings;
}

int mainSub(int a_argc, lamefe::Char *a_argv[])
{
    if (a_argc < 2)
    {
        LAMEFE_COUT << "Usage:" << std::endl <<
            "lamefe dirpath [quality [mode value]]" << std::endl <<
            "\tquality: 0...9 (best-slowest...worst-fastest) or '*' for default" << std::endl <<
            "\tmode: c - CBR, a - ABR, v - VBR" << std::endl <<
            "\tvalue: in kbps for CBR & ABR or 0...9 (best-slowest...worst-fastest) for VBR" << std::endl;
        return -1;
    }

    lamefe::Encoder l_encoder(LAMEFE_COUT);
    l_encoder.set(settings(a_argc, a_argv));
    l_encoder.list(a_argv[1]);
    l_encoder.run();

    return 0;
}

int LAMEFE_MAIN(int a_argc, lamefe::Char *a_argv[])
{
    try
    {
        try { return mainSub(a_argc, a_argv); }
        catch (const std::exception &a)
        {
            LAMEFE_COUT << std::endl << "Exception: " << a.what() << std::endl;
            return -2;
        }
    }
    catch (...) {}

    return -3;
}
