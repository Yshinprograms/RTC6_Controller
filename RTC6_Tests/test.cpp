// RTC6_Tests/test.cpp
// This file contains unit tests for the RTC6Controller and its dependencies.
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// --- Application Headers (Adjust paths based on your VS Project setup) ---
// Assuming RTC6_Controller project root is in Additional Include Directories for RTC6_Tests.
// e.g., "$(SolutionDir)RTC6_Controller\"
#include "RTC6Common.h"                // For Rtc6Exception, Point, and common types
#include "IRTCAPIDriver.h"  // Interface for the low-level RTC6 driver
#include "IListCommandBuilder.h" // Interface for list command building
#include "ILaserControl.h"  // Interface for laser control
#include "IMicroVectorProcessor.h" // Interface for micro-vector processing

#include "RTCAPIDriver.h"     // Concrete low-level driver (for instantiation in tests)
#include "ListCommandBuilder.h" // Concrete list command builder (for instantiation in tests)
#include "LaserControl.h"     // Concrete laser control (for instantiation in tests)
#include "MicroVectorProcessor.h"    // Concrete micro-vector processor (for instantiation in tests)
#include "RTC6Controller.h"   // The main high-level controller (Unit Under Test)


class Turtle {
public:
    virtual ~Turtle() {}
    virtual void PenUp() = 0;
    virtual void PenDown() = 0;
    virtual void Forward(int distance) = 0;
};

// --- Mock Implementations for Dependency Injection ---
// These classes simulate the behavior of the real interfaces for isolated unit testing.
// They record calls, allow setting expectations, and return predefined values.

// Mock for IRTCAPIDriver
class MockRTCAPIDriver : public IRTCAPIDriver {
public:
    // MOCK_METHOD(ReturnType, MethodName, (ParameterTypes...), (const_qualifier, override));
    MOCK_METHOD(UINT, init_rtc6_dll, (), (override));
    MOCK_METHOD(UINT, rtc6_count_cards, (), (override));
    MOCK_METHOD(UINT, select_rtc, (UINT cardNo), (override));
    MOCK_METHOD(UINT, load_program_file, (const char* path), (override));
    MOCK_METHOD(void, free_rtc6_dll, (), (override));
    MOCK_METHOD(UINT, get_rtc_version, (), (override));
    MOCK_METHOD(UINT, get_bios_version, (), (override));
    MOCK_METHOD(UINT, n_get_serial_number, (UINT cardNo), (override));
    MOCK_METHOD(UINT, n_get_last_error, (UINT cardNo), (override));
    MOCK_METHOD(UINT, get_error, (), (override));
    MOCK_METHOD(void, reset_error, (UINT cardNo, UINT errorMask), (override));
    MOCK_METHOD(void, set_laser_mode, (UINT mode), (override));
    MOCK_METHOD(void, set_laser_control, (UINT ctrl), (override));
    MOCK_METHOD(void, set_mark_speed, (double speed), (override));
    MOCK_METHOD(void, set_jump_speed, (double speed), (override));
    MOCK_METHOD(void, set_start_list, (UINT listNo), (override));
    MOCK_METHOD(void, set_end_of_list, (), (override));
    MOCK_METHOD(void, execute_list, (UINT listNo), (override));
    MOCK_METHOD(void, jump_abs, (LONG x, LONG y), (override));
    MOCK_METHOD(void, mark_abs, (LONG x, LONG y), (override));
    MOCK_METHOD(void, micro_vector_abs, (LONG x, LONG y, LONG laserOnBits, LONG laserOffBits), (override));
    MOCK_METHOD(void, set_laser_power_list, (UINT powerValue), (override));
    MOCK_METHOD(void, set_mark_speed_list, (double speedValue), (override));
    MOCK_METHOD(void, set_defocus_list, (LONG zAxisValue), (override));
    MOCK_METHOD(UINT, get_status, (), (override));
    MOCK_METHOD(UINT, get_list_pointer, (), (override));
};

