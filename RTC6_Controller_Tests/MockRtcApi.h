#pragma once
#include "gmock/gmock.h"
#include "InterfaceRtcApi.h"

class MockRtcApi : public InterfaceRtcApi {
public:
    MOCK_METHOD(void, api_auto_change, (), (override));
    MOCK_METHOD(void, api_set_start_list, (UINT listNo), (override));
    MOCK_METHOD(void, api_set_end_of_list, (), (override));
    MOCK_METHOD(void, api_execute_list, (UINT listNo), (override));
    MOCK_METHOD(UINT, api_read_status, (), (override));
    MOCK_METHOD(void, api_jump_abs, (INT x, INT y), (override));
    MOCK_METHOD(void, api_mark_abs, (INT x, INT y), (override));
    MOCK_METHOD(void, api_set_defocus_list, (INT offset), (override));
    MOCK_METHOD(void, api_set_mark_speed, (double speed), (override));
    MOCK_METHOD(void, api_set_laser_power, (UINT port, UINT power), (override));
};