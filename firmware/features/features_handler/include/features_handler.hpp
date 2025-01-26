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
#include "keys_config.hpp"
#include "storage.hpp"

#include <memory>
#include <unordered_map>

enum class FeatureType {
    CTRL_C_V,
    TIME_TRACKER,
    NONE,
};

typedef struct {
    uint32_t magic;
    FeatureType current_feature;
    bool is_feature_set;
} FeaturesHandlerConfig_t;

class Feature {
  public:
    explicit Feature(KeysConfig& keys_config) : keys_config(keys_config) {}
    virtual ~Feature() = default;

    // Called on each HID task loop
    virtual void handle(Buttons& buttons)          = 0;
    virtual void init()                            = 0;
    virtual std::string get_log(uint log_id) const = 0;

  protected:
    KeysConfig& keys_config;
};

class FeaturesHandler {
  public:
    FeaturesHandler(Storage& storage, KeysConfig& keys_config);
    ~FeaturesHandler() = default;

    void init();
    void factory_init();
    void switch_to_feature(FeatureType type);
    void handle(Buttons& buttons);
    std::string get_feature_log(FeatureType f_type, uint log_id) const;
    FeatureType get_current_feature() const;

  private:
    FeaturesHandlerConfig_t config;
    Storage& storage;
    std::unordered_map<FeatureType, std::unique_ptr<Feature>> features;
    KeysConfig& keys_config;

    void initialize_features();
    bool is_factory_required() const;
};
