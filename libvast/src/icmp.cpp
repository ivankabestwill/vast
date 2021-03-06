/******************************************************************************
 *                    _   _____   __________                                  *
 *                   | | / / _ | / __/_  __/     Visibility                   *
 *                   | |/ / __ |_\ \  / /          Across                     *
 *                   |___/_/ |_/___/ /_/       Space and Time                 *
 *                                                                            *
 * This file is part of VAST. It is subject to the license terms in the       *
 * LICENSE file found in the top-level directory of this distribution and at  *
 * http://vast.io/license. No part of VAST, including this file, may be       *
 * copied, modified, propagated, or distributed except according to the terms *
 * contained in the LICENSE file.                                             *
 ******************************************************************************/

#include "vast/icmp.hpp"

#include <cstdint>

#include <caf/optional.hpp>

namespace vast {

caf::optional<icmp_type> dual(icmp_type x) {
  switch (x) {
    default:
      return caf::none;
    case icmp_type::echo:
      return icmp_type::echo_reply;
    case icmp_type::echo_reply:
      return icmp_type::echo;
    case icmp_type::tstamp:
      return icmp_type::tstamp_reply;
    case icmp_type::tstamp_reply:
      return icmp_type::tstamp;
    case icmp_type::info:
      return icmp_type::info_reply;
    case icmp_type::info_reply:
      return icmp_type::info;
    case icmp_type::rtr_solicit:
      return icmp_type::rtr_advert;
    case icmp_type::rtr_advert:
      return icmp_type::rtr_solicit;
    case icmp_type::mask:
      return icmp_type::mask_reply;
    case icmp_type::mask_reply:
      return icmp_type::mask;
  }
}

caf::optional<icmp6_type> dual(icmp6_type x) {
  switch (x) {
    default:
      return caf::none;
    case icmp6_type::echo_request:
      return icmp6_type::echo_reply;
    case icmp6_type::echo_reply:
      return icmp6_type::echo_request;
    case icmp6_type::mld_listener_query:
      return icmp6_type::mld_listener_report;
    case icmp6_type::mld_listener_report:
      return icmp6_type::mld_listener_query;
    case icmp6_type::nd_router_solicit:
      return icmp6_type::nd_router_advert;
    case icmp6_type::nd_router_advert:
      return icmp6_type::nd_router_solicit;
    case icmp6_type::nd_neighbor_solicit:
      return icmp6_type::nd_neighbor_advert;
    case icmp6_type::nd_neighbor_advert:
      return icmp6_type::nd_neighbor_solicit;
    case icmp6_type::wru_request:
      return icmp6_type::wru_reply;
    case icmp6_type::wru_reply:
      return icmp6_type::wru_request;
    case icmp6_type::haad_request:
      return icmp6_type::haad_reply;
    case icmp6_type::haad_reply:
      return icmp6_type::haad_request;
  }
}

} // namespace vast
