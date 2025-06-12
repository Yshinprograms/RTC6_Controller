#include "RtcApiWrapper.h"
#include "RTC6impl.h" // The real DLL functions

void RtcApiWrapper::api_auto_change() { auto_change(); }
void RtcApiWrapper::api_set_start_list(UINT listNo) { set_start_list(listNo); }
void RtcApiWrapper::api_set_end_of_list() { set_end_of_list(); }
void RtcApiWrapper::api_execute_list(UINT listNo) { execute_list(listNo); }
UINT RtcApiWrapper::api_read_status() { return read_status(); }
void RtcApiWrapper::api_jump_abs(INT x, INT y) { jump_abs(x, y); }
void RtcApiWrapper::api_mark_abs(INT x, INT y) { mark_abs(x, y); }
void RtcApiWrapper::api_set_defocus_list(INT offset) { set_defocus_list(offset); }
void RtcApiWrapper::api_set_mark_speed(double speed) { set_mark_speed(speed); }
void RtcApiWrapper::api_set_laser_power(UINT port, UINT power) { set_laser_power(port, power); }