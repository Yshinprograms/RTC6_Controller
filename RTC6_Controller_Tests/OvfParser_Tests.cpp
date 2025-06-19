#include "pch.h"
#include "gtest/gtest.h"
#include "OvfParser.h"
#include "open_vector_format.pb.h"
#include <stdexcept>
#include <fstream> 

void CreateTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename, std::ios::binary);
    file << content;
    file.close();
}

class OvfParserTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        // Malformed file (fundamentally not an OVF file)
        CreateTestFile(s_malformedFile, "THIS IS NOT AN OVF FILE");

        // A file that is too short (correct magic, but no pointer)
        CreateTestFile(s_truncatedHeaderFile, "LVF!"); // Note: 4 bytes, same as "LVF!\x21" is 4 bytes. Let's make it more explicit.

        char magic_only[] = { 0x4c, 0x56, 0x46, 0x21 };
        std::ofstream truncated(s_truncatedHeaderFile, std::ios::binary);
        truncated.write(magic_only, sizeof(magic_only));
        truncated.close();
    }

    static void TearDownTestSuite() {
        remove(s_malformedFile.c_str());
        remove(s_truncatedHeaderFile.c_str());
    }

    void SetUp() override {
        parser = std::make_unique<OvfParser>();
    }

    std::unique_ptr<OvfParser> parser;

    // --- Asset Filenames ---
    static const std::string s_validFile;
    static const std::string s_emptyFile;
    static const std::string s_largeFile;

    static const std::string s_nonExistentFile;
    static const std::string s_malformedFile;
    static const std::string s_truncatedHeaderFile;
};

const std::string OvfParserTest::s_validFile = "valid_3_layers.ovf";
const std::string OvfParserTest::s_emptyFile = "empty_0_layers.ovf";
const std::string OvfParserTest::s_largeFile = "large_1000_layers.ovf";
const std::string OvfParserTest::s_nonExistentFile = "no_such_file.ovf";
const std::string OvfParserTest::s_malformedFile = "malformed_test.ovf";
const std::string OvfParserTest::s_truncatedHeaderFile = "truncated_header.ovf";

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
    ASSERT_TRUE(parser->openFile(s_validFile));
    ASSERT_EQ(parser->getNumberOfWorkPlanes(), 3);
    ASSERT_TRUE(parser->openFile(s_emptyFile));
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 0);
}

TEST_F(OvfParserTest, GetWorkPlane_WithValidIndex_ReturnsCorrectData) {
    parser->openFile(s_validFile);
    open_vector_format::WorkPlane wp;
    ASSERT_NO_THROW(wp = parser->getWorkPlane(1));
    EXPECT_EQ(wp.work_plane_number(), 1);
    EXPECT_FLOAT_EQ(wp.z_pos_in_mm(), 0.1f);
}

TEST_F(OvfParserTest, GetWorkPlane_WhenFileIsNotOpen_ThrowsRuntimeError) {
    EXPECT_THROW(parser->getWorkPlane(0), std::runtime_error);
}

TEST_F(OvfParserTest, GetWorkPlane_WithInvalidIndex_ThrowsOutOfRange) {
    parser->openFile(s_validFile);
    EXPECT_THROW(parser->getWorkPlane(3), std::out_of_range);
    EXPECT_THROW(parser->getWorkPlane(-1), std::out_of_range);
}

TEST_F(OvfParserTest, GetWorkPlane_WithRandomAccessOrder_ReturnsCorrectDataForEachLayer) {
    parser->openFile(s_validFile);
    open_vector_format::WorkPlane wp;
    ASSERT_NO_THROW(wp = parser->getWorkPlane(2));
    EXPECT_EQ(wp.work_plane_number(), 2);
    ASSERT_NO_THROW(wp = parser->getWorkPlane(0));
    EXPECT_EQ(wp.work_plane_number(), 0);
    ASSERT_NO_THROW(wp = parser->getWorkPlane(1));
    EXPECT_EQ(wp.work_plane_number(), 1);
}

TEST_F(OvfParserTest, GetNumberOfWorkPlanes_EmptyFile_ReturnsZero) {
    parser->openFile(s_emptyFile);
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 0);
}

TEST_F(OvfParserTest, GetNumberOfWorkPlanes_WithValidFile_ReturnsCorrectCount) {
    parser->openFile(s_validFile);
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 3);
}

TEST_F(OvfParserTest, GetNumberOfWorkPlanes_LargeFile_ReturnsCorrectCount) {
    parser->openFile(s_largeFile);
    EXPECT_EQ(parser->getNumberOfWorkPlanes(), 1000);
}

TEST_F(OvfParserTest, OpenFile_FileWithTruncatedHeader_ReturnsFalse) {
    EXPECT_FALSE(parser->openFile(s_truncatedHeaderFile));
}