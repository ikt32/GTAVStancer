#pragma once
#include <cstdint>

class CTimer {
public:
    explicit CTimer(int64_t timeout);
    ~CTimer() = default;
    void Reset();
    void Reset(int64_t newTimeout);
    bool Expired() const;
    int64_t Elapsed() const;
    int64_t Period() const;
private:
    int64_t mPeriod;
    int64_t mPreviousTime;
};
