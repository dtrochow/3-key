/*
 * 3-key Project
 *
 * This file is part of the 3-key project.
 *
 * Copyright (C) 2025 Dominik Trochowski <dominik.trochowski@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <optional>
#include <variant>

#include "features_handler.hpp"
#include "time_tracker_types.hpp"


enum class FeatureCmdStatus_e : uint8_t {
    Success,
    SetCommandUnsupported,
    GetCommandUnsupported,
    InvalidPayload,
    InvalidCommand,
    Error,
};

/* -------------------------------------------------------------------------- */
/*                        Time Tracker Feature Commands                       */
/* -------------------------------------------------------------------------- */

struct GetTimeTrackerEntryCmd {
    uint32_t session_id;
};

struct GetTimeTrackerCurrentActiveSessionIdCmd {};

struct NewTimeTrackerSessionCmd {};

struct SetTimeTrackerMediumThresholdCmd {
    uint32_t threshold_ms;
};

struct SetTimeTrackerLongThresholdCmd {
    uint32_t threshold_ms;
};

/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                          Feature Command Responses                         */
/* -------------------------------------------------------------------------- */

// clang-format off
using FeatureCommand =
    std::variant<GetTimeTrackerEntryCmd,
                 GetTimeTrackerCurrentActiveSessionIdCmd,
                 NewTimeTrackerSessionCmd,
                 SetTimeTrackerMediumThresholdCmd,
                 SetTimeTrackerLongThresholdCmd>;

// Define possible return types for get_cmd
using FeatureCmdResultVariant = std::variant<std::monostate, TimeTrackingEntry_t, SessionId>;
using FeatureCmdResult        = std::pair<FeatureCmdStatus_e, FeatureCmdResultVariant>;
// clang-format on

/* -------------------------------------------------------------------------- */

enum class FeatureType_e : uint8_t {
    CtrlCV,
    TimeTracker,
    FeatureNone,
};

struct FeaturesHandlerConfig_t {
    uint32_t magic;
    FeatureType_e current_feature;
    bool is_feature_set;
};
