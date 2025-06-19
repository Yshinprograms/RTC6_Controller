#include "pch.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

// The class we are testing
#include "PrintController.h"

// The interfaces for all its dependencies
#include "MockCommunicator.h"
#include "MockOvfParser.h"
#include "MockUI.h"
#include "MockListHandler.h"
#include "MockGeometryHandler.h"

// The data structures it uses
#include "PrintJobConfig.h"
#include "open_vector_format.pb.h"

using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;

// =================================================================================
// ===                            TEST FIXTURE                                   ===
// =================================================================================

class PrintControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // --- THIS IS THE FIX ---
        // Populate the dummy JobShell with the required parameter key.
        // The default VectorBlock has a marking_params_key of 0.
        // We must ensure that key exists in our dummy JobShell's map to prevent a crash.
        open_vector_format::MarkingParams defaultParams;
        dummyJobShell.mutable_marking_params_map()->insert({ 0, defaultParams });

        // Populate our dummy WorkPlanes with some data so the processing loop runs.
        dummyWorkPlane_0.set_work_plane_number(0);
        dummyWorkPlane_0.add_vector_blocks(); // This block will have marking_params_key = 0 by default.
        dummyWorkPlane_1.set_work_plane_number(1);
        dummyWorkPlane_1.add_vector_blocks(); // This block will also have marking_params_key = 0.

        // Create the controller instance, injecting all our mocks.
        controller = std::make_unique<PrintController>(
            mockCommunicator,
            mockParser,
            mockUI,
            mockListHandler,
            mockGeoHandler,
            config
        );
    }

    // Mocks for all dependencies
    MockCommunicator mockCommunicator;
    MockOvfParser mockParser;
    MockUI mockUI;
    MockListHandler mockListHandler;
    MockGeometryHandler mockGeoHandler;

    // Configuration and Data
    PrintJobConfig config{ "dummy_path.ovf", 100 };
    open_vector_format::Job dummyJobShell;
    open_vector_format::WorkPlane dummyWorkPlane_0;
    open_vector_format::WorkPlane dummyWorkPlane_1;

    // The System Under Test (SUT)
    std::unique_ptr<PrintController> controller;
};


TEST_F(PrintControllerTest, Run_HappyPathForTwoLayers_ExecutesFullProcessInOrder) {
    // --- Setup expectations for calls that can happen multiple times ---
    EXPECT_CALL(mockParser, getNumberOfWorkPlanes()).WillRepeatedly(Return(2));
    EXPECT_CALL(mockParser, getJobShell()).WillRepeatedly(Return(dummyJobShell));
    EXPECT_CALL(mockListHandler, getCurrentFillListId())
        .WillOnce(Return(1)).WillOnce(Return(1))
        .WillOnce(Return(2)).WillOnce(Return(2));

    // This call happens once per layer, for a total of two times.
    EXPECT_CALL(mockGeoHandler, processVectorBlock(_, _)).Times(2);

    // --- Enforce the main sequence of events ---
    InSequence s;

    // Init
    EXPECT_CALL(mockUI, displayMessage("--- Initializing Hardware ---"));
    EXPECT_CALL(mockCommunicator, connectAndSetupBoard()).WillOnce(Return(true));
    EXPECT_CALL(mockUI, displayMessage("Hardware successfully initialized."));

    // Parse
    EXPECT_CALL(mockUI, displayMessage("\n--- Parsing OVF File ---"));
    EXPECT_CALL(mockParser, openFile(_)).WillOnce(Return(true));
    EXPECT_CALL(mockUI, displayMessage("Successfully opened and parsed OVF file. Found 2 layer(s) to process."));

    // Process
    EXPECT_CALL(mockUI, displayMessage("\n--- Starting Layer Processing ---"));

    // Layer 0
    EXPECT_CALL(mockParser, getWorkPlane(0)).WillOnce(Return(dummyWorkPlane_0));
    EXPECT_CALL(mockUI, displayProgress("Preparing geometry on List 1", 0, 2));
    EXPECT_CALL(mockListHandler, beginListPreparation());
    EXPECT_CALL(mockListHandler, endListPreparation());
    EXPECT_CALL(mockUI, displayMessage("Executing Layer 0 on List 1."));
    EXPECT_CALL(mockListHandler, executeCurrentListAndCycle());
    EXPECT_CALL(mockListHandler, getLastExecutedListId()).WillOnce(Return(1));

    // Layer 1
    EXPECT_CALL(mockParser, getWorkPlane(1)).WillOnce(Return(dummyWorkPlane_1));
    EXPECT_CALL(mockUI, displayProgress("Preparing geometry on List 2", 1, 2));
    EXPECT_CALL(mockListHandler, beginListPreparation());
    EXPECT_CALL(mockListHandler, endListPreparation());
    EXPECT_CALL(mockUI, displayMessage("Waiting for previous layer on List 1 to finish..."));
    EXPECT_CALL(mockListHandler, isListBusy(1)).WillOnce(Return(false));
    EXPECT_CALL(mockUI, displayMessage("List 1 is now free."));
    EXPECT_CALL(mockUI, displayMessage("Simulating 100ms for powder bed recoating..."));
    EXPECT_CALL(mockUI, displayMessage("Executing Layer 1 on List 2."));
    EXPECT_CALL(mockListHandler, executeCurrentListAndCycle());
    EXPECT_CALL(mockListHandler, getLastExecutedListId()).WillOnce(Return(2));

    // Final wait
    EXPECT_CALL(mockUI, displayMessage("Waiting for previous layer on List 2 to finish..."));
    EXPECT_CALL(mockListHandler, isListBusy(2)).WillOnce(Return(false));
    EXPECT_CALL(mockUI, displayMessage("List 2 is now free."));
    EXPECT_CALL(mockUI, displayMessage("Simulating 100ms for powder bed recoating..."));

    // Final message
    EXPECT_CALL(mockUI, displayMessage("\n--- All 2 Layers Processed ---"));

    // ACT
    controller->run();
}

