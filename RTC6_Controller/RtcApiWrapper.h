#pragma once
#include "InterfaceRtcApi.h"

class RtcApiWrapper : public InterfaceRtcApi {
public:
    void api_auto_change() override;
    void api_set_start_list(UINT listNo) override;
    void api_set_end_of_list() override;
    void api_execute_list(UINT listNo) override;
    UINT api_read_status() override;
    void api_jump_abs(INT x, INT y) override;
    void api_mark_abs(INT x, INT y) override;
    void api_set_defocus_list(INT offset) override;
    void api_set_mark_speed(double speed) override;
    void api_set_laser_power(UINT port, UINT power) override;
};