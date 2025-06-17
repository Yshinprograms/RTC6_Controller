#include "pch.h"
#include "gtest/gtest.h"
#include "OvfParser.h"
#include "open_vector_format.pb.h"
#include <stdexcept>
#include <fstream> // Needed for creating a dummy malformed file

class OvfParserTest : public ::testing::Test {
protected:
    // This function is called once before all tests in this suite are run.
    static void SetUpTestSuite() {
        // We no longer generate complex files here. Instead, we just create
        // a simple malformed file for one specific test case.
        // The main test files (`valid_3_layers.ovf`, `empty_0_layers.ovf`)
        // are assumed to exist as assets.
        std::ofstream malformed(s_malformedFile, std::ios::binary);
        malformed << "THIS IS NOT AN OVF FILE";
        malformed.close();
    }

    // This function is called once after all tests are done.
    static void TearDownTestSuite() {
        // Clean up the one file we created.
        remove(s_malformedFile.c_str());
    }

    // This function is called before each individual test.
    void SetUp() override {
        // Create a fresh parser instance for each test to ensure they are isolated.
        parser = std::make_unique<OvfParser>();
    }

    std::unique_ptr<OvfParser> parser;

    // These filenames point to the asset files that should be in the output directory.
    static const std::string s_validFile;
    static const std::string s_emptyFile;
    static const std::string s_malformedFile;
    static const std::string s_nonExistentFile;
};

// Define the static filenames for our tests.
const std::string OvfParserTest::s_validFile = "valid_3_layers.ovf";
const std::string OvfParserTest::s_emptyFile = "empty_0_layers.ovf";
const std::string OvfParserTest::s_malformedFile = "malformed_test.ovf";
const std::string OvfParserTest::s_nonExistentFile = "no_such_file.ovf";


// =================================================================================
// ===                            THE TEST CASES                                 ===
// =================================================================================

TEST_F(OvfParserTest, OpenFileSucceedsForValidFile) {
    ASSERT_TRUE(parser->openFile(s_validFile));
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 3);
    auto jobShell = parser->getJobShell();
    ASSERT_TRUE(jobShell.has_job_meta_data());
    EXPECT_EQ(jobShell.job_meta_data().job_name(), "TestJob");
}

TEST_F(OvfParserTest, OpenFileSucceedsForEmptyLayerFile) {
    ASSERT_TRUE(parser->openFile(s_emptyFile));
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 0);
}

TEST_F(OvfParserTest, OpenFileFailsForMalformedFile) {
    ASSERT_FALSE(parser->openFile(s_malformedFile));
}

TEST_F(OvfParserTest, OpenFileFailsForNonExistentFile) {
    ASSERT_FALSE(parser->openFile(s_nonExistentFile));
}

TEST_F(OvfParserTest, GetWorkPlaneSucceedsAndReturnsCorrectData) {
    ASSERT_TRUE(parser->openFile(s_validFile));
    open_vector_format::WorkPlane wp;
    ASSERT_NO_THROW(wp = parser->getWorkPlane(1));
    EXPECT_EQ(wp.work_plane_number(), 1);
    EXPECT_FLOAT_EQ(wp.z_pos_in_mm(), 0.05f);
}

TEST_F(OvfParserTest, GetWorkPlaneThrowsExceptionIfFileNotOpen) {
    EXPECT_THROW(parser->getWorkPlane(0), std::runtime_error);
}

TEST_F(OvfParserTest, GetWorkPlaneThrowsExceptionForInvalidIndex) {
    parser->openFile(s_validFile);
    ASSERT_THROW(parser->getWorkPlane(3), std::out_of_range); // Boundary check
    ASSERT_THROW(parser->getWorkPlane(-1), std::out_of_range); // Negative check
}

TEST_F(OvfParserTest, RandomAccessOfWorkPlanesSucceeds) {
    ASSERT_TRUE(parser->openFile(s_validFile));
    // Access layers out of order to test the file seeking logic.
    ASSERT_NO_THROW(parser->getWorkPlane(2));
    ASSERT_NO_THROW(parser->getWorkPlane(0));
    ASSERT_NO_THROW(parser->getWorkPlane(1));
}