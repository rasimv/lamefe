#pragma once

#include "encoderimpl.h"
#include <set>

namespace lamefe
{
    inline String formatString(const Settings &a_output)
    {
        SStream l_ss;
        if (a_output.quality) l_ss << a_output.quality.value();
        else l_ss << "*";
        l_ss << " | ";
        if (a_output.mode) l_ss << (a_output.mode.value() == Settings::SMCBR? "cbr":
                                    a_output.mode.value() == Settings::SMABR? "abr":
                                    a_output.mode.value() == Settings::SMVBR? "vbr": "*");
        else l_ss << "*";
        l_ss << " ";
        if (a_output.value) l_ss << a_output.value.value();
        else l_ss << "*";
        return l_ss.str();
    }

    inline void EncoderImpl::set(Settings &&a)
    {
        // Save encoding settings
        m_settings = std::move(a);
        m_log << formatString(m_settings) << std::endl;
    }

    inline void EncoderImpl::list(std::filesystem::path &&a_dir)
    {
        // List wave-files in the directory
        decltype(m_files) l_files;
        for (const auto &l_entry: std::filesystem::directory_iterator(a_dir))
        {
            if (!l_entry.is_regular_file()) continue;
            const auto l_filepath(l_entry.path());
            if (!l_filepath.has_filename()) continue;
            auto l_filename(l_filepath.filename());
            const auto l_ext(l_filename.extension());
            static const std::set<std::filesystem::path> s_extList{".wav", ".WAV", ".Wav"};
            if (s_extList.find(l_ext) == std::end(s_extList)) continue;
            l_files.push_back(std::move(l_filename));
        }

        m_dir = std::move(a_dir);
        m_files = std::move(l_files);

        if (m_files.size() > 0) m_log << m_files.size();
        else m_log << "No";
        m_log << " wave-file(s) found" << std::endl;
    }
}
