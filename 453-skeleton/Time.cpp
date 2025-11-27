//
// Created by mohammad on 27/01/25.
//

#include "Time.hpp"

#include <chrono>
#include <thread>

//======================================================================================================================

std::shared_ptr<Time> Time::Instance()
{
    std::shared_ptr<Time> shared_ptr = _instance.lock();
    if (shared_ptr == nullptr)
    {
        shared_ptr = std::make_shared<Time>();
        _instance = shared_ptr;
    }
    return shared_ptr;
}

//======================================================================================================================

Time::Time()
{
    _now = std::chrono::high_resolution_clock::now();
    _start = _now;
    _deltaTime = MaxDeltaTime;
}

//======================================================================================================================

Time::~Time() = default;

//======================================================================================================================

void Time::Update()
{
    auto timePoint = std::chrono::high_resolution_clock::now();
    {
        std::chrono::duration<float> const duration = timePoint - _now;
        _deltaTime = duration.count();
    }

    if (_deltaTime <= MinDeltaTime)
    {
        int const dtMs = static_cast<int>((MinDeltaTime - _deltaTime) * 1000.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(dtMs));
        timePoint = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> const duration = timePoint - _now;
        _deltaTime = duration.count();
    }

    {
        std::chrono::duration<float> const duration = timePoint - _start;
        _timeSec = duration.count();
    }

    _now = timePoint;
}

//======================================================================================================================

float Time::DeltaTimeSec() const
{
    return _deltaTime;
}

//======================================================================================================================

float Time::NowSec() const
{
    return _timeSec;
}

//======================================================================================================================
