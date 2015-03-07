#include "stdafx.h"
#include "Goer.h"


namespace {
    std::string StatusToString(coro::EventStatus s)
    {
        switch (s)
        {
        case coro::EventStatus::ES_NORMAL:
            return "Normal";

        case coro::EventStatus::ES_CANCELLED:
            return "Cancelled";

        case coro::EventStatus::ES_TIMEDOUT:
            return "Timed out";
            
            /*default:
              return "<unknown>";*/
        }
    }
}

coro::EventException::EventException(EventStatus s)
    : std::runtime_error("Journey event received: " + StatusToString(s))
    , st(s)
{

}

coro::CGoer::CGoer()
    : m_state(std::make_shared<State>())
{

}

coro::EventStatus coro::CGoer::Reset()
{
    EventStatus s = GetState().status;
    GetState().status = EventStatus::ES_NORMAL;
    return s;
}

bool coro::CGoer::Cancel()
{
    return SetStatus(EventStatus::ES_CANCELLED);
}

bool coro::CGoer::Timedout()
{
    return SetStatus(EventStatus::ES_TIMEDOUT);
}

bool coro::CGoer::SetStatus(EventStatus s)
{
    auto& ss = GetState().status;
    if (ss != EventStatus::ES_NORMAL)
        return false;
    ss = s;
    return true;
}

coro::CGoer::State& coro::CGoer::GetState()
{
    State* s = m_state.get();
    return *s;
}