// Mock for IListCommandBuilder
class MockListCommandBuilder : public IListCommandBuilder {
public:
    MOCK_METHOD(void, SetStartList, (UINT listNumber), (override));
    MOCK_METHOD(void, SetEndOfList, (), (override));
    MOCK_METHOD(void, ExecuteList, (UINT listNumber), (override));
    MOCK_METHOD(void, JumpAbsolute, (LONG x, LONG y), (override));
    MOCK_METHOD(void, MarkAbsolute, (LONG x, LONG y), (override));
    MOCK_METHOD(void, AddMicroVector, (LONG x, LONG y, LONG laserOnBits, LONG laserOffBits), (override));
    MOCK_METHOD(void, SetCurrentListLaserPower, (UINT powerValue), (override));
    MOCK_METHOD(void, SetCurrentListMarkSpeed, (double speedValue), (override));
    MOCK_METHOD(void, SetCurrentListZAxisHeight, (LONG zAxisValue), (override));
};

// Mock for ILaserControl
class MockLaserControl : public ILaserControl {
public:
    MOCK_METHOD(UINT, GetPowerForList, (), (const, override));
    MOCK_METHOD(double, GetSpeedForList, (), (const, override));
    MOCK_METHOD(LONG, GetZAxisHeightForList, (), (const, override));
    MOCK_METHOD(LONG, GetLaserOnBitValue, (), (const, override));
    MOCK_METHOD(LONG, GetLaserOffBitValue, (), (const, override));
    MOCK_METHOD(void, ConfigureGlobalLaser, (UINT mode, UINT controlSignals, double markSpeed, double jumpSpeed), (override));
};


// --- Test Fixture for RTC6Controller ---
// A test fixture sets up common objects needed by multiple tests in this suite.
class RTC6ControllerTest : public ::testing::Test {
protected:
    // Pointers to the Mock objects. These are *raw* pointers to the mocks
    // which are initially owned by unique_ptrs in SetUp.
    // We use raw pointers here because the ownership of the unique_ptrs is transferred
    // to the `controller` (Unit Under Test) in SetUp.
    MockRTCAPIDriver* driverMock;
    MockListCommandBuilder* listBuilderMock;
    MockLaserControl* laserControlMock;

    // The Unit Under Test (UUT) itself.
    std::unique_ptr<RTC6Controller> controller;

    // SetUp is called before each test in this fixture.
    void SetUp() override {
        // Create unique_ptrs for the mocks.
        // Google Mock can handle NiceMock/StrictMock wrappers for more specific behavior.
        // For simplicity, we use the base Mock class here.
        auto ownedDriverMock = std::make_unique<MockRTCAPIDriver>();
        auto ownedListBuilderMock = std::make_unique<MockListCommandBuilder>();
        auto ownedLaserControlMock = std::make_unique<MockLaserControl>();

        // Get raw pointers before transferring ownership. These are used to set expectations.
        driverMock = ownedDriverMock.get();
        listBuilderMock = ownedListBuilderMock.get();
        laserControlMock = ownedLaserControlMock.get();

        // Instantiate the RTC6Controller, passing ownership of the mocks.
        // This is dependency injection in action.
        controller = std::make_unique<RTC6Controller>(
            std::move(ownedDriverMock),
            std::move(ownedListBuilderMock),
            std::move(ownedLaserControlMock)
        );
    }
    // TearDown is automatically called after each test.
    // unique_ptrs handle the deletion of `controller`, and its destructor
    // will clean up the mocks it owns.
};

// --- Test Cases for RTC6Controller ---