// =================================================================================
// ===                     NEW: EDGE CASE AND FAILURE TESTS                      ===
// =================================================================================

TEST_F(PrintControllerTest, Run_HardwareInitializationFails_StopsAndLogsError) {
    // ARRANGE: Script a scenario where the hardware fails to connect.
    InSequence s;

    EXPECT_CALL(mockUI, displayMessage("--- Initializing Hardware ---"));
    EXPECT_CALL(mockCommunicator, connectAndSetupBoard()).WillOnce(Return(false)); // The failure point
    EXPECT_CALL(mockUI, displayError("Hardware initialization failed. Aborting print job."));

    // ASSERT: The controller must not attempt to parse the file if hardware fails.
    EXPECT_CALL(mockParser, openFile(_)).Times(0);

    // ACT
    controller->run();
}

TEST_F(PrintControllerTest, Run_FileParsingFails_StopsAndLogsError) {
    // ARRANGE: Script a scenario where the file is invalid.
    InSequence s;

    // Hardware setup succeeds...
    EXPECT_CALL(mockUI, displayMessage("--- Initializing Hardware ---"));
    EXPECT_CALL(mockCommunicator, connectAndSetupBoard()).WillOnce(Return(true));
    EXPECT_CALL(mockUI, displayMessage("Hardware successfully initialized."));

    // ...but file parsing fails.
    EXPECT_CALL(mockUI, displayMessage("\n--- Parsing OVF File ---"));
    EXPECT_CALL(mockParser, openFile(config.ovfFilePath)).WillOnce(Return(false)); // The failure point
    EXPECT_CALL(mockUI, displayError("Could not parse OVF file: " + config.ovfFilePath));

    // ASSERT: The controller must not start the processing job.
    EXPECT_CALL(mockUI, displayMessage("\n--- Starting Layer Processing ---")).Times(0);
    EXPECT_CALL(mockParser, getWorkPlane(_)).Times(0);

    // ACT
    controller->run();
}

