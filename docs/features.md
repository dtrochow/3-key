# Features

## Adding a New Feature to the Project
This guide provides a step-by-step approach to add and integrate a new feature into the project. Each feature should be implemented as a module under the features directory.

## Project Structure
The `features` directory contains:

- Individual `feature` modules, each in its own folder.
- A `features_handler` module for managing and dispatching features.

## Steps to Add a New Feature

- Create a New Feature Directory
    -  Navigate to the features directory.
    - Create a new folder for the feature. For example, for a feature named new_feature, create:
    ```bash
    features/new_feature
    ```
    - Inside this folder, create the following: 
        - A `CMakeLists.txt` file for the feature.
        - A `new_feature.cpp` file containing the feature implementation.
        - A include directory containing a header file, e.g., `new_feature.hpp`.

- Define the `Feature` Class
    - Define a new class that inherits from the Feature base class in `features_handler.hpp`.
    - Implement the handle method for the feature's behavior.
    - Example: `features/new_feature/include/new_feature.hpp`

    ``` cpp
    #pragma once

    #include "features_handler.hpp"
    #include "buttons.hpp"

    class NewFeature : public Feature {
    public:
        void handle(const Buttons& buttons) override;
    };
    ```

    - Example: `features/new_feature/new_feature.cpp`

    ```cpp
    #include "new_feature.hpp"

    void NewFeature::handle(const Buttons& buttons) {
        // Implement the feature-specific behavior here
    }
    ```

- Update the `FeatureType` Enum
    - Open `features_handler.hpp`.
    - Add a new entry for the feature to the `FeatureType` enum.
    - Example:

    ```cpp
    enum class FeatureType {
        CTRL_C_V,
        NEW_FEATURE, // Add your feature here
        NONE,
    };
    ```

- Register the Feature in `FeaturesHandler`
    - Open `features_handler.cpp`.
    - Update the init method to register the new feature.
    - Example:

    ```cpp
    void FeaturesHandler::initialize_featuresinit() {
        features[FeatureType::CTRL_C_V] = std::make_unique<CtrlCVFeature>(keys_config);
        features[FeatureType::NEW_FEATURE] = std::make_unique<NewFeature>(keys_config);
    }
    ```

- Integrate the Feature with the `Terminal`
    - Open `terminal.cpp`.
    - Update the handle_feature_command method to parse the new feature name and map it to `FeatureType`.
    - Example:

    ```cpp
    bool Terminal::handle_feature_command(const std::vector<std::string>& params) {
        if (params.size() != 1) {
            add_log("Error: feature requires exactly 1 parameter");
            return false;
        }

        const std::string& feature_name = params[0];
        FeatureType feature;

        if (feature_name == "ctrl_c_v") {
            feature = FeatureType::CTRL_C_V;
        } else if (feature_name == "new_feature") { // Add parsing logic
            feature = FeatureType::NEW_FEATURE;
        } else {
            add_log("Error: Unknown feature");
            return false;
        }

        add_log("Feature enabled: " + feature_name);

        // Logic to switch to the feature
        features_handler.switch_to_feature(feature);

        return true;
    }
    ```

- Create the Feature’s `CMakeLists.txt`
    - Example: `features/new_feature/CMakeLists.txt`

    ```cmake
    # Define the new feature module
    add_library(new_feature STATIC new_feature.cpp)

    # Include the necessary directories
    target_include_directories(new_feature PUBLIC include)

    # Link dependencies
    target_link_libraries(new_feature PUBLIC features_handler)
    ```

- Update the Parent `CMakeLists.txt`
    - Open `features/CMakeLists.txt`.
    - Add the new feature module.
    - Example:

    ```cmake
    add_subdirectory(ctrl_c_v)
    add_subdirectory(new_feature) # Add your new feature here
    ```

- Test the Feature
    - Build the project using the top-level CMakeLists.txt.
    - Test the new feature using the terminal interface by executing the command:
    ``` php
    feature <feature_name>
    ```
    - Replace <feature_name> with the feature name, e.g., new_feature.

- Future Expansion
    - To add more features, repeat the steps above for each new feature.
    - Use the `FeatureType` enum and `FeaturesHandler` to manage feature-specific behavior efficiently.

