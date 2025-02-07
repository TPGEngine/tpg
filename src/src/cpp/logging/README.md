# Event-Driven Logging Architecture for TPG

## Overview

This document describes the event-driven architecture implemented for logging data points to CSV files during the training process. This system captures metrics from various stages of the training generation, such as replacement, selection, and timing.

### Motivation for Event-Driven Architecture

The TPG RL framework does not inherently store many of the desired metrics as state variables. Instead, these metrics are often calculated or become available only after specific events occur during the training process. An event-driven architecture was chosen to address this, allowing us to:

*   **Capture data at the point of occurrence:** Log metrics immediately after the relevant event, ensuring accuracy and completeness.
*   **Decouple logging from core logic:** Avoid cluttering the core TPG code with logging concerns, promoting cleaner and more maintainable code.
*   **Flexibility and Extensibility:** Easily add or modify logging for different stages without impacting other parts of the system.

## File Structure

The logging library is organized into the following directories:

*   **`core/`:** Contains core components that are fundamental to the logging architecture. This includes:
    * `event_types.h`: Defines the `EventType` enum, which lists all possible events that can be logged.
    * `event_dispatcher.h`:  Header file for the `EventDispatcher` class, a singleton responsible for managing event subscriptions and notifications.
    * `event_dispatcher.tpp`: Implements the template `EventDispatcher` class
*   **`loggers/`:** Contains logger classes, each responsible for subscribing to a specific event and handling the associated metrics.  Each stage has its own subdirectory within `loggers/`.
    * `loggers/<stage>/<stage>_logger.h`:  Header file for the logger class for a specific stage (e.g., `loggers/replacement/replacement_logger.h`).
    * `loggers/<stage>/<stage>_logger.cc`:  Source file for the logger class for a specific stage (e.g., `loggers/replacement/replacement_logger.cc`).
*   **`metrics/`:** Contains the definitions for the metrics structures and builders. Each stage has its own subdirectory within `metrics/`.
    *   `metrics/<stage>/<stage>_metrics.h`:  Header file defining the metrics `struct` for a specific stage (e.g., `metrics/replacement/replacement_metrics.h`).
    *   `metrics/<stage>/<stage>_metrics_builder.h`:  Header file for the metrics builder class for a specific stage (e.g., `metrics/replacement/replacement_metrics_builder.h`).
    *   `metrics/<stage>/<stage>_metrics_builder.cc`:  Source file for the metrics builder class for a specific stage (e.g., `metrics/replacement/replacement_metrics_builder.cc`).
*   **`storage/`:** Contains storage classes responsible for writing the metrics data to CSV files. Each stage has its own subdirectory within `storage/`.
    *   `storage/<stage>/<stage>_storage.h`:  Header file for the storage class for a specific stage (e.g., `storage/replacement/replacement_storage.h`).
    *   `storage/<stage>/<stage>_storage.cc`:  Source file for the storage class for a specific stage (e.g., `storage/replacement/replacement_storage.cc`).
    *   `storage/csv_storage.h`: Defines the base `CSVStorage` class, which provides common functionality for writing data to CSV files.

## Architecture Components

The logging system consists of the following key components:

1.  **Event Types (`logging/core/event_types.h`):** Defines an enumeration of all possible events that can be logged (e.g., `REPLACEMENT`, `SELECTION`, `TMS`).
2.  **Metrics Structures (`logging/metrics/<stage>/<stage>_metrics.h`):**  Defines a `struct` to hold the specific metrics associated with a particular event/stage.  For example, `ReplacementMetrics` stores metrics related to the replacement stage.
3.  **Metrics Builders (`logging/metrics/<stage>/<stage>_metrics_builder.h` and .cc):** Provides a builder pattern to construct the metrics struct in a controlled and readable manner.
4.  **Storage Classes (`logging/storage/<stage>/<stage>_storage.h` and .cc):** Responsible for writing the metrics data to a CSV file.  Each stage has its own storage class (e.g., `ReplacementStorage`).  These classes inherit from a generic `CSVStorage` class (not shown in the provided code, but assumed to exist).
5.  **Loggers (`logging/loggers/<stage>/<stage>_logger.h` and .cc):**  Subscribes to specific events via an `EventDispatcher` and, upon receiving an event, retrieves the associated metrics and passes them to the corresponding storage class for writing to the CSV file.
6.  **Event Dispatcher (`logging/core/event_dispatcher.h` - Not shown):** A singleton class responsible for managing event subscriptions and notifying subscribers when an event occurs.  It's a template class, parameterized by the type of data associated with the event.