TEST_F(PrintControllerTest, Run_FileWithZeroLayers_ExitsGracefully) {
    // ARRANGE: Script a scenario for a valid but empty job file.
    EXPECT_CALL(mockParser, getNumberOfWorkPlanes()).WillRepeatedly(Return(0)); // The key condition

    InSequence s;

    // Hardware and parsing succeed...
    EXPECT_CALL(mockUI, displayMessage("--- Initializing Hardware ---"));
    EXPECT_CALL(mockCommunicator, connectAndSetupBoard()).WillOnce(Return(true));
    EXPECT_CALL(mockUI, displayMessage("Hardware successfully initialized."));
    EXPECT_CALL(mockUI, displayMessage("\n--- Parsing OVF File ---"));
    EXPECT_CALL(mockParser, openFile(_)).WillOnce(Return(true));
    EXPECT_CALL(mockUI, displayMessage("Successfully opened and parsed OVF file. Found 0 layer(s) to process."));

    // ...but the controller sees there are no layers and exits gracefully.
    EXPECT_CALL(mockUI, displayMessage("No layers found in the file. Nothing to process."));

    // ASSERT: The main processing loop is never entered.
    EXPECT_CALL(mockUI, displayMessage("\n--- Starting Layer Processing ---")).Times(0);

    // ACT
    controller->run();
}

TEST_F(PrintControllerTest, Run_VectorBlockWithMissingParamsKey_LogsWarningAndContinues) {
    // ARRANGE: Create a scenario where the data contract is broken.
    // The VectorBlock will request key 0, but the JobShell's map will be empty.

    // Create a new empty job shell for this specific test.
    open_vector_format::Job emptyJobShell;

    // Set up the mocks for a single-layer job.
    EXPECT_CALL(mockParser, getNumberOfWorkPlanes()).WillRepeatedly(Return(1));
    EXPECT_CALL(mockParser, getJobShell()).WillRepeatedly(Return(emptyJobShell)); // Return the empty shell
    EXPECT_CALL(mockListHandler, getCurrentFillListId()).WillRepeatedly(Return(1));

    InSequence s;

    // Standard successful startup
    EXPECT_CALL(mockUI, displayMessage("--- Initializing Hardware ---"));
    EXPECT_CALL(mockCommunicator, connectAndSetupBoard()).WillOnce(Return(true));
    EXPECT_CALL(mockUI, displayMessage("Hardware successfully initialized."));
    EXPECT_CALL(mockUI, displayMessage("\n--- Parsing OVF File ---"));
    EXPECT_CALL(mockParser, openFile(_)).WillOnce(Return(true));
    EXPECT_CALL(mockUI, displayMessage("Successfully opened and parsed OVF file. Found 1 layer(s) to process."));
    EXPECT_CALL(mockUI, displayMessage("\n--- Starting Layer Processing ---"));

    // The layer processing begins...
    EXPECT_CALL(mockParser, getWorkPlane(0)).WillOnce(Return(dummyWorkPlane_0));
    EXPECT_CALL(mockUI, displayProgress("Preparing geometry on List 1", 0, 1));
    EXPECT_CALL(mockListHandler, beginListPreparation());

    // ASSERT: The controller MUST catch the error and log it, NOT crash.
    EXPECT_CALL(mockUI, displayError("Marking params key 0 not found in JobShell map. Skipping vector block."));

    // ASSERT: The controller MUST continue gracefully after the error.
    EXPECT_CALL(mockListHandler, endListPreparation());
    EXPECT_CALL(mockUI, displayMessage("Executing Layer 0 on List 1."));
    EXPECT_CALL(mockListHandler, executeCurrentListAndCycle());
    EXPECT_CALL(mockListHandler, getLastExecutedListId()).WillOnce(Return(1));

    // Final wait and completion
    EXPECT_CALL(mockUI, displayMessage("Waiting for previous layer on List 1 to finish..."));
    EXPECT_CALL(mockListHandler, isListBusy(1)).WillOnce(Return(false));
    EXPECT_CALL(mockUI, displayMessage("List 1 is now free."));
    EXPECT_CALL(mockUI, displayMessage("Simulating 100ms for powder bed recoating..."));
    EXPECT_CALL(mockUI, displayMessage("\n--- All 1 Layers Processed ---"));

    // ACT
    controller->run();
}