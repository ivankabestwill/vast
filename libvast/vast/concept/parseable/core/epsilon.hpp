#pragma once

#include "vast/concept/parseable/core/parser.hpp"

namespace vast {

class epsilon_parser : public parser<epsilon_parser> {
public:
  using attribute = unused_type;

  template <class Iterator, class Attribute>
  bool parse(Iterator&, const Iterator&, Attribute&) const {
    return true;
  }
};

namespace parsers {

auto const eps = epsilon_parser{};

} // namespace parsers
} // namespace vast


