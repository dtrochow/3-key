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

FeaturesHandler::FeaturesHandler(Storage& storage, KeysConfig& keys_config)
: storage(storage), keys_config(keys_config) {}

void FeaturesHandler::init() {
    (void)storage.get_blob(BlobType::FEATURES_HANDLER_CONFIG, config);

    initialize_features();

    if (is_factory_required()) {
        factory_init();
    }
}

void FeaturesHandler::initialize_features() {
    features[FeatureType::CTRL_C_V] = std::make_unique<CtrlCVFeature>(keys_config);
}

void FeaturesHandler::factory_init() {
    config.magic = BLOB_MAGIC;
    /* Setting default feature */
    switch_to_feature(FeatureType::CTRL_C_V);
    (void)storage.save_blob(BlobType::FEATURES_HANDLER_CONFIG, config);
}

bool FeaturesHandler::is_factory_required() const {
    return (config.magic != BLOB_MAGIC);
}

void FeaturesHandler::switch_to_feature(FeatureType type) {
    FeaturesHandlerConfig_t conifg_cpy = config;

    config.is_feature_set  = (type == FeatureType::NONE) ? false : true;
    config.current_feature = type;

    if ((features.find(type) == features.end()) && (type != FeatureType::NONE)) {
        config = conifg_cpy;
        return;
    }

    if (type != FeatureType::NONE)
        features[type]->init();

    (void)storage.save_blob(BlobType::FEATURES_HANDLER_CONFIG, config);
}

void FeaturesHandler::handle(const Buttons& buttons) {
    if (!config.is_feature_set)
        return;

    auto it = features.find(config.current_feature);
    if (it == features.end())
        return;

    it->second->handle(buttons);
}
