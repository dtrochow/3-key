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

#include <limits>
#include <pico/types.h>

#include "buttons_config.hpp"
#include "keys_config.hpp"
#include "leds_config.hpp"
#include "storage_config.hpp"
#include "time.hpp"
#include "time_tracker.hpp"
#include "time_tracker_types.hpp"

static repeating_timer_t* tracking_timer = nullptr;

bool TimeTracker::timer_callback(repeating_timer_t* timer) {
    auto* tracker = static_cast<TimeTracker*>(timer->user_data);
    if (!tracker)
        return false;

    constexpr uint64_t elapsed_time_us = (kTracingTimerIntervalMs * kMicrosecondsInMillisecondCount);
    auto& entry = tracker->data.tracking_entries[tracker->data.active_session];
    if (entry.tracking_work) {
        entry.work_time_us += elapsed_time_us;
    } else if (entry.tracking_meetings) {
        entry.meeting_time_us += elapsed_time_us;
    }

    const uint64_t total_tracked_time_ms = get_milliseconds_tracked(entry);
    const uint64_t medium_threshold      = tracker->data.medium_threshold_ms;
    const uint64_t long_threshold        = tracker->data.long_threshold_ms;
    if ((total_tracked_time_ms >= medium_threshold) && !entry.medium_threshold_reached) {
        tracker->led_enable(kFunctionKeyId, Color_e::Yellow);
        entry.medium_threshold_reached = true;
    } else if ((total_tracked_time_ms >= long_threshold) && !entry.long_threshold_reached) {
        tracker->led_enable(kFunctionKeyId, Color_e::Red);
        entry.long_threshold_reached = true;
    }

    if (tracker->is_time_to_save()) {
        tracker->save_tracking_data();
        tracker->zero_intervals_count();
    } else {
        tracker->increment_intervals_count();
    }

    return true;
}

void TimeTracker::init() {
    storage.get_blob(BlobType_e::TimeTrackerData, data);
    if (is_factory_required())
        factory_init();

    disable_all_leds();
    stop_tracking();
    check_thresholds();

    keys_config.switch_leds_mode(LedsMode_e::HandledByFeature);
    set_tracking_date();

    tracking_timer = new repeating_timer_t;
    add_repeating_timer_ms(kTracingTimerIntervalMs, TimeTracker::timer_callback, this, tracking_timer);
}

void TimeTracker::deinit() {
    keys_config.switch_leds_mode(LedsMode_e::WhenButtonPressed);
    disable_all_leds();

    if (tracking_timer) {
        cancel_repeating_timer(tracking_timer);
        delete tracking_timer;
        tracking_timer = nullptr;
    }
}

void TimeTracker::factory_init() {
    data.magic               = kBlobMagicNumber;
    data.medium_threshold_ms = kMediumThresholdMsDefault;
    data.long_threshold_ms   = kLongThresholdMsDefault;

    for (auto& entry : data.tracking_entries) {
        entry.start_time_us            = 0;
        entry.work_time_us             = 0;
        entry.meeting_time_us          = 0;
        entry.tracking_work            = false;
        entry.tracking_meetings        = false;
        entry.tracking_date            = {};
        entry.medium_threshold_reached = false;
        entry.long_threshold_reached   = false;
    }

    data.active_session = 0;

    save_tracking_data();
}

bool TimeTracker::is_factory_required() const {
    return (data.magic != kBlobMagicNumber);
}

void TimeTracker::stop_tracking() {
    previous_tracking_type = TrackingType_e::None;
    auto& entry            = data.tracking_entries[data.active_session];
    if (entry.tracking_work) {
        entry.tracking_work    = false;
        previous_tracking_type = TrackingType_e::WorkTracking;
        led_disable(kWorkTrackingKeyId);
    } else if (entry.tracking_meetings) {
        entry.tracking_meetings = false;
        previous_tracking_type  = TrackingType_e::MeetingTracking;
        led_disable(kMeetingTrackingKeyId);
    }
}

void TimeTracker::resume_tracking() {
    auto& entry = data.tracking_entries[data.active_session];
    if (previous_tracking_type == TrackingType_e::WorkTracking) {
        entry.tracking_work = true;
        const Color_e color = get_key_color_info(kWorkTrackingKeyId)->color;
        led_enable(kWorkTrackingKeyId, color);
    } else if (previous_tracking_type == TrackingType_e::MeetingTracking) {
        entry.tracking_meetings = true;
        const Color_e color     = get_key_color_info(kMeetingTrackingKeyId)->color;
        led_enable(kMeetingTrackingKeyId, color);
    }
}

void TimeTracker::check_thresholds() {
    const auto& entry = data.tracking_entries[data.active_session];
    if (entry.medium_threshold_reached) {
        led_enable(kFunctionKeyId, Color_e::Yellow);
    }
    if (entry.long_threshold_reached) {
        led_enable(kFunctionKeyId, Color_e::Red);
    }
}

void TimeTracker::set_tracking_date() {
    auto& entry = data.tracking_entries[data.active_session];
    if (is_date_empty(entry.tracking_date))
        entry.tracking_date = time.get_current_date_and_time();
}

