#pragma once

#include "platform/network_policy.hpp"

namespace network_policy
{
enum Stage
{
  Ask,
  Always,
  Never,
  Today,
  NotToday
};

void SetStage(Stage state);
Stage GetStage();

bool CanUseNetwork();
bool IsAllowedToday();
}  // namespace network_policy
