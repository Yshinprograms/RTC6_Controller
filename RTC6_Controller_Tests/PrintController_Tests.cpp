// =========================================================================
// ===                  File: PrintController_Tests.cpp                  ===
// =========================================================================
#include "pch.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "PrintController.h"
#include "OvfParser.h"

// --- Mock Classes for Dependencies ---
#include "InterfaceListHandler.h"
#include "GeometryHandler.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

// A mock version of the ListHandler to observe interactions
class MockListHandler : public InterfaceListHandler {
public:
    MOCK_METHOD(bool, setupAutoChangeMode, (), (override));
    MOCK_METHOD(void, reArmAutoChange, (), (override));
    MOCK_METHOD(bool, beginListPreparation, (), (override));
    MOCK_METHOD(void, endListPreparation, (), (override));
    MOCK_METHOD(bool, executeCurrentListAndCycle, (), (override));
    MOCK_METHOD(bool, isListBusy, (UINT listIdToCheck), (const, override));
    MOCK_METHOD(UINT, getCurrentFillListId, (), (const, override));
    MOCK_METHOD(void, addJumpAbsolute, (INT x, INT y), (override));
    MOCK_METHOD(void, addMarkAbsolute, (INT x, INT y), (override));
    MOCK_METHOD(void, addSetFocusOffset, (INT offset_bits), (override));
    MOCK_METHOD(void, addSetMarkSpeed, (double speed_mm_s), (override));
    MOCK_METHOD(void, addSetLaserPower, (UINT port, UINT power), (override));
};

// A mock version of the GeometryHandler to observe interactions
class MockGeometryHandler : public GeometryHandler {
public:
    // We need to call the base constructor with a dummy mock, since it requires a reference.
    MockGeometryHandler(InterfaceListHandler& dummy) : GeometryHandler(dummy) {}

    // The method we want to spy on.
    MOCK_METHOD(void, processVectorBlock, (const open_vector_format::VectorBlock& block, const open_vector_format::MarkingParams& params), (override));
};

// --- Custom Matchers for Verifying VectorBlock Type ---
// This powerful technique lets us check the type of the 'oneof' field inside the VectorBlock.
MATCHER(IsLineSequence, "") {
    return arg.vector_data_case() == open_vector_format::VectorBlock::kLineSequence;
}
MATCHER(IsHatches, "") {
    return arg.vector_data_case() == open_vector_format::VectorBlock::kHatches;
}


// --- The Test Fixture ---
class PrintControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mocks. The MockGeometryHandler needs a dummy handler for its own constructor.
        mockListHandler = std::make_shared<MockListHandler>();
        mockGeometryHandler = std::make_shared<MockGeometryHandler>(*mockListHandler);

        // Use a REAL parser, as we're testing the integration from file to controller.
        parser = std::make_unique<OvfParser>();

        // Create the object under test, injecting the real parser and the mock handlers.
        controller = std::make_unique<PrintController>(
            *parser,
            *mockListHandler,
            *mockGeometryHandler
        );
    }

    std::unique_ptr<OvfParser> parser;
    std::shared_ptr<MockListHandler> mockListHandler;
    std::shared_ptr<MockGeometryHandler> mockGeometryHandler;
    std::unique_ptr<PrintController> controller;

    static const std::string s_hatchedSquareFile;
};

const std::string PrintControllerTest::s_hatchedSquareFile = "hatched_square_1_layer.ovf";

// --- The Integration Test Case ---
TEST_F(PrintControllerTest, PrintLayer_WithHatchedSquare_ProcessesContourThenHatchesInOrder) {
    // Arrange
    // Load the test file which contains a contour (LineSequence) and infill (Hatches).
    ASSERT_TRUE(parser->openFile(s_hatchedSquareFile)) << "Test file could not be opened. Did you update the Pre-Build Event?";

    // We expect the whole process to be wrapped in list preparation commands.
    // And the geometry processing must happen in the correct order.
    InSequence s;

    // 1. Expect the list preparation to begin.
    EXPECT_CALL(*mockListHandler, beginListPreparation()).WillOnce(Return(true));

    // 2. Expect processVectorBlock to be called for the CONTOUR first.
    //    We use our custom matcher to verify the type of the block.
    EXPECT_CALL(*mockGeometryHandler, processVectorBlock(IsLineSequence(), _));

    // 3. Expect processVectorBlock to be called for the HATCHES second.
    EXPECT_CALL(*mockGeometryHandler, processVectorBlock(IsHatches(), _));

    // 4. Expect the list preparation to end.
    EXPECT_CALL(*mockListHandler, endListPreparation());

    // Act
    // Call the method we want to test.
    ASSERT_NO_THROW(controller->printLayer(0));
}