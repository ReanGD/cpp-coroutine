#pragma once
#ifndef GOER_H
#define GOER_H

#include <memory>
#include <stdexcept>

namespace coro
{
	enum class EventStatus
	{
		ES_NORMAL,
		ES_CANCELLED,
		ES_TIMEDOUT
	};

	struct EventException
		: std::runtime_error
	{
		EventException(EventStatus s);
		EventStatus Status() { return st; }
	private:
		EventStatus st;
	};

	class CGoer
	{
		struct State
		{
			EventStatus status = EventStatus::ES_NORMAL;
		};
	public:
		CGoer();
	public:
		EventStatus Reset();
		bool Cancel();
		bool Timedout();
	private:
		bool SetStatus(EventStatus s);
		State& GetState();
	private:
		std::shared_ptr<State> m_state;
	};

}

#endif

