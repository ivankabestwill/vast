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

#include "vast/concept/parseable/vast/json.hpp"
#include "vast/defaults.hpp"
#include "vast/detail/line_range.hpp"
#include "vast/error.hpp"
#include "vast/event.hpp"
#include "vast/format/multi_layout_reader.hpp"
#include "vast/format/ostream_writer.hpp"
#include "vast/fwd.hpp"
#include "vast/json.hpp"
#include "vast/logger.hpp"
#include "vast/schema.hpp"

#include <caf/expected.hpp>
#include <caf/fwd.hpp>

namespace vast::format::json {

class writer : public ostream_writer {
public:
  using defaults = vast::defaults::export_::json;

  using super = ostream_writer;

  using super::super;

  caf::error write(const table_slice& x) override;

  const char* name() const override;
};

/// Adds a JSON object to a table slice builder according to a given layout.
/// @param builder The builder to add the JSON object to.
/// @param xs The JSON object to add to *builder.
/// @param layout The record type describing *xs*.
/// @returns An error iff the operation failed.
caf::error add(table_slice_builder& builder, const vast::json::object& xs,
               const record_type& layout);

/// @relates reader
struct default_selector {
  caf::optional<record_type> operator()(const vast::json::object&) {
    return layout;
  }

  caf::error schema(vast::schema sch) {
    auto entry = *sch.begin();
    if (!caf::holds_alternative<record_type>(entry))
      return make_error(ec::invalid_configuration,
                        "only record_types supported for json schema");
    layout = flatten(caf::get<record_type>(entry));
    return caf::none;
  }

  vast::schema schema() const {
    vast::schema result;
    if (layout)
      result.add(*layout);
    return result;
  }

  static const char* name() {
    return "json-reader";
  }

  caf::optional<record_type> layout = caf::none;
};

/// A reader for JSON data. It operates with a *selector* to determine the
/// mapping of JSON object to the appropriate record type in the schema.
template <class Selector = default_selector>
class reader final : public multi_layout_reader {
public:
  using super = multi_layout_reader;

  /// Constructs a JSON reader.
  /// @param table_slice_type The ID for table slice type to build.
  /// @param options Additional options.
  /// @param in The stream of JSON objects.
  explicit reader(caf::atom_value table_slice_type,
                  const caf::settings& options,
                  std::unique_ptr<std::istream> in = nullptr);

  void reset(std::unique_ptr<std::istream> in);

  caf::error schema(vast::schema sch) override;

  vast::schema schema() const override;

  const char* name() const override;

protected:
  caf::error read_impl(size_t max_events, size_t max_slice_size,
                       consumer& f) override;

private:
  using iterator_type = std::string_view::const_iterator;

  Selector selector_;
  std::unique_ptr<std::istream> input_;
  std::unique_ptr<detail::line_range> lines_;
  caf::optional<size_t> proto_field_;
  std::vector<size_t> port_fields_;
};

// -- implementation ----------------------------------------------------------

template <class Selector>
reader<Selector>::reader(caf::atom_value table_slice_type,
                         const caf::settings& /*options*/,
                         std::unique_ptr<std::istream> in)
  : super(table_slice_type) {
  if (in != nullptr)
    reset(std::move(in));
}

template <class Selector>
void reader<Selector>::reset(std::unique_ptr<std::istream> in) {
  VAST_ASSERT(in != nullptr);
  input_ = std::move(in);
  lines_ = std::make_unique<detail::line_range>(*input_);
}

template <class Selector>
caf::error reader<Selector>::schema(vast::schema s) {
  return selector_.schema(std::move(s));
}

template <class Selector>
vast::schema reader<Selector>::schema() const {
  return selector_.schema();
}

template <class Selector>
const char* reader<Selector>::name() const {
  return Selector::name();
}

template <class Selector>
caf::error reader<Selector>::read_impl(size_t max_events, size_t max_slice_size,
                                       consumer& cons) {
  VAST_ASSERT(max_events > 0);
  VAST_ASSERT(max_slice_size > 0);
  size_t produced = 0;
  for (; produced < max_events; lines_->next()) {
    // EOF check.
    if (lines_->done())
      return finish(cons, make_error(ec::end_of_input, "input exhausted"));
    auto& line = lines_->get();
    vast::json j;
    if (!parsers::json(line, j)) {
      VAST_WARNING(this, "failed to parse line", lines_->line_number(), ":",
                   line);
      continue;
    }
    auto xs = caf::get_if<vast::json::object>(&j);
    if (!xs)
      return make_error(ec::type_clash, "not a json object");
    auto layout = selector_(*xs);
    if (!layout)
      continue;
    auto bptr = builder(*layout);
    if (bptr == nullptr)
      return make_error(ec::parse_error, "unable to get a builder");
    if (auto err = add(*bptr, *xs, *layout)) {
      err.context() += caf::make_message("line", lines_->line_number());
      return finish(cons, err);
    }
    produced++;
    if (bptr->rows() == max_slice_size)
      if (auto err = finish(cons, bptr))
        return err;
  }
  return finish(cons);
}

} // namespace vast::format::json
