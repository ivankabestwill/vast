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

#pragma once

#include <caf/actor.hpp>
#include <caf/actor_system.hpp>
#include <caf/scoped_actor.hpp>
#include <caf/test/dsl.hpp>

#include "vast/system/configuration.hpp"

#include "fixtures/filesystem.hpp"
#include "test.hpp"

namespace fixtures {

/// Configures the actor system of a fixture with default settings for unit
/// testing.
struct test_configuration : vast::system::configuration {
  test_configuration();
};

/// A fixture with an actor system that uses the default work-stealing
/// scheduler.
struct actor_system : filesystem {
  actor_system();

  ~actor_system();

  void enable_profiler();

  auto error_handler() {
    return [&](const caf::error& e) { FAIL(system.render(e)); };
  }

  test_configuration config;
  caf::actor_system system;
  caf::scoped_actor self;
  caf::actor profiler;
};

/// A fixture with an actor system that uses the test coordinator for
/// determinstic testing of actors.
struct deterministic_actor_system
  : test_coordinator_fixture<test_configuration>,
    filesystem {

  deterministic_actor_system();

  auto error_handler() {
    return [&](const caf::error& e) { FAIL(sys.render(e)); };
  }
};

} // namespace fixtures

