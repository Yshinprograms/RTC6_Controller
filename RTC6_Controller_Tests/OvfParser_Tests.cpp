#include "pch.h"
#include "gtest/gtest.h"
#include "OvfParser.h"
#include "open_vector_format.pb.h"
#include <stdexcept>
#include <fstream> 

class OvfParserTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // Create a simple malformed file for one specific test case.
        std::ofstream malformed(s_malformedFile, std::ios::binary);
        malformed << "THIS IS NOT AN OVF FILE";
        malformed.close();
    }

    static void TearDownTestSuite() {
        // Clean up the one file we created.
        remove(s_malformedFile.c_str());
    }

    void SetUp() override {
        parser = std::make_unique<OvfParser>();
    }

    std::unique_ptr<OvfParser> parser;

    // These filenames point to the asset files that should be in the output directory.
    static const std::string s_validFile;
    static const std::string s_emptyFile;
    static const std::string s_malformedFile;
    static const std::string s_nonExistentFile;
};
// Reminder to add the necessary test files via pre-build steps or manually.
// Define the static filenames for our tests.
const std::string OvfParserTest::s_validFile = "valid_3_layers.ovf";
const std::string OvfParserTest::s_emptyFile = "empty_0_layers.ovf";
const std::string OvfParserTest::s_malformedFile = "malformed_test.ovf";
const std::string OvfParserTest::s_nonExistentFile = "no_such_file.ovf";


// =================================================================================
// ===                            TEST CASES                                     ===
// =================================================================================

// ------ openFile Tests ------
TEST_F(OvfParserTest, OpenFile_WithValidFile_ReturnsTrueAndParsesMetadata) {
    ASSERT_TRUE(parser->openFile(s_validFile));
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 3);

    auto jobShell = parser->getJobShell();
    ASSERT_TRUE(jobShell.has_job_meta_data());
    EXPECT_EQ(jobShell.job_meta_data().job_name(), "Hatched Square Job");
}

TEST_F(OvfParserTest, OpenFile_WithEmptyLayerFile_ReturnsTrueAndSetsLayerCountToZero) {
    ASSERT_TRUE(parser->openFile(s_emptyFile));
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 0);
}

TEST_F(OvfParserTest, OpenFile_WithMalformedFile_ReturnsFalse) {
    EXPECT_FALSE(parser->openFile(s_malformedFile));
}

TEST_F(OvfParserTest, OpenFile_WithNonExistentFile_ReturnsFalse) {
    EXPECT_FALSE(parser->openFile(s_nonExistentFile));
}

TEST_F(OvfParserTest, OpenFile_WhenCalledTwice_ResetsStateAndParsesSecondFile) {
    // First call with the valid file
    ASSERT_TRUE(parser->openFile(s_validFile));
    ASSERT_EQ(parser->getNumberOfWorkPlanes(), 3);

    // Second call with the empty file
    ASSERT_TRUE(parser->openFile(s_emptyFile));
    // State should be completely reset.
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 0);
}

// ------ getWorkPlane Tests ------
TEST_F(OvfParserTest, GetWorkPlane_WithValidIndex_ReturnsCorrectData) {
    parser->openFile(s_validFile);

    open_vector_format::WorkPlane wp;
    ASSERT_NO_THROW(wp = parser->getWorkPlane(1));

    EXPECT_EQ(wp.work_plane_number(), 1);
    EXPECT_FLOAT_EQ(wp.z_pos_in_mm(), 0.1f);
}

TEST_F(OvfParserTest, GetWorkPlane_WhenFileIsNotOpen_ThrowsRuntimeError) {
    // Expect that calling getWorkPlane before openFile throws the correct exception.
    EXPECT_THROW(parser->getWorkPlane(0), std::runtime_error);
}

TEST_F(OvfParserTest, GetWorkPlane_WithInvalidIndex_ThrowsOutOfRange) {
    parser->openFile(s_validFile);

    // Check boundaries: one past the end, and a negative index.
    EXPECT_THROW(parser->getWorkPlane(3), std::out_of_range);
    EXPECT_THROW(parser->getWorkPlane(-1), std::out_of_range);
}

TEST_F(OvfParserTest, GetWorkPlane_WithRandomAccessOrder_ReturnsCorrectDataForEachLayer) {
    parser->openFile(s_validFile);

    open_vector_format::WorkPlane wp;

    // Access layers out of order to stress the file seeking logic.
    ASSERT_NO_THROW(wp = parser->getWorkPlane(2));
    EXPECT_EQ(wp.work_plane_number(), 2);

    ASSERT_NO_THROW(wp = parser->getWorkPlane(0));
    EXPECT_EQ(wp.work_plane_number(), 0);

    ASSERT_NO_THROW(wp = parser->getWorkPlane(1));
    EXPECT_EQ(wp.work_plane_number(), 1);
}