void TimeTracker::save_buttons_state() {
    saved_buttons_state = keys_config.get_key_cfgs();
}

void TimeTracker::restore_buttons_state() {
    disable_all_leds();
    for (uint id = 0; id < saved_buttons_state.size(); ++id) {
        const auto& button = saved_buttons_state[id];
        if (button.enabled)
            led_enable(id, button.color);
    }
    saved_buttons_state = {};
}

bool TimeTracker::is_next_slot_empty() const {
    SessionId next_session = data.active_session + 1;
    if (next_session >= kMaxTimeTrackerEntriesCount) {
        next_session = 0;
    }
    auto& entry = data.tracking_entries[next_session];
    return ((entry.tracking_meetings == 0) && (entry.tracking_work == 0));
}

void TimeTracker::handle_key_0_press(auto& entry, const bool is_long_press) {
    if (!is_long_press) {
        if (awaiting_confirmation) {
            /* Cancel moving to next session */
            awaiting_confirmation = false;
            restore_buttons_state();
            resume_tracking();
            return;
        }

        if (entry.tracking_work) {
            entry.tracking_work = false;
            led_disable(kWorkTrackingKeyId);
        } else {
            if (entry.tracking_meetings) {
                entry.tracking_meetings = false;
                led_disable(kMeetingTrackingKeyId);
            }
            entry.tracking_work = true;
            const Color_e color = get_key_color_info(kWorkTrackingKeyId)->color;
            led_enable(kWorkTrackingKeyId, color);
        }
    }
}

void TimeTracker::handle_key_1_press(auto& entry, const bool is_long_press) {
    if (awaiting_confirmation) {
        return;
    }
    if (!is_long_press) {
        if (entry.tracking_meetings) {
            entry.tracking_meetings = false;
            led_disable(kMeetingTrackingKeyId);
        } else {
            if (entry.tracking_work) {
                entry.tracking_work = false;
                led_disable(kWorkTrackingKeyId);
            }
            entry.tracking_meetings = true;
            const Color_e color     = get_key_color_info(kMeetingTrackingKeyId)->color;
            led_enable(kMeetingTrackingKeyId, color);
        }
    } else {
        /*
            Show current session ID by blinking the LED 0 & 1
            LED 0 - tens part
            LED 1 - ones part
        */
        save_buttons_state();
        disable_all_leds();

        const uint session_id           = data.active_session;
        const uint led_0                = session_id / 10;
        const uint led_1                = session_id % 10;
        constexpr uint led_blink_period = 500;
        led_blink(kWorkTrackingKeyId, led_blink_period, led_0, Color_e::Green);
        led_blink(kMeetingTrackingKeyId, led_blink_period, led_1, Color_e::Green);

        restore_buttons_state();
    }
}

void TimeTracker::handle_key_2_press(const bool is_long_press) {
    Color_e color = get_key_color_info(kFunctionKeyId)->color;
    if (is_long_press) {
        if (awaiting_confirmation)
            return;
        if (!is_next_slot_empty()) {
            /* Wait for next session confirmation */
            awaiting_confirmation = true;
            save_buttons_state();
            disable_all_leds();
            stop_tracking();
            led_enable(kWorkTrackingKeyId, Color_e::Red);
            led_enable(kFunctionKeyId, Color_e::Green);
            return;
        }

        move_to_next_session();
    } else {
        if (awaiting_confirmation) {
            /* Confirm moving to next session */
            awaiting_confirmation = false;
            move_to_next_session();
            return;
        }

        if (!is_any_threshold_reached()) {
            /* Tracked hours indicator */
            const uint hours = get_hours_tracked();
            if (hours > 0) {
                led_blink(kFunctionKeyId, 800, hours, Color_e::Green);
            } else {
                led_blink(kFunctionKeyId, 800, 1, Color_e::Green);
            }
        }
    }
}

void TimeTracker::initialize_new_session() {
    auto& entry                    = data.tracking_entries[data.active_session];
    entry.start_time_us            = 0;
    entry.work_time_us             = 0;
    entry.meeting_time_us          = 0;
    entry.tracking_work            = false;
    entry.tracking_meetings        = false;
    entry.tracking_date            = time.get_current_date_and_time();
    entry.medium_threshold_reached = false;
    entry.long_threshold_reached   = false;
}

void TimeTracker::move_to_next_session(bool animate) {
    if (data.active_session < (kMaxTimeTrackerEntriesCount - 1)) {
        data.active_session++;
    } else {
        data.active_session = 0;
    }
    stop_tracking();
    disable_all_leds();
    initialize_new_session();
    if (animate) {
        next_session_animation(Color_e::Green);
    }
}

void TimeTracker::tracker(const uint key_id, const bool is_long_press) {
    auto& entry = data.tracking_entries[data.active_session];

    set_tracking_date();

    switch (key_id) {
        case kWorkTrackingKeyId: handle_key_0_press(entry, is_long_press); break;
        case kMeetingTrackingKeyId: handle_key_1_press(entry, is_long_press); break;
        case kFunctionKeyId: handle_key_2_press(is_long_press); break;
        /* Unexpected key_id */
        default: return;
    }
}

