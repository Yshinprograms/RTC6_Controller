## Hardware Configuration

Before compiling and running this application on a physical machine, all machine-specific calibration values **must** be set in the `RTC6_Main/config.h` file.

These values are critical for the geometric accuracy, safety, and correct operation of the hardware.

### Key Calibration Values

-   **`MachineConfig::MM_TO_BITS_CONVERSION_FACTOR`**: The primary scale factor that converts millimeters to the RTC6 board's internal integer units. This must be determined experimentally.
-   **`MachineConfig::RTC6_CORRECTION_FILE_PATH`**: The full path to the `.ct5` field correction file provided by SCANLAB for your specific lens and scanner setup.
-   **`MachineConfig::MAX_LASER_POWER_W`**: The maximum rated power of the connected laser in Watts. This is used to correctly scale power values from the OVF file.