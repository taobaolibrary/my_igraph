/*
* @brief CPU timer for Unix
* @author Deyuan Qiu
* @date May 6, 2009
* @file timer.h
*/

#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <Mmsystem.h> 
#include <stdio.h>

#pragma comment(lib, "Winmm.lib")

class CTimer{
public:
	CTimer(void){ init(); };

	/*
	* Get elapsed time from last reset()
	* or class construction.
	* @return The elapsed time.
	*/
	DWORD getTime(void);

	/*
	* Reset the timer.
	*/
	void reset(void);

private:
	DWORD _time;
	DWORD _lStart;
	DWORD _lStop;
	void init(void);
};

#endif /* TIMER_H_ */