void TimeTracker::handle(Buttons& buttons) {
    const auto pressed_key = buttons.get_pending_button();
    if (pressed_key.has_value()) {
        const ButtonState_t button_state = pressed_key.value();
        const uint key_id                = button_state.key_id;
        const bool is_long_press         = button_state.is_long_press;
        tracker(key_id, is_long_press);
    }
}

std::string TimeTracker::get_log(uint log_id) const {
    std::string log;
    auto& entry = data.tracking_entries[data.active_session];
    switch (static_cast<TimeTrackerLog_e>(log_id)) {
        case TimeTrackerLog_e::CurrentWorkTimeReport: {
            const uint64_t total_seconds = entry.work_time_us / kMicrosecondsInSecondCount;
            const uint64_t hours         = (total_seconds / kSecondsInHourCount);
            const uint64_t minutes = ((total_seconds % kSecondsInHourCount) / kSecondsInMinuteCount);
            const uint64_t seconds = (total_seconds % kSecondsInMinuteCount);

            log = time.get_current_date_and_time_string() + " Work: " + std::to_string(hours) +
                "h " + std::to_string(minutes) + "min " + std::to_string(seconds) + "s";
        } break;
        case TimeTrackerLog_e::CurrentMeetingsTimeReport: {
            const uint64_t total_seconds = entry.meeting_time_us / kMicrosecondsInSecondCount;
            const uint64_t hours         = (total_seconds / kSecondsInHourCount);
            const uint64_t minutes = ((total_seconds % kSecondsInHourCount) / kSecondsInMinuteCount);
            const uint64_t seconds = (total_seconds % kSecondsInMinuteCount);

            log = time.get_current_date_and_time_string() + " Meetings: " + std::to_string(hours) +
                "h " + std::to_string(minutes) + "min " + std::to_string(seconds) + "s";
        } break;
        case TimeTrackerLog_e::CurrentSessionId:
            log = "Current session ID: " + std::to_string(data.active_session);
            break;
        default: log = "Invalid log ID";
    }
    return log;
}

bool TimeTracker::is_date_empty(DateTime_t& date_time) const {
    return ((date_time.day == 0) && (date_time.hour == 0) && (date_time.minute == 0) &&
        (date_time.year == 0) && (date_time.month == 0) && (date_time.day == 0));
}

FeatureCmdResult TimeTracker::get_cmd(const FeatureCommand& command) const {
    if (std::holds_alternative<GetTimeTrackerEntryCmd>(command)) {
        const uint32_t session_id = std::get<GetTimeTrackerEntryCmd>(command).session_id;
        /* Current session case */
        if (session_id == uint32_t(-1)) {
            return { FeatureCmdStatus_e::Success, data.tracking_entries[data.active_session] };
        } else {
            if (session_id >= kMaxTimeTrackerEntriesCount) {
                return { FeatureCmdStatus_e::InvalidPayload, std::monostate{} };
            }
            return { FeatureCmdStatus_e::Success, data.tracking_entries[session_id] };
        }
    } else if (std::holds_alternative<GetTimeTrackerCurrentActiveSessionIdCmd>(command)) {
        return { FeatureCmdStatus_e::Success, data.active_session };
    }
    return { FeatureCmdStatus_e::InvalidCommand, std::monostate{} };
}

FeatureCmdStatus_e TimeTracker::set_cmd(const FeatureCommand& command) {
    if (std::holds_alternative<NewTimeTrackerSessionCmd>(command)) {
        move_to_next_session(false);
        return FeatureCmdStatus_e::Success;
    } else if (std::holds_alternative<SetTimeTrackerMediumThresholdCmd>(command)) {
        const auto threshold_ms = std::get<SetTimeTrackerMediumThresholdCmd>(command).threshold_ms;
        if (threshold_ms > std::numeric_limits<uint64_t>::max()) {
            return FeatureCmdStatus_e::InvalidPayload;
        }
        data.medium_threshold_ms = threshold_ms;
        save_tracking_data();
        return FeatureCmdStatus_e::Success;
    } else if (std::holds_alternative<SetTimeTrackerLongThresholdCmd>(command)) {
        const auto threshold_ms = std::get<SetTimeTrackerLongThresholdCmd>(command).threshold_ms;
        if (threshold_ms > std::numeric_limits<uint64_t>::max()) {
            return FeatureCmdStatus_e::InvalidPayload;
        }
        data.long_threshold_ms = threshold_ms;
        save_tracking_data();
        return FeatureCmdStatus_e::Success;
    } else if (std::holds_alternative<GetTimeTrackerEntryCmd>(command)) {
        return FeatureCmdStatus_e::SetCommandUnsupported;
    } else if (std::holds_alternative<GetTimeTrackerCurrentActiveSessionIdCmd>(command)) {
        return FeatureCmdStatus_e::GetCommandUnsupported;
    }
    return FeatureCmdStatus_e::InvalidCommand;
}
