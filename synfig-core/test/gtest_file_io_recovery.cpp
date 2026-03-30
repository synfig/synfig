/* === S Y N F I G ========================================================= */
/*!	\file gtest_file_io_recovery.cpp
**	\brief Tests for FileSystemTemporary (auto-recovery support)
**
**	\legal
**	Copyright (c) 2024 Synfig contributors
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

#include <gtest/gtest.h>

#include <synfig/general.h>
#include <synfig/main.h>
#include <synfig/filesystemtemporary.h>
#include <synfig/filesystemnative.h>

#include <string>

using namespace synfig;

class FileIORecoveryTest : public ::testing::Test {
protected:
	static std::unique_ptr<Main> main_instance;

	static void SetUpTestSuite() {
		if (!main_instance)
			main_instance = std::make_unique<Main>(".");
	}
};

std::unique_ptr<Main> FileIORecoveryTest::main_instance;

TEST_F(FileIORecoveryTest, CreateTemporaryFileSystem) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_recovery");
	ASSERT_TRUE(fs);
	EXPECT_EQ(fs->get_tag(), "test_recovery");
}

TEST_F(FileIORecoveryTest, TemporaryFileSystemStartsEmpty) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_empty");
	EXPECT_TRUE(fs->empty());
}

TEST_F(FileIORecoveryTest, WriteAndReadBackFile) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_rw");
	ASSERT_TRUE(fs);

	// Write content
	const std::string test_content = "Hello, recovery test!";
	{
		FileSystem::WriteStream::Handle ws = fs->get_write_stream("test_file.txt");
		ASSERT_TRUE(ws);
		*ws << test_content;
	}

	// Read it back
	{
		FileSystem::ReadStream::Handle rs = fs->get_read_stream("test_file.txt");
		ASSERT_TRUE(rs);
		std::string read_back;
		std::getline(*rs, read_back);
		EXPECT_EQ(read_back, test_content);
	}
}

TEST_F(FileIORecoveryTest, FileExistsAfterWrite) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_exists");

	{
		FileSystem::WriteStream::Handle ws = fs->get_write_stream("check_me.txt");
		ASSERT_TRUE(ws);
		*ws << "data";
	}

	EXPECT_TRUE(fs->is_file("check_me.txt"));
	EXPECT_FALSE(fs->empty());
}

TEST_F(FileIORecoveryTest, NonExistentFileIsNotFile) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_nofile");
	EXPECT_FALSE(fs->is_file("does_not_exist.txt"));
}

TEST_F(FileIORecoveryTest, MetaDataStorageAndRetrieval) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_meta");

	fs->set_meta("author", "test_suite");
	fs->set_meta("version", "1.0");

	EXPECT_EQ(fs->get_meta("author"), "test_suite");
	EXPECT_EQ(fs->get_meta("version"), "1.0");
	EXPECT_EQ(fs->get_meta("nonexistent"), "");
}

TEST_F(FileIORecoveryTest, ClearMetaData) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_clear_meta");

	fs->set_meta("key1", "value1");
	fs->set_meta("key2", "value2");
	EXPECT_EQ(fs->get_meta("key1"), "value1");

	fs->clear_meta();
	EXPECT_EQ(fs->get_meta("key1"), "");
	EXPECT_EQ(fs->get_meta("key2"), "");
}

TEST_F(FileIORecoveryTest, FileRemoval) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_remove");

	{
		FileSystem::WriteStream::Handle ws = fs->get_write_stream("removable.txt");
		ASSERT_TRUE(ws);
		*ws << "temporary data";
	}

	EXPECT_TRUE(fs->is_file("removable.txt"));
	EXPECT_TRUE(fs->file_remove("removable.txt"));
}

TEST_F(FileIORecoveryTest, DirectoryCreateAndScan) {
	FileSystemTemporary::Handle fs = new FileSystemTemporary("test_dir");

	EXPECT_TRUE(fs->directory_create("subdir"));

	FileSystem::FileList files;
	bool scanned = fs->directory_scan("subdir", files);
	EXPECT_TRUE(scanned);
}

TEST_F(FileIORecoveryTest, GetSystemTemporaryDirectory) {
	filesystem::Path tmp_dir = FileSystemTemporary::get_system_temporary_directory();
	// System temp directory should not be empty
	EXPECT_FALSE(tmp_dir.empty());
}

TEST_F(FileIORecoveryTest, GenerateTemporaryFilename) {
	filesystem::Path tmp_file = FileSystemTemporary::generate_system_temporary_filename("test_gen", ".tmp");
	EXPECT_FALSE(tmp_file.empty());
}
