/* === S Y N F I G ========================================================= */
/*!	\file storagezip.cpp
**	\brief StorageZip
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "storagezip.h"
#include <cstring>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace StorageZIP_InternalStructs
{
	typedef unsigned int uint32_t;
	typedef unsigned short int uint16_t;

	struct LocalFileHeader
	{
		enum { valid_signature__ = 0x04034b50 };

		uint32_t signature;			//!< Local file header signature = 0x04034b50 (read as a little-endian number)
		uint16_t version;			//!< Version needed to extract (minimum)
		uint16_t flags;				//!< General purpose bit flag
		uint16_t compression;		//!< Compression method
		uint16_t modification_time;	//!< File last modification time
		uint16_t modification_date; //!< File last modification date
		uint32_t crc32;				//!< CRC-32
		uint32_t compressed_size;	//!< Compressed size
		uint32_t uncompressed_size;	//!< Uncompressed size
		uint16_t filename_length;	//!< File name length (n)
		uint16_t extrafield_length; //!< Extra field length (m)
		// next:
		//   filename - n bytes
		//   extrafield - m bytes
		//   file data
		//   optional LocalFileDataDescriptor (if bit 3 (0x08) set in flags)

		inline LocalFileHeader()
		{
			memset(this, 0, sizeof(*this));
			signature = valid_signature__;
		}
	};

	struct LocalFileHeaderOverwrite
	{
		uint32_t crc32;				//!< CRC-32
		uint32_t compressed_size;	//!< Compressed size
		uint32_t uncompressed_size;	//!< Uncompressed size

		inline LocalFileHeaderOverwrite()
		{
			memset(this, 0, sizeof(*this));
		}

		inline static size_t offset_from_header()
		{
			return (size_t)(&((LocalFileHeader*)0)->crc32);
		}
	};

	struct CentralDirectoryFileHeader
	{
		enum { valid_signature__ = 0x02014b50 };

		uint32_t signature;			//!< Central directory file header signature = 0x02014b50
		uint16_t version;			//!< Version made by
		uint16_t min_version;		//!< Version needed to extract (minimum)
		uint16_t flags;				//!< General purpose bit flag
		uint16_t compression;		//!< Compression method
		uint16_t modification_time;	//!< File last modification time
		uint16_t modification_date; //!< File last modification date
		uint32_t crc32;				//!< CRC-32
		uint32_t compressed_size;	//!< Compressed size
		uint32_t uncompressed_size;	//!< Uncompressed size
		uint16_t filename_length;	//!< File name length (n)
		uint16_t extrafield_length;	//!< Extra field length (m)
		uint16_t filecomment_length;//!< File comment length (k)
		uint16_t disk_number;		//!< Disk number where file starts
		uint16_t internal_attrs;	//!< Internal file attributes
		uint32_t external_attrs;	//!< External file attributes
		uint32_t offset;			//!< Relative offset of local file header.
									//!< This is the number of bytes between the
									//!< start of the first disk on which the
									//!< file occurs, and the start of the local
									//!< file header. This allows software reading
									//!< the central directory to locate the
									//!< position of the file inside the .ZIP file.
		// next:
		//   filename - n bytes
		//   extrafield - m bytes
		//   filecomment - k bytes
		//   next CentralDirectorySignature

		inline CentralDirectoryFileHeader()
		{
			memset(this, 0, sizeof(*this));
			signature = valid_signature__;
		}
	};

	struct EndOfCentralDirectory
	{
		enum { valid_signature__ = 0x06054b50 };

		uint32_t signature;			//!< End of central directory signature = 0x06054b50
		uint16_t current_disk;		//!< Number of this disk
		uint16_t first_disk;		//!< Disk where central directory starts
		uint16_t current_records;	//!< Number of central directory records on this disk
		uint16_t total_records;		//!< Total number of central directory records
		uint32_t size;				//!< Size of central directory (bytes)
		uint32_t offset;			//!< Offset of start of central directory, relative to start of archive
		uint16_t comment_length;	//!< Comment length (n)
		// next:
		//   comment - n bytes
		//   end of file

		inline EndOfCentralDirectory()
		{
			memset(this, 0, sizeof(*this));
			signature = valid_signature__;
		}
	};
}

using namespace StorageZIP_InternalStructs;

void StorageZip::FileInfo::split_name()
{
	size_t pos = name.rfind('/');
	if (pos == std::string::npos || pos == 0)
	{
		name_part_directory.clear();
		name_part_localname = name;
	}
	else
	{
		name_part_directory = name.substr(0, pos);
		name_part_localname = name.substr(pos + 1);
	}
}

StorageZip::StorageZip():
storage_file_(NULL),
prev_storage_size_(0),
file_reading_(false),
file_writing_(false),
file_processed_size_(0),
changed_(false)
{ }

StorageZip::~StorageZip() { close(); }

bool StorageZip::create(const std::string &storage_filename)
{
	if (is_opened()) return false;
	storage_file_ = fopen(storage_filename.c_str(), "wb");
	return is_opened();
}

bool StorageZip::open(const std::string &storage_filename)
{
	if (is_opened()) return false;
	FILE *f = fopen(storage_filename.c_str(), "r+b");
	if (f == NULL) return false;

	// check size of file
	fseek(f, 0, SEEK_END);
	long int filesize = ftell(f);
	if (filesize <= (long int)sizeof(EndOfCentralDirectory))
		{ fclose(f); return false; }

	char buffer[(1 << 16) + sizeof(EndOfCentralDirectory)];

	// search "end of central directory" record
	EndOfCentralDirectory ecd;
	size_t read_size = filesize > (long int)sizeof(buffer)
					 ? sizeof(buffer) : (size_t)filesize;
	fseek(f, -(long int)read_size, SEEK_END);
	read_size = fread(&buffer, 1, read_size, f);
	bool found = false;
	for(int i = read_size - sizeof(EndOfCentralDirectory); i >= 0; i--)
	{
		EndOfCentralDirectory *e = (EndOfCentralDirectory*)&buffer[i];
		if (e->signature == EndOfCentralDirectory::valid_signature__
		 && e->comment_length == (uint16_t)(read_size - sizeof(EndOfCentralDirectory) - i))
		{
			ecd = *e;
			found = true;
			break;
		}
	}
	if (!found)
		{ fclose(f); return false; }

	// read "central directory"
	FileMap files;
	fseek(f, ecd.offset, SEEK_SET);
	for(int i = 0; i < ecd.current_records; i++)
	{
		CentralDirectoryFileHeader cdfh;
		if (sizeof(cdfh) != fread(&cdfh, 1, sizeof(cdfh), f))
			{ fclose(f); return false; }

		// read name, comment and extrafield
		size_t extra_size = cdfh.filename_length
		                  + cdfh.filecomment_length
		                  + cdfh.extrafield_length;
		if (extra_size != fread(buffer, 1, extra_size, f))
			{ fclose(f); return false; }

		if (cdfh.filename_length > 0
		 && (cdfh.flags & 0x0071) == 0
		 && cdfh.compression == 0)
		{
			FileInfo info;
			if (buffer[cdfh.filename_length - 1] == '/')
			{
				info.name = std::string(buffer, buffer + cdfh.filename_length - 1);
				info.is_directory = true;
			}
			else
			{
				info.name = std::string(buffer, buffer + cdfh.filename_length);
			}

			info.size = cdfh.compressed_size;
			info.header_offset = cdfh.offset;
			info.crc32 = cdfh.crc32;
			info.split_name();
			files[info.name] = info;
		}
	}

	// create directories
	for(FileMap::iterator i = files.begin(); i != files.end();)
	{
		if (files.find( i->second.name_part_directory ) == files.end())
		{
			FileInfo info;
			info.name = i->second.name_part_directory;
			info.is_directory = true;
			info.split_name();
			files[ info.name ] = info;
			i = files.begin();
		}
		else i++;
	}

	// loaded
	fseek(f, 0, SEEK_END);
	storage_file_ = f;
	files_.swap( files );
	prev_storage_size_ = filesize;
	file_reading_ = false;
	file_writing_ = false;
	changed_ = false;
	return true;
}

void StorageZip::close()
{
	if (!is_opened()) return;

	// close opened file if need
	file_close();

	if (changed_)
	{
		// write central directory
		fseek(storage_file_, 0, SEEK_END);
		uint32_t central_directory_offset = ftell(storage_file_);
		for(FileMap::iterator i = files_.begin(); i != files_.end(); i++)
		{
			FileInfo &info = i->second;
			CentralDirectoryFileHeader cdfh;
			cdfh.min_version = 20;
			cdfh.compressed_size = cdfh.uncompressed_size = info.size;
			cdfh.crc32 = info.crc32;
			cdfh.filename_length = (uint16_t)info.name.size();
			if (info.is_directory)
				cdfh.filename_length++;
			// TODO: modification date/time

			// write header
			fwrite(&cdfh, 1, sizeof(cdfh), storage_file_);

			// write name
			fwrite(info.name.c_str(), 1, info.name.size(), storage_file_);
			if (info.is_directory)
				fputc('/', storage_file_);
		}

		// end of central directory
		EndOfCentralDirectory ecd;
		ecd.offset = central_directory_offset;
		ecd.total_records = files_.size();
		ecd.size = ftell(storage_file_) - central_directory_offset;
		if (prev_storage_size_ > 0)
			ecd.comment_length = sprintf(NULL, "%llx", prev_storage_size_);

		// write header
		fwrite(&ecd, 1, sizeof(ecd), storage_file_);

		// write comment if need
		if (prev_storage_size_)
			fprintf(storage_file_, "%llx", prev_storage_size_);
	}

	// close storage file and clead variables
	fclose(storage_file_);
	files_.clear();
	prev_storage_size_ = 0;
	file_reading_ = false;
	file_writing_ = false;
	changed_ = false;
}


bool StorageZip::is_opened()
{
	return storage_file_ != NULL;
}

bool StorageZip::is_file(const std::string &filename)
{
	if (!is_opened()) return false;
	FileMap::const_iterator i = files_.find(filename);
	return i != files_.end() && !i->second.is_directory;
}

bool StorageZip::is_directory(const std::string &filename)
{
	if (!is_opened()) return false;
	FileMap::const_iterator i = files_.find(filename);
	return i != files_.end() && i->second.is_directory;
}

bool StorageZip::directory_create(const std::string &dirname)
{
	if (!is_opened()) return false;
	if (is_file(dirname)) return false;
	if (is_directory(dirname)) return true;
	if (dirname.size() > (1 << 16) - 2 - sizeof(CentralDirectoryFileHeader))
		return false;

	FileInfo info;
	info.name = dirname;
	info.split_name();
	info.is_directory = true;
	if (info.name_part_localname.empty()
	 || !is_directory(info.name_part_directory)) return false;

	// write header
	LocalFileHeader lfh;
	lfh.version = 20;
	lfh.filename_length = info.name.size() + 1;
	// TODO: modification date/time
	fseek(storage_file_, 0, SEEK_END);
	info.header_offset = ftell(storage_file_);
	changed_ = true;
	fwrite(&lfh, 1, sizeof(lfh), storage_file_);
	fwrite(info.name.c_str(), 1, info.name.size(), storage_file_);
	fputc('/', storage_file_);

	files_[info.name] = info;
	return true;
}

bool StorageZip::directory_scan(const std::string &dirname, std::list< std::string > &out_files)
{
	out_files.clear();
	if (!is_directory(dirname)) return false;
	for(FileMap::iterator i = files_.begin(); i != files_.end(); i++)
		if (i->second.name_part_directory == dirname)
			out_files.push_back(i->second.name_part_localname);
	return true;
}

bool StorageZip::file_remove(const std::string &filename)
{
	if (is_directory(filename))
	{
		std::list< std::string > files;
		directory_scan(filename, files);
		if (!files.empty()) return false;
		changed_ = true;
		files_.erase(filename);
	}
	else
	if (is_file(filename))
	{
		if (file_is_opened() && file_->first == filename)
			return false;
		changed_ = true;
		files_.erase(filename);
	}
	return true;
}

bool StorageZip::file_open_read(const std::string &filename)
{
	if (!is_opened() || file_is_opened()) return false;
	file_ = files_.find(filename);
	if (file_ == files_.end() || file_->second.is_directory)
		return false;

	// read header
	LocalFileHeader lfh;
	fseek(storage_file_, file_->second.header_offset, SEEK_SET);
	if (sizeof(lfh) != fread(&lfh, 1, sizeof(lfh), storage_file_))
		return false;
	if (lfh.signature != LocalFileHeader::valid_signature__)
		return false;

	// seek to file begin
	fseek(storage_file_, lfh.filename_length + lfh.extrafield_length, SEEK_CUR);
	file_reading_ = true;
	file_processed_size_ = 0;
	return true;
}

bool StorageZip::file_open_write(const std::string &filename)
{
	if (!is_opened() || file_is_opened()) return false;
	if (filename.size() > (1 << 16) - 1 - sizeof(CentralDirectoryFileHeader))
		return false;
	file_ = files_.find(filename);

	FileInfo new_info;
	if (file_ == files_.end())
	{
		// create new file
		new_info.name = filename;
		new_info.split_name();
		if (new_info.name_part_localname.empty()
		 || !is_directory(new_info.name_part_directory)) return false;
	}
	else
	if (file_->second.is_directory)
		return false;

	FileInfo &info = file_ == files_.end() ? new_info :file_->second;

	// write header
	LocalFileHeader lfh;
	lfh.version = 20;
	lfh.filename_length = info.name.size();
	// TODO: modification date/time
	fseek(storage_file_, 0, SEEK_END);
	long int offset = ftell(storage_file_);
	changed_ = true;
	if (sizeof(lfh) != fwrite(&lfh, 1, sizeof(lfh), storage_file_))
		return false;
	if (info.name.size() != fwrite(info.name.c_str(), 1, info.name.size(), storage_file_))
		return false;

	// update file info
	info.header_offset = offset;
	info.size = 0;
	info.crc32 = 0;
	if (file_ == files_.end())
	{
		files_[new_info.name] = new_info;
		file_ = files_.find(filename);
	}
	file_writing_ = true;
	file_processed_size_ = 0;
	return true;
}

void StorageZip::file_close()
{
	if (file_is_opened_for_write())
	{
		LocalFileHeaderOverwrite lfho;
		lfho.crc32 = file_->second.crc32;
		lfho.compressed_size = lfho.uncompressed_size = file_->second.size;
		fseek(storage_file_, file_->second.header_offset + LocalFileHeaderOverwrite::offset_from_header(), SEEK_SET);
		fwrite(&lfho, 1, sizeof(lfho), storage_file_);
		file_writing_ = false;
	}
	file_reading_ = false;
	file_writing_ = false;
	file_processed_size_ = 0;
}

bool StorageZip::file_is_opened_for_read()
{
	return is_opened() && file_reading_;
}

bool StorageZip::file_is_opened_for_write()
{
	return is_opened() && file_writing_;
}

size_t StorageZip::file_read(void *buffer, size_t size)
{
	if (!file_is_opened_for_read()) return 0;
	file_size_t remain_size = file_->second.size - file_processed_size_;
	size_t s = remain_size > (file_size_t)size ? size : (size_t)remain_size;
	s = fread(buffer, 1, s, storage_file_);
	file_processed_size_ += s;
	return s;
}

size_t StorageZip::file_write(const void *buffer, size_t size)
{
	if (!file_is_opened_for_write()) return 0;
	size_t s = fwrite(buffer, 1, size, storage_file_);
	file_processed_size_ += s;
	file_->second.size = file_processed_size_;
	// TODO: crc32
	return s;
}

/* === E N T R Y P O I N T ================================================= */


