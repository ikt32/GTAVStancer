#include "Timer.hpp"
#include <chrono>

inline auto now() {
    using namespace std::chrono;
    auto tEpoch = steady_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(tEpoch).count();
}

CTimer::CTimer(int64_t timeout) :
    mPeriod(timeout),
    mPreviousTime(now()) {
}

void CTimer::Reset() {
    mPreviousTime = now();
}

void CTimer::Reset(int64_t newTimeout) {
    mPeriod = newTimeout;
    mPreviousTime = now();
}

bool CTimer::Expired() const {
    return now() > mPreviousTime + mPeriod;
}

int64_t CTimer::Elapsed() const {
    return now() - mPreviousTime;
}

int64_t CTimer::Period() const {
    return mPeriod;
}
