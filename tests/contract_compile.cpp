// Host-only contract check; no firmware or device I/O.
#include "../firmware/solreader_custom/include/solreader/contracts.hpp"
#include <type_traits>

using CompositionHandler = decltype(&solreader::TextComposition::on_composition);
static_assert(std::is_invocable_v<CompositionHandler, solreader::TextComposition &,
                                 const solreader::CompositionEvent &>);
static_assert(!std::is_invocable_v<CompositionHandler, solreader::TextComposition &,
                                  const solreader::InputEvent &>);
static_assert(!std::is_convertible_v<solreader::InputEvent, solreader::CompositionEvent>);
int main() { return 0; }
