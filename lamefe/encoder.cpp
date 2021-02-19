#include "encoder.h"
#include "encoderimpl.h"

namespace lamefe
{
    Encoder::Encoder(OStream &a_log): m_impl(new EncoderImpl(a_log)) {}

    Encoder::~Encoder() { delete m_impl; }

    void Encoder::set(Settings a) { m_impl->set(std::move(a)); }

    void Encoder::list(std::filesystem::path a_dir) { m_impl->list(std::move(a_dir)); }

    void Encoder::run() { m_impl->run(); }
}
