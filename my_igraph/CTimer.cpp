/*
* @brief CPU timer for Unix
* @author Deyuan Qiu
* @date May 6, 2009
* @file timer.cpp
*/

#include "CTimer.h"

void CTimer::init(void){
	_lStart = 0;
	_lStop = 0;
	_lStart = timeGetTime();
}

DWORD CTimer::getTime(void){
	_lStop = timeGetTime();
	return _lStop - _lStart;
}

void CTimer::reset(void){
	init();
}
