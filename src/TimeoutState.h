#pragma once
#ifndef TIMEOUT_STATE_H
#define TIMEOUT_STATE_H

#include <map>
#include <chrono>
#include <stdint.h>


namespace coro
{
    class CTimeoutState
    {
    private:
        enum class State
        {
            WAIT,
            OFF,
            TRIGGERED
        };
        struct SData
        {
            SData() = default;
            SData(const uint32_t& p_scheduler_id, const std::chrono::milliseconds& p_duration)
                : scheduler_id(p_scheduler_id)
                , duration(p_duration)
            {}
            uint32_t scheduler_id;
            State state = State::WAIT;
            std::chrono::milliseconds duration;
        };
    public:
        CTimeoutState() = default;
        ~CTimeoutState() = default;
    private:
        void RecalcLock();
    public:
        uint32_t Add(const uint32_t& scheduler_id, const std::chrono::milliseconds& duration);
        bool Activate(const uint32_t& id, uint32_t& scheduler_id);
        void Cancel(const uint32_t& id);
        void CallThrow(const uint32_t& scheduler_id);
        bool IsLock() const;
        bool CheckScheduler(const uint32_t& scheduler_id, uint32_t& next_scheduler_id) const;
    private:
        bool m_lock = false;
        uint32_t m_last_id = 0;
        std::map<uint32_t, SData> m_storage;
    };
}
#endif
