//////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Hongzhi Wu.  All Rights Reserved.
//
//  Created on:   01-15-2008
//  File:         codex_utils_profile.h
//  Content:      header file for profile related functionalities
//
//////////////////////////////////////////////////////////////////////////////

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
WARNING : EDIT ON THE ORIGINAL VERSION OF THIS FILE
OR EVERYTHING WILL BE LOST!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#pragma once

#include <Windows.h>

namespace codex
{
namespace utils
{
	class timer
	{
	private:
		double			f_time, f_elapsed_time;
		LARGE_INTEGER	qw_time;

		bool			b_QPF;
		LONGLONG		ll_QPF_ticks_per_sec, ll_last_elapsed_time,
						ll_base_time;
		double			f_last_elapsed_time, f_base_time;

	public:
		timer();

		void update();
		double elapsed_time();
		double abs_time();
	};

	void decompose_time(DWORD &days, DWORD &hours, 
		DWORD &minutes, DWORD &seconds, 
		DWORD &milliseconds, double t);
}
}