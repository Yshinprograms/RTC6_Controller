#pragma once

namespace Rtc6Constants {

    // Symbolic constants for the accumulated error register (queried by get_error())
    // See SCANLAB RTC6 Manual, p. 439-441
    namespace Error {
        constexpr unsigned int NONE = 0;
        constexpr unsigned int TIMEOUT = 1 << 3;
        constexpr unsigned int PARAMETER_ERROR = 1 << 4;
        constexpr unsigned int BUSY = 1 << 5;
        constexpr unsigned int REJECTED = 1 << 6;
        constexpr unsigned int VERSION_MISMATCH = 1 << 8;
        constexpr unsigned int TYPE_REJECTED = 1 << 10;
        constexpr unsigned int ETH_ERROR = 1 << 13;
    }

    // Symbolic constants for the real-time list status register (queried by read_status())
    // See SCANLAB RTC6 Manual, p. 648
    namespace Status {
        constexpr unsigned int BUSY1 = 1 << 4;  // List 1 is currently executing
        constexpr unsigned int BUSY2 = 1 << 5;  // List 2 is currently executing
    }
}