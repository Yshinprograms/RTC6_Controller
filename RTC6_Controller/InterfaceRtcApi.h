#pragma once
#include "RTC6impl.h"

class InterfaceRtcApi {
public:
    virtual ~InterfaceRtcApi() = default;

    virtual void api_auto_change() = 0;
    virtual void api_set_start_list(UINT listNo) = 0;
    virtual void api_set_end_of_list() = 0;
    virtual void api_execute_list(UINT listNo) = 0;
    virtual UINT api_read_status() = 0;
    virtual void api_jump_abs(INT x, INT y) = 0;
    virtual void api_mark_abs(INT x, INT y) = 0;
    virtual void api_set_defocus_list(INT offset) = 0;
    virtual void api_set_mark_speed(double speed) = 0;
    virtual void api_set_laser_power(UINT port, UINT power) = 0;
};