TEST_F(RTC6ControllerTest, InitializationSucceeds) {
    // Set expectations on the mock driver for a successful initialization sequence.
    EXPECT_CALL(*driverMock, init_rtc6_dll()).WillOnce(Return(0)); // Simulate DLL init success
    EXPECT_CALL(*driverMock, rtc6_count_cards()).WillOnce(Return(1)); // Simulate one board detected
    EXPECT_CALL(*driverMock, select_rtc(1)).WillOnce(Return(1)); // Simulate selecting board 1
    EXPECT_CALL(*driverMock, load_program_file(_)).WillOnce(Return(0)); // Simulate firmware load success
    EXPECT_CALL(*driverMock, reset_error(1, _)).Times(1); // Expect reset_error to be called once for board 1

    // Act: Call the method of the Unit Under Test (RTC6Controller).
    ASSERT_NO_THROW(controller->InitializeAndLoadFirmware(1));

    // Assert: Verify the state of the controller.
    ASSERT_TRUE(controller->isInitialized_); // Access private member for testing purposes
}

TEST_F(RTC6ControllerTest, InitializationFailsIfDriverInitFails) {
    // Set expectation: init_rtc6_dll returns an error code.
    EXPECT_CALL(*driverMock, init_rtc6_dll()).WillOnce(Return(RTC6::ErrorCode::HARDWARE_ERROR));

    // Act & Assert: Expect Rtc6Exception to be thrown.
    ASSERT_THROW(controller->InitializeAndLoadFirmware(1), Rtc6Exception);
    ASSERT_FALSE(controller->isInitialized_); // Should not be initialized on failure
}

TEST_F(RTC6ControllerTest, InitializationFailsIfNoBoardsDetected) {
    EXPECT_CALL(*driverMock, init_rtc6_dll()).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, rtc6_count_cards()).WillOnce(Return(0)); // Simulate 0 boards detected

    ASSERT_THROW(controller->InitializeAndLoadFirmware(1), Rtc6Exception);
    ASSERT_FALSE(controller->isInitialized_);
}

TEST_F(RTC6ControllerTest, QueryBoardInfoCallsDriverMethods) {
    // First, simulate successful initialization of the controller.
    EXPECT_CALL(*driverMock, init_rtc6_dll()).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, rtc6_count_cards()).WillOnce(Return(1));
    EXPECT_CALL(*driverMock, select_rtc(1)).WillOnce(Return(1));
    EXPECT_CALL(*driverMock, load_program_file(_)).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, reset_error(1, _)).Times(1);
    controller->InitializeAndLoadFirmware(1);

    // Set expectations for query methods, providing dummy return values.
    UINT expectedRtcVer = 1000, expectedBiosVer = 200, expectedSerial = 12345;
    EXPECT_CALL(*driverMock, get_rtc_version()).WillOnce(Return(expectedRtcVer));
    EXPECT_CALL(*driverMock, get_bios_version()).WillOnce(Return(expectedBiosVer));
    EXPECT_CALL(*driverMock, n_get_serial_number(1)).WillOnce(Return(expectedSerial));
    EXPECT_CALL(*driverMock, get_error()).WillOnce(Return(0)); // Simulate no accumulated errors

    // Act: Call the method under test.
    UINT actualRtcVer, actualBiosVer, actualSerial;
    ASSERT_NO_THROW(controller->QueryBoardInformation(1, actualRtcVer, actualBiosVer, actualSerial));

    // Assert: Verify the returned values match expectations.
    ASSERT_EQ(actualRtcVer, expectedRtcVer);
    ASSERT_EQ(actualBiosVer, expectedBiosVer);
    ASSERT_EQ(actualSerial, expectedSerial);
}

