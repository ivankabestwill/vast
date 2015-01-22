#ifndef VAST_ACTOR_REPLICATOR_H
#define VAST_ACTOR_REPLICATOR_H

#include <caf/all.hpp>
#include "vast/actor/actor.h"

namespace vast {

/// Replicates a message by relaying it to a set of workers.
class replicator : public flow_controlled_actor
{
public:
  replicator()
  {
    trap_unexpected(false);
  }

  void at(caf::exit_msg const& msg) override
  {
    workers_.clear();
    quit(msg.reason);
  }

  void at(caf::down_msg const& msg) override
  {
    workers_.erase(
        std::remove_if(
            workers_.begin(),
            workers_.end(),
            [=](caf::actor const& a) { return a == last_sender(); }),
        workers_.end());

    if (workers_.empty())
      quit(msg.reason);
  }

  caf::message_handler make_handler() override
  {
    using namespace caf;

    return
    {
      on(atom("add"), atom("worker"), arg_match) >> [=](actor a)
      {
        VAST_DEBUG(this, "adds worker", a);
        monitor(a);
        workers_.push_back(std::move(a));
      },
      on(atom("workers")) >> [=]
      {
        return workers_;
      },
      others() >> [=]
      {
        for (auto& a : workers_)
          // FIXME: use a method of sending appropriate for 1-n communication.
          //send_tuple_as(last_sender(), a, last_dequeued());
          forward_to(a);
      }
    };
  }

  std::string name() const
  {
    return "replicator";
  }

private:
  std::vector<caf::actor> workers_;
};

} // namespace vast

#endif