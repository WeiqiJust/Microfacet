//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   01-16-2008
//  File:         codex_utils_profile.cpp
//  Content:      classes and functions for profiling
//
//////////////////////////////////////////////////////////////////////////////

#include "codex_utils_profile.h"


codex::utils::timer::timer()
	:ll_QPF_ticks_per_sec(0), ll_last_elapsed_time(0), ll_base_time(0),
	f_last_elapsed_time(0),
	f_base_time(0)
{
	LARGE_INTEGER qw_ticks_per_sec;
	
	b_QPF = QueryPerformanceFrequency(&qw_ticks_per_sec) == TRUE;

	if (b_QPF)
	{
		ll_QPF_ticks_per_sec = qw_ticks_per_sec.QuadPart;

		QueryPerformanceCounter(&qw_time);
		ll_base_time = qw_time.QuadPart;
	} else {
		f_time		= GetTickCount() * 0.001;
		f_base_time = f_time;
	}

	update();
}

void codex::utils::timer::update()
{
	if (b_QPF)
	{
		QueryPerformanceCounter( &qw_time );

		f_elapsed_time = (double) (qw_time.QuadPart - ll_last_elapsed_time) / 
			(double) ll_QPF_ticks_per_sec;
		ll_last_elapsed_time = qw_time.QuadPart;

		f_time = (double) (qw_time.QuadPart) / (double) ll_QPF_ticks_per_sec;
	} else {
		// Get either the current time or the stop time, depending
		// on whether we're stopped and what command was sent
		f_time = GetTickCount() * 0.001;

		f_elapsed_time		= (double) (f_time - f_last_elapsed_time);
		f_last_elapsed_time = f_time;
	}
}

double codex::utils::timer::elapsed_time()
{
	return f_elapsed_time;
}

double codex::utils::timer::abs_time()
{
	if (b_QPF)
	{
		QueryPerformanceCounter( &qw_time );
		return (double) (qw_time.QuadPart) / (double) ll_QPF_ticks_per_sec;
	} else {
		return GetTickCount() * 0.001;
	}
}

void codex::utils::decompose_time(DWORD &days, DWORD &hours, 
								  DWORD &minutes, DWORD &seconds, 
								  DWORD &milliseconds, double t)
{
	DWORD		time;

	time		= DWORD(t*1000);

	days		= time / (24*60*60*1000);
	hours		= (time / (60*60*1000)) % 24;
	minutes		= (time / (60*1000)) % 60;
	seconds		= (time / 1000) % 60;
	milliseconds= time % 1000;
}