## Adding Logging for a New Stage

This section provides a step-by-step guide on how to add logging for a new stage in the TPG RL framework.  We'll use the "Replacement" stage as an example, but remember to replace "Replacement" with the name of your new stage.

**1. Define a New Event Type**

*   **File:** `logging/core/event_types.h`
*   Add a new entry to the `EventType` enum representing your new stage.

    ```c++
    // EventTypes.h
    #pragma once
    #include <map>
    #include <string>

    enum class EventType {
        REPLACEMENT,
        SELECTION,
        TMS,
        // Add your new event here:
        YOUR_NEW_STAGE
    };
    ```

**2. Create a Metrics Structure**

*   **File:** `logging/metrics/<stage>/<stage>_metrics.h` (e.g., `logging/metrics/your_new_stage/your_new_stage_metrics.h`)
*   Create a `struct` to hold the metrics you want to log for your new stage.

    ```c++
    // your_new_stage_metrics.h
    #ifndef YOUR_NEW_STAGE_METRICS_H
    #define YOUR_NEW_STAGE_METRICS_H

    #include <cstddef>
    #include <cstdint>

    struct YourNewStageMetrics {
        YourNewStageMetrics(long generation, size_t metric1, double metric2)
            :   generation(generation),
                metric1(metric1),
                metric2(metric2) {}

        const long generation;
        const std::size_t metric1;
        const double metric2;
    };

    #endif
    ```

**3. Create a Metrics Builder**

*   **Files:** `logging/metrics/<stage>/<stage>_metrics_builder.h` and `logging/metrics/<stage>/<stage>_metrics_builder.cc` (e.g., `logging/metrics/your_new_stage/your_new_stage_metrics_builder.h` and `.cc`)
*   Implement a builder class to construct instances of your metrics struct.

    **Header File (`your_new_stage_metrics_builder.h`):**

    ```c++
    // your_new_stage_metrics_builder.h
    #ifndef YOUR_NEW_STAGE_METRICS_BUILDER_H
    #define YOUR_NEW_STAGE_METRICS_BUILDER_H

    #include "your_new_stage_metrics.h"

    class YourNewStageMetricsBuilder {
    public:
      YourNewStageMetricsBuilder& with_generation(long generation);
      YourNewStageMetricsBuilder& with_metric1(std::size_t metric1);
      YourNewStageMetricsBuilder& with_metric2(double metric2);

      YourNewStageMetrics build() const;

    private:
      long generation_;
      std::size_t metric1_;
      double metric2_;
    };

    #endif
    ```

    **Source File (`your_new_stage_metrics_builder.cc`):**

    ```c++
    // your_new_stage_metrics_builder.cc
    #include "your_new_stage_metrics_builder.h"

    YourNewStageMetricsBuilder& YourNewStageMetricsBuilder::with_generation(
        long generation) {
      generation_ = generation;
      return *this;
    }

    YourNewStageMetricsBuilder& YourNewStageMetricsBuilder::with_metric1(
        std::size_t metric1) {
      metric1_ = metric1;
      return *this;
    }

    YourNewStageMetricsBuilder& YourNewStageMetricsBuilder::with_metric2(
        double metric2) {
      metric2_ = metric2;
      return *this;
    }

    YourNewStageMetrics YourNewStageMetricsBuilder::build() const {
      return YourNewStageMetrics(generation_, metric1_, metric2_);
    }
    ```

**4. Create a Storage Class**

