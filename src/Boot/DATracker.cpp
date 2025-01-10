/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- DATracker.cpp -
	Digital audio tracker
*/

#include "Boot/DATracker.h"

namespace DATracker
{
	// DA tracker constants
	static constexpr Timer::FixedTime c_extrap_time = 1.5;
	static constexpr Timer::FixedTime c_latency = 0.025;

	// DA tracker state
	static uint8_t cur_track;
	static Timer::FixedTime raw_time, extrap_time, track_time;

	// DA report callback
	static void DAReport(CKSDK::CD::IRQStatus status, const CKSDK::CD::Result &result)
	{
		// Check status
		if (status != CKSDK::CD::IRQStatus::DataReady)
			return;

		// Get result as report
		CKSDK::CD::DAReport &report = *((CKSDK::CD::DAReport*)&result);

		// Check if this is absolute or local to the track
		if (report.result.second.b & 0x80)
		{
			// Calculate time in sectors
			uint32_t sectors = report.result.minute.Dec() * 60 * 75;
			sectors += CKSDK::CD::BCD::Dec(report.result.second.b & 0x7F) * 75;
			sectors += report.result.sector.Dec();

			// Calculate fixed time in seconds
			raw_time = Timer::FixedTime(sectors) / 75 + c_latency;
			extrap_time = raw_time;
		}
	}

	// DA tracker functions
	KEEP void Callback_Loop(CKSDK::CD::IRQStatus status, const CKSDK::CD::Result &result)
	{
		// Initialize state
		raw_time = extrap_time = track_time = 0;

		// Replay track
		CKSDK::CD::PlayTrack(cur_track, DAReport, Callback_Loop);
	}

	KEEP void PlayTrack(uint8_t track, CKSDK::CD::Callback end_cb)
	{
		// Initialize state
		raw_time = extrap_time = track_time = 0;

		// Play track
		cur_track = track;
		CKSDK::CD::PlayTrack(track, DAReport, end_cb);
	}

	KEEP Timer::FixedTime Tick(Timer::FixedTime dt)
	{
		CKSDK::OS::DisableIRQ();

		// Extrapolate time
		if (extrap_time < raw_time + c_extrap_time)
			extrap_time += dt;

		// Update track time
		Timer::FixedTime tt = track_time;
		if (tt < extrap_time)
			tt = extrap_time;
		track_time = tt;

		CKSDK::OS::EnableIRQ();
		return tt;
	}
}
