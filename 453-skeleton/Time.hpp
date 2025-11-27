//
// Created by mohammad on 27/01/25.
//

#pragma once

#include <chrono>
#include <memory>

class Time
{
public:

    static constexpr int MaxFramerate = 120;
    static constexpr int MinFramerate = 30;
    static constexpr float MinDeltaTime = 1.0f / static_cast<float>(MaxFramerate);
    static constexpr float MaxDeltaTime = 1.0f / static_cast<float>(MinFramerate);

    static std::shared_ptr<Time> Instance();

    explicit Time();
    ~Time();

    // Delete copy constructor and copy assignment operator
    Time(const Time&) = delete;            // Copy constructor
    Time& operator=(const Time&) = delete; // Copy assignment

    // Delete move constructor and move assignment operator
    Time(Time&&) = delete;                 // Move constructor
    Time& operator=(Time&&) = delete;      // Move assignment

    void Update();

    [[nodiscard]]
    float DeltaTimeSec() const;

    [[nodiscard]]
    float NowSec() const;

private:

    inline static std::weak_ptr<Time> _instance {};

    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    TimePoint _start{};
    TimePoint _now {};
    float _deltaTime {};
    float _timeSec{};
};
