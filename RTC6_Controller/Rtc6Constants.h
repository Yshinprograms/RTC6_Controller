#pragma once

// Provides clear, readable constants for RTC6 API bitmasks, preventing "magic numbers".
namespace Rtc6Constants {

    // Symbolic constants for the accumulated error register (queried by get_error())
    // See SCANLAB RTC6 Manual, p. 439-441
    namespace Error {
        constexpr unsigned int None = 0;
        constexpr unsigned int Timeout = 1 << 3;              // Bit 3: No response from board
        constexpr unsigned int Parameter = 1 << 4;            // Bit 4: Invalid parameter in a command
        constexpr unsigned int Busy = 1 << 5;                 // Bit 5: List processing is active
        constexpr unsigned int Rejected = 1 << 6;             // Bit 6: List command rejected (e.g., invalid input pointer)
        constexpr unsigned int VersionMismatch = 1 << 8;      // Bit 8: Incompatible DLL, RBF, or OUT versions
        constexpr unsigned int TypeRejected = 1 << 10;          // Bit 10: Command not for this board type (e.g., ETH cmd on PCIe)
        constexpr unsigned int EthError = 1 << 13;              // Bit 13: General Ethernet error
    }

    // Symbolic constants for the real-time list status register (queried by read_status())
    // See SCANLAB RTC6 Manual, p. 648
    namespace Status {
        constexpr unsigned int Busy1 = 1 << 4;  // Bit 4: List 1 is currently executing
        constexpr unsigned int Busy2 = 1 << 5;  // Bit 5: List 2 is currently executing
    }
}