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

#include "vast/system/connect_to_node.hpp"

#include <caf/actor_system.hpp>
#include <caf/actor_system_config.hpp>
#include <caf/io/middleman.hpp>
#include <caf/scoped_actor.hpp>
#include <caf/settings.hpp>

#include "vast/config.hpp"

#ifdef VAST_USE_OPENSSL
#include <caf/openssl/all.hpp>
#endif

#include "vast/concept/parseable/vast/endpoint.hpp"
#include "vast/concept/printable/to_string.hpp"
#include "vast/concept/printable/vast/port.hpp"
#include "vast/defaults.hpp"
#include "vast/endpoint.hpp"
#include "vast/error.hpp"
#include "vast/filesystem.hpp"
#include "vast/logger.hpp"
#include "vast/system/node.hpp"

using namespace caf;

namespace vast::system {

caf::expected<actor>
connect_to_node(scoped_actor& self, const caf::settings& opts) {
  // Fetch values from config.
  auto id = get_or(opts, "system.node-id", defaults::system::node_id);
  auto db_dir
    = get_or(opts, "system.db-directory", defaults::system::db_directory);
  auto abs_dir = path{db_dir}.complete();
  endpoint node_endpoint;
  auto str = get_or(opts, "system.endpoint", defaults::system::endpoint);
  if (!parsers::endpoint(str, node_endpoint))
    return make_error(ec::parse_error, "invalid endpoint", str);
  if (node_endpoint.port.type() == port::port_type::unknown)
    node_endpoint.port.type(port::tcp);
  if (node_endpoint.port.type() != port::port_type::tcp)
    return make_error(ec::invalid_configuration, "invalid protocol",
                      node_endpoint.port);
  VAST_DEBUG(self, "connects to remote node:", id);
  auto& sys_cfg = self->system().config();
  auto use_encryption = !sys_cfg.openssl_certificate.empty()
                        || !sys_cfg.openssl_key.empty()
                        || !sys_cfg.openssl_passphrase.empty()
                        || !sys_cfg.openssl_capath.empty()
                        || !sys_cfg.openssl_cafile.empty();
  auto host = node_endpoint.host;
  if (node_endpoint.host.empty())
    node_endpoint.host = "127.0.0.1";
  VAST_INFO(self, "connects to",
            node_endpoint.host << ':' << to_string(node_endpoint.port));
  if (use_encryption) {
#ifdef VAST_USE_OPENSSL
    return openssl::remote_actor(self->system(), node_endpoint.host,
                                 node_endpoint.port.number());
#else
    return make_error(ec::unspecified, "not compiled with OpenSSL support");
#endif
  }
  auto& mm = self->system().middleman();
  return mm.remote_actor(node_endpoint.host, node_endpoint.port.number());
}

} // namespace vast::system
