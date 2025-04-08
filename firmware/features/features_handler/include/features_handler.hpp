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

#include "buttons.hpp"
#include "features_handler_types.hpp"
#include "keys_config.hpp"
#include "storage.hpp"
#include "time.hpp"

#include <memory>
#include <string>
#include <unordered_map>


class Feature {
  public:
    explicit Feature(KeysConfig& keys_config_) : keys_config(keys_config_) {}
    virtual ~Feature() = default;

    virtual void handle(Buttons& buttons)          = 0;
    virtual void init()                            = 0;
    virtual void deinit()                          = 0;
    virtual void factory_init()                    = 0;
    virtual std::string get_log(uint log_id) const = 0;

    virtual FeatureCmdResult get_cmd(const FeatureCommand& command) const {
        (void)command;
        return { FeatureCmdStatus::GET_COMMAND_UNSUPPORTED, std::monostate{} };
    }

    virtual FeatureCmdStatus set_cmd(const FeatureCommand& command) {
        (void)command;
        return FeatureCmdStatus::SET_COMMAND_UNSUPPORTED;
    }

  protected:
    KeysConfig& keys_config;
};

class FeaturesHandler {
  public:
    FeaturesHandler(Storage& storage, KeysConfig& keys_config, Time& time);
    ~FeaturesHandler() = default;

    void init();
    void factory_init();
    void factory_init_features();
    void switch_to_feature(FeatureType type);
    void handle(Buttons& buttons);
    std::string get_feature_log(FeatureType f_type, uint log_id) const;
    FeatureType get_current_feature() const;
    std::string get_current_feature_name() const;

    FeatureCmdResult get_cmd(FeatureType f_type, const FeatureCommand& command) const;
    FeatureCmdStatus set_cmd(FeatureType f_type, const FeatureCommand& command) const;

  private:
    FeaturesHandlerConfig_t config;
    Storage& storage;
    std::unordered_map<FeatureType, std::unique_ptr<Feature>> features;
    KeysConfig& keys_config;
    Time& time;

    void initialize_features();
    bool is_factory_required() const;
};