*   **Files:** `logging/storage/<stage>/<stage>_storage.h` and `logging/storage/<stage>/<stage>_storage.cc` (e.g., `logging/storage/your_new_stage/your_new_stage_storage.h` and `.cc`)
*   Create a storage class responsible for writing the metrics to a CSV file.

    **Header File (`your_new_stage_storage.h`):**

    ```c++
    // your_new_stage_storage.h
    #pragma once
    #include "storage/csv_storage.h"
    #include "metrics/your_new_stage/your_new_stage_metrics.h"

    class YourNewStageStorage : public CSVStorage<YourNewStageMetrics> {
    public:
      static YourNewStageStorage& instance() {
        static YourNewStageStorage instance;
        return instance;
      }

      void init(const int& seed_tpg, const int& pid) override;
      void append(const YourNewStageMetrics& metrics) override;

      // Disable copy and move operations.
      YourNewStageStorage(const YourNewStageStorage&) = delete;
      YourNewStageStorage& operator=(const YourNewStageStorage&) = delete;
      YourNewStageStorage(YourNewStageStorage&&) = delete;
      YourNewStageStorage& operator=(YourNewStageStorage&&) = delete;
      ~YourNewStageStorage() override = default;

    private:
      YourNewStageStorage() = default;
    };
    ```

    **Source File (`your_new_stage_storage.cc`):**

    ```c++
    // your_new_stage_storage.cc
    #include "your_new_stage_storage.h"
    #include <iomanip>
    #include <sstream>

    void YourNewStageStorage::init(const int& seed_tpg, const int& pid) {
      std::stringstream filename;
      filename << "your_new_stage." << seed_tpg << "." << pid << ".csv";

      file_.open(filename.str());
      file_ << "generation,metric1,metric2\n";
      file_.flush();
    }

    void YourNewStageStorage::append(const YourNewStageMetrics& metrics) {
      file_ << metrics.generation << "," << metrics.metric1 << ","
            << metrics.metric2 << "\n";
      file_.flush();
    }
    ```

**5. Create a Logger Class**

*   **Files:** `logging/loggers/<stage>/<stage>_logger.h` and `logging/loggers/<stage>/<stage>_logger.cc` (e.g., `logging/loggers/your_new_stage/your_new_stage_logger.h` and `.cc`)
*   Create a logger class that subscribes to the event and writes the metrics to the storage.

    **Header File (`your_new_stage_logger.h`):**

    ```c++
    // your_new_stage_logger.h
    #pragma once
    #include "storage/your_new_stage/your_new_stage_storage.h"
    #include "core/event_dispatcher.h"
    #include "metrics/your_new_stage/your_new_stage_metrics.h"

    class YourNewStageLogger {
    public:
      YourNewStageLogger() = default;
      void init();

    private:
      void handleEvent(const YourNewStageMetrics& metrics);
    };
    ```

    **Source File (`your_new_stage_logger.cc`):**

    ```c++
    // your_new_stage_logger.cc
    #include "your_new_stage_logger.h"
    #include "core/event_dispatcher.h"
    #include "core/event_types.h"
    #include "storage/your_new_stage/your_new_stage_storage.h"

    void YourNewStageLogger::init() {
      EventDispatcher<YourNewStageMetrics>::instance().subscribe(
          EventType::YOUR_NEW_STAGE,
          [this](const YourNewStageMetrics& data) { handleEvent(data); });
    }

    void YourNewStageLogger::handleEvent(const YourNewStageMetrics& metrics) {
      YourNewStageStorage::instance().append(metrics);
    }
    ```

**6. Instantiate the Logger in `TPGExperimentMPI.cc`**

*   **File:** `TPGExperimentMPI.cc`
*   Instantiate and initialize the logger and storage classes.

    ```c++
       if (tpg.GetParam<int>("replay") == 0) {
          // Initialize logger classes
          SelectionStorage::instance().init(seed_tpg, pid);
          SelectionLogger selectionLogger;
          selectionLogger.init();
          TimingStorage::instance().init(seed_tpg, pid);
          TimingLogger timingLogger;
          timingLogger.init();
          ReplacementStorage::instance().init(seed_tpg, pid);
          ReplacementLogger replacementLogger;
          replacementLogger.init();

          // Initialize your new stage logger here:
          YourNewStageStorage::instance().init(seed_tpg, pid);
          YourNewStageLogger yourNewStageLogger;
          yourNewStageLogger.init();
       };
    ```

**7. Trigger the Event in the Code**

*   Find the appropriate location in your code where the event for your new stage occurs.
*   Construct the metrics using the builder.
*   Notify the `EventDispatcher` of the event, passing the metrics data.

    ```c++
    // Example: Inside the function where your new stage logic is executed
    YourNewStageMetricsBuilder builder;
    builder.with_generation(GetState("t_current"))
        .with_metric1(some_value)
        .with_metric2(another_value);

    YourNewStageMetrics metrics = builder.build();
    EventDispatcher<YourNewStageMetrics>::instance().notify(EventType::YOUR_NEW_STAGE, metrics);
    ```