TEST_F(RTC6ControllerTest, ConfigureGlobalLaserDelegatesToLaserControl) {
    // Simulate successful initialization first.
    EXPECT_CALL(*driverMock, init_rtc6_dll()).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, rtc6_count_cards()).WillOnce(Return(1));
    EXPECT_CALL(*driverMock, select_rtc(1)).WillOnce(Return(1));
    EXPECT_CALL(*driverMock, load_program_file(_)).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, reset_error(1, _)).Times(1);
    controller->InitializeAndLoadFirmware(1);

    // Set expectations on the MockLaserControl.
    UINT mode = 0; // CO2 mode
    UINT ctrlSignals = 0x18; // Example control signals
    double markSpeed = 100.0;
    double jumpSpeed = 500.0;
    EXPECT_CALL(*laserControlMock, ConfigureGlobalLaser(mode, ctrlSignals, markSpeed, jumpSpeed)).Times(1);

    // Act: Call the method under test.
    ASSERT_NO_THROW(controller->ConfigureGlobalLaser(mode, ctrlSignals, markSpeed, jumpSpeed));
}

TEST_F(RTC6ControllerTest, OpenListCallsCommandBuilder) {
    // Simulate successful initialization.
    EXPECT_CALL(*driverMock, init_rtc6_dll()).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, rtc6_count_cards()).WillOnce(Return(1));
    EXPECT_CALL(*driverMock, select_rtc(1)).WillOnce(Return(1));
    EXPECT_CALL(*driverMock, load_program_file(_)).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, reset_error(1, _)).Times(1);
    controller->InitializeAndLoadFirmware(1);

    // Set expectation on the MockListCommandBuilder.
    UINT listNum = 1;
    EXPECT_CALL(*listBuilderMock, SetStartList(listNum)).Times(1);

    // Act & Assert.
    ASSERT_NO_THROW(controller->OpenList(listNum));
}

TEST_F(RTC6ControllerTest, IsListBusyCallsDriverAndReturnsStatus) {
    // Simulate successful initialization.
    EXPECT_CALL(*driverMock, init_rtc6_dll()).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, rtc6_count_cards()).WillOnce(Return(1));
    EXPECT_CALL(*driverMock, select_rtc(1)).WillOnce(Return(1));
    EXPECT_CALL(*driverMock, load_program_file(_)).WillOnce(Return(0));
    EXPECT_CALL(*driverMock, reset_error(1, _)).Times(1);
    controller->InitializeAndLoadFirmware(1);

    // Simulate driver reporting busy status (bit 1 is 2)
    EXPECT_CALL(*driverMock, get_status()).WillOnce(Return(2)); // Busy status bit set

    // Act & Assert.
    ASSERT_TRUE(controller->IsListBusy());

    // Simulate driver reporting not busy status
    EXPECT_CALL(*driverMock, get_status()).WillOnce(Return(0)); // Not busy status bit (0)

    ASSERT_FALSE(controller->IsListBusy());
}

// --- Test Fixture for MicroVectorProcessor ---
// Testing MicroVectorProcessor requires a separate fixture as its dependencies are different.
class MicroVectorProcessorTest : public ::testing::Test {
protected:
    MockListCommandBuilder* listBuilderMock;
    MockLaserControl* laserControlMock;
    std::unique_ptr<MicroVectorProcessor> processor;

    void SetUp() override {
        auto ownedListBuilderMock = std::make_unique<MockListCommandBuilder>();
        auto ownedLaserControlMock = std::make_unique<MockLaserControl>();

        listBuilderMock = ownedListBuilderMock.get();
        laserControlMock = ownedLaserControlMock.get();

        processor = std::make_unique<MicroVectorProcessor>(
            ownedListBuilderMock.release(), // Release ownership as MicroVectorProcessor takes raw pointers in this specific case
            ownedLaserControlMock.release()
        );
        // Note: In this case, since MicroVectorProcessor takes raw pointers in its constructor,
        // it doesn't take ownership. So, ownedListBuilderMock and ownedLaserControlMock
        // should NOT be released. They should be passed as raw pointers directly.
        // Let's correct this common pattern: Mocks are owned by the test fixture, and
        // the UUT takes non-owning raw pointers to them.

        // Corrected SetUp: Mocks owned by the fixture, UUT takes raw pointers.
        listBuilderMock = new MockListCommandBuilder(); // Manually allocate for this scenario
        laserControlMock = new MockLaserControl();
        processor = std::make_unique<MicroVectorProcessor>(listBuilderMock, laserControlMock);
    }

