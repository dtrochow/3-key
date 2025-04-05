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

#include "features_handler.hpp"
#include "features.hpp"
#include "keys_config.hpp"
#include "storage.hpp"
#include "time.hpp"
#include "time_tracker.hpp"

FeaturesHandler::FeaturesHandler(Storage& storage, KeysConfig& keys_config, Time& time)
: storage(storage), keys_config(keys_config), time(time) {}

void FeaturesHandler::init() {
    (void)storage.get_blob(BlobType::FEATURES_HANDLER_CONFIG, config);

    initialize_features();

    if (is_factory_required()) {
        factory_init();
    }

    if (config.is_feature_set)
        features[config.current_feature]->init();
}

void FeaturesHandler::initialize_features() {
    features[FeatureType::CTRL_C_V]     = std::make_unique<CtrlCVFeature>(keys_config);
    features[FeatureType::TIME_TRACKER] = std::make_unique<TimeTracker>(keys_config, storage, time);
}

void FeaturesHandler::factory_init() {
    config.magic = BLOB_MAGIC;
    /* Setting default feature */
    switch_to_feature(FeatureType::CTRL_C_V);
    (void)storage.save_blob(BlobType::FEATURES_HANDLER_CONFIG, config);
}

void FeaturesHandler::factory_init_features() {
    for (auto& feature : features) {
        feature.second->deinit();
        feature.second->factory_init();
    }
}

bool FeaturesHandler::is_factory_required() const {
    return (config.magic != BLOB_MAGIC);
}

void FeaturesHandler::switch_to_feature(FeatureType type) {
    FeaturesHandlerConfig_t conifg_cpy = config;

    if (config.is_feature_set) {
        auto it = features.find(config.current_feature);
        if (it != features.end()) {
            it->second->deinit();
        }
    }

    config.is_feature_set  = (type == FeatureType::NONE) ? false : true;
    config.current_feature = type;

    /* Feature not found */
    if ((features.find(type) == features.end()) && (type != FeatureType::NONE)) {
        config = conifg_cpy;
        return;
    }

    if (type != FeatureType::NONE)
        features[type]->init();

    (void)storage.save_blob(BlobType::FEATURES_HANDLER_CONFIG, config);
}

void FeaturesHandler::handle(Buttons& buttons) {
    if (!config.is_feature_set)
        return;

    auto it = features.find(config.current_feature);
    if (it == features.end())
        return;

    it->second->handle(buttons);
}

std::string FeaturesHandler::get_feature_log(FeatureType f_type, uint log_id) const {
    const auto& feature = features.at(f_type);
    return feature->get_log(log_id);
}

FeatureType FeaturesHandler::get_current_feature() const {
    return config.current_feature;
}

std::string FeaturesHandler::get_current_feature_name() const {
    switch (config.current_feature) {
        case FeatureType::CTRL_C_V: return "ctrl_c_v";
        case FeatureType::TIME_TRACKER: return "time-tracker";
        case FeatureType::NONE: return "none";
    }
    return "unknown";
}
