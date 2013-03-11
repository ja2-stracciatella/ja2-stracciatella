#ifndef MICROINI_PROPERTIES_HPP
#define MICROINI_PROPERTIES_HPP

#include <MicroIni/Container.hpp>
#include <MicroIni/Line.hpp>

namespace MicroIni
{

/// A Container of \link Line Lines\endlink.
class MICROINI_API Properties : public Container<Line>
{};

} // namespace MicroIni

#endif // MICROINI_PROPERTIES_HPP