    void TearDown() override {
        // Since we manually allocated in the corrected SetUp, we must manually delete.
        delete listBuilderMock;
        delete laserControlMock;
        // processor (unique_ptr) cleans itself up.
    }
};

TEST_F(MicroVectorProcessorTest, ProcessMicroVectorPathThrowsOnEmptyPath) {
    std::vector<Point> emptyPath;
    ASSERT_THROW(processor->ProcessMicroVectorPath(emptyPath), std::invalid_argument);
}

TEST_F(MicroVectorProcessorTest, ProcessMicroVectorPathGeneratesCorrectMicroVectors) {
    // Define a simple path: a single horizontal line
    std::vector<Point> path = {
        Point(0, 0, 0),
        Point(RTC6::MillimetersToBits(1.0), 0, 0) // 1mm long line
    };

    // Set processor configuration
    processor->SetMicroVectorTimeStep(RTC6::CLOCK_CYCLE_MICROSECONDS); // 10us
    processor->SetMinimumSegmentLength(0); // Process all segments
    processor->EnableDynamicParameterAdjustment(true);

    // Mock laser control to return fixed values for calculation
    double expectedSpeed = RTC6::DEFAULT_MARK_SPEED_BITS_MS; // 50 bits/ms
    UINT expectedPower = 10000;
    LONG expectedZ = 0;
    LONG laserOnBits = 0xFFFFFFFF; // All bits ON
    LONG laserOffBits = 0x00000000; // All bits OFF

    EXPECT_CALL(*laserControlMock, GetSpeedForList()).WillRepeatedly(Return(expectedSpeed));
    EXPECT_CALL(*laserControlMock, GetPowerForList()).WillRepeatedly(Return(expectedPower));
    EXPECT_CALL(*laserControlMock, GetZAxisHeightForList()).WillRepeatedly(Return(expectedZ));
    EXPECT_CALL(*laserControlMock, GetLaserOnBitValue()).WillRepeatedly(Return(laserOnBits));
    EXPECT_CALL(*laserControlMock, GetLaserOffBitValue()).WillRepeatedly(Return(laserOffBits));

    // Calculate expected number of micro-vectors for a 1mm line at 50 bits/ms with 10us steps
    // 1mm = 10000 bits (assuming default_bits_per_mm)
    // Time to mark 1mm = 10000 bits / (50 bits/ms) = 200 ms = 200,000 us
    // Num micro-vectors = 200,000 us / 10 us/vector = 20,000 vectors
    int expectedNumMicroVectors = 20000;

    // Expect calls on list builder
    EXPECT_CALL(*listBuilderMock, JumpAbsolute(path[0].X, path[0].Y)).Times(1);
    EXPECT_CALL(*listBuilderMock, SetCurrentListLaserPower(expectedPower)).Times(1);
    EXPECT_CALL(*listBuilderMock, SetCurrentListMarkSpeed(expectedSpeed)).Times(1);
    EXPECT_CALL(*listBuilderMock, SetCurrentListZAxisHeight(expectedZ)).Times(1);

    // Expect the correct number of micro-vector calls with correct laser states
    EXPECT_CALL(*listBuilderMock, AddMicroVector(_, _, laserOnBits, laserOffBits))
        .Times(expectedNumMicroVectors); // Check that the correct number of vectors are added

    // Act
    ASSERT_NO_THROW(processor->ProcessMicroVectorPath(path));
}

// Add more tests for:
// - ListCommandBuilder methods (delegation to driverMock)
// - LaserControl methods (configuration and getters)
// - RTCAPIDriver methods (raw DLL calls and error throwing - would require a mocking framework that can mock static/global functions or test internal state)
// - Edge cases for MicroVectorProcessor (e.g., very short segments, vertical/horizontal lines)
// - Error propagation through layers.