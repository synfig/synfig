/* === S Y N F I G ========================================================= */
/*!	\file filecontainerzip.cpp
**	\brief FileContainerZip
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

#include <cstring>
#include <stdint.h>
#include <cstddef>

#include <libxml++/libxml++.h>
#ifdef _WIN32
#include <glibmm.h>
#endif

#include <ETL/stringf>

#include "zstreambuf.h"

#include "filecontainerzip.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

namespace synfig
{
	namespace FileContainerZip_InternalStructs
	{

		#pragma pack(push, 1)

		struct DOSTimestamp
		{
			uint16_t dos_time;
			uint16_t dos_date;

			inline DOSTimestamp():
				dos_time(0), dos_date(0) { }

			inline explicit DOSTimestamp(uint16_t dos_time, uint16_t dos_date):
				dos_time(dos_time), dos_date(dos_date) { }

			inline explicit DOSTimestamp(time_t time)
			{
				tm *t = localtime(&time);
				dos_time = ((t->tm_sec  & 0x3f) >>  1)
						 | ((t->tm_min  & 0x3f) <<  5)
						 | ((t->tm_hour & 0x1f) << 11);
				dos_date = ((t->tm_mday & 0x1f) <<  0)
						 | ((t->tm_mon  & 0x0f) <<  5)
						 | ((t->tm_year & 0x7f) <<  9);
			}

			inline time_t get_time()
			{
				tm t;
				memset(&t, 0, sizeof(t));
				t.tm_sec  = (dos_time <<  1) & 0x3f;
				t.tm_min  = (dos_time >>  5) & 0x3f;
				t.tm_hour = (dos_time >> 11) & 0x1f;
				t.tm_mday = (dos_time >>  0) & 0x1f;
				t.tm_mon  = (dos_time >>  5) & 0x0f;
				t.tm_year = (dos_time >>  9) & 0x7f;
				return mktime(&t);
			}
		};

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
				const static LocalFileHeader dummy;
				return (size_t)((const char *)&dummy.crc32 - (const char *)&dummy);
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

		#pragma pack(pop)
	}
}

using namespace synfig::FileContainerZip_InternalStructs;

void FileContainerZip::FileInfo::split_name()
{
	size_t pos = name.rfind('/');
	if (pos == String::npos || pos == 0)
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

FileContainerZip::FileContainerZip():
storage_file_(NULL),
prev_storage_size_(0),
file_reading_whole_container_(false),
file_reading_(false),
file_writing_(false),
file_processed_size_(0),
changed_(false)
{ }

FileContainerZip::~FileContainerZip() { close(); }

unsigned int FileContainerZip::crc32(unsigned int previous_crc, const void *buffer, size_t size)
{
	static const unsigned int table[] = {
	    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	};

	unsigned int crc = previous_crc ^ 0xFFFFFFFFUL;
	unsigned char *char_buf = (unsigned char *)buffer;
	size_t remain = size;
	while (remain--)
		crc = table[(crc ^ *char_buf++) & 0xFF] ^ (crc >> 8);
	return crc ^ 0xFFFFFFFFUL;
}

String FileContainerZip::encode_history(const FileContainerZip::HistoryRecord &history_record)
{
	xmlpp::Document document;
	document.
		create_root_node("history")->
		add_child("prev_storage_size")->
		set_child_text(strprintf("%lld", history_record.prev_storage_size));
	return document.write_to_string_formatted();
}

FileContainerZip::HistoryRecord FileContainerZip::decode_history(const String &comment)
{
	HistoryRecord history_record;
	xmlpp::DomParser parser;
	parser.parse_memory(comment);
	if(parser)
	{
		xmlpp::Element *root = parser.get_document()->get_root_node();
		if (root->get_name() == "history")
		{
			xmlpp::Element::NodeList list = root->get_children();
			for(xmlpp::Element::NodeList::iterator i = list.begin(); i != list.end(); i++)
			{
				if ((*i)->get_name() == "prev_storage_size")
				{
					String s;
					xmlpp::Element::NodeList list = (*i)->get_children();
					for(xmlpp::Element::NodeList::iterator j = list.begin(); j != list.end(); j++)
						if (dynamic_cast<xmlpp::TextNode*>(*j))
							s += dynamic_cast<xmlpp::TextNode*>(*j)->get_content();
					history_record.prev_storage_size = strtoll(s.c_str(), NULL, 10);
					if (history_record.prev_storage_size < 0)
						history_record.prev_storage_size = 0;
				}
			}
		}
	}
	return history_record;
}

void FileContainerZip::read_history(std::list<HistoryRecord> &list, FILE *f, file_size_t size)
{
	if (size < (long int)sizeof(EndOfCentralDirectory))
		return;

	// search "end of central directory" record

	char buffer[(1 << 16) + sizeof(EndOfCentralDirectory)];
	String comment;
	size_t read_size = size > (long int)sizeof(buffer)
					 ? sizeof(buffer) : (size_t)size;
	fseek(f, (long int)size - (long int)read_size, SEEK_SET);
	read_size = fread(&buffer, 1, read_size, f);
	bool found = false;
	HistoryRecord history_record;

	for(int i = read_size - sizeof(EndOfCentralDirectory); i >= 0; i--)
	{
		EndOfCentralDirectory *e = (EndOfCentralDirectory*)&buffer[i];
		if (e->signature == EndOfCentralDirectory::valid_signature__
		 && e->comment_length == (uint16_t)(read_size - sizeof(EndOfCentralDirectory) - i))
		{
			// found
			if (e->comment_length > 0)
			{
				comment = String(buffer + i + sizeof(EndOfCentralDirectory), e->comment_length);
				history_record = decode_history(comment);
				history_record.storage_size = size;
				found = true;
			}
			break;
		}
	}

	if (found)
	{
		list.front() = history_record;
		if (history_record.prev_storage_size > 0 && history_record.prev_storage_size < size) {
			list.push_front(HistoryRecord(0, history_record.prev_storage_size));
			read_history(list, f, history_record.prev_storage_size);
		}
	}
}

std::list<FileContainerZip::HistoryRecord> FileContainerZip::read_history(const String &container_filename)
{
	std::list<HistoryRecord> list;
	
#ifdef _WIN32
	FILE *f = fopen(Glib::locale_from_utf8(fix_slashes(container_filename)).c_str(), "rb");
#else
	FILE *f = fopen(container_filename.c_str(), "rb");
#endif
	if (f == NULL) return list;

	fseek(f, 0, SEEK_END);
	long int size = ftell(f);
	if (size >= (long int)sizeof(EndOfCentralDirectory))
	{
		list.push_front(HistoryRecord(0, size));
		read_history(list, f, size);
	}

	fclose(f);
	return list;
}

bool FileContainerZip::create(const String &container_filename)
{
	if (is_opened()) return false;
#ifdef _WIN32
	storage_file_ = fopen(Glib::locale_from_utf8(fix_slashes(container_filename)).c_str(), "w+b");
#else
	storage_file_ = fopen(fix_slashes(container_filename).c_str(), "w+b");
#endif	
	
	if (is_opened()) changed_ = true;
	return is_opened();
}

bool FileContainerZip::open_from_history(const String &container_filename, file_size_t truncate_storage_size) {
	if (is_opened()) return false;
#ifdef _WIN32
	FILE *f = fopen(Glib::locale_from_utf8(fix_slashes(container_filename)).c_str(), "r+b");
#else
	FILE *f = fopen(fix_slashes(container_filename).c_str(), "r+b");
#endif		
	if (f == NULL) return false;

	// check size of file
	fseek(f, 0, SEEK_END);
	long int filesize = ftell(f);
	long int actual_filesize = filesize;
	if (filesize < (long int)sizeof(EndOfCentralDirectory))
		{ fclose(f); return false; }

	if (truncate_storage_size > 0 && truncate_storage_size < filesize)
		filesize = (long int)truncate_storage_size;

	char buffer[(1 << 16) + sizeof(EndOfCentralDirectory)];

	// search "end of central directory" record
	EndOfCentralDirectory ecd;
	size_t read_size = filesize > (long int)sizeof(buffer)
					 ? sizeof(buffer) : (size_t)filesize;
	fseek(f, filesize - (long int)read_size, SEEK_SET);
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
		 && (cdfh.flags & 0x0071) == 0 )
		{
			FileInfo info;
			if (buffer[cdfh.filename_length - 1] == '/')
			{
				info.name = String(buffer, buffer + cdfh.filename_length - 1);
				info.is_directory = true;
			}
			else
			{
				info.name = String(buffer, buffer + cdfh.filename_length);
			}

			info.directory_saved = info.is_directory;
			info.size = cdfh.compressed_size;
			info.header_offset = cdfh.offset;
			info.compression = cdfh.compression;
			info.crc32 = cdfh.crc32;
			info.time = DOSTimestamp(cdfh.modification_time, cdfh.modification_date).get_time();
			info.split_name();
			files[info.name] = info;
		}
	}

	// create directories
	for(FileMap::iterator i = files.begin(); i != files.end();)
	{
		if (!i->second.name_part_directory.empty()
		 && files.find( i->second.name_part_directory ) == files.end())
		{
			FileInfo info;
			info.name = i->second.name_part_directory;
			info.is_directory = true;
			info.time = time(NULL);
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
	prev_storage_size_ = actual_filesize;
	file_reading_ = false;
	file_writing_ = false;
	changed_ = false;
	return true;
}

bool FileContainerZip::open(const String &container_filename)
{
	return open_from_history(container_filename);
}

bool FileContainerZip::save()
{
	if (file_is_opened()) return false;
	if (!changed_) return true;

	fseek(storage_file_, 0, SEEK_END);

	// write headers of new directories
	for(FileMap::iterator i = files_.begin(); i != files_.end(); i++)
	{
		FileInfo &info = i->second;
		if (info.is_directory && !info.directory_saved)
		{
			LocalFileHeader lfh;
			lfh.version = 20;
			lfh.filename_length = info.name.size() + 1;
			DOSTimestamp dos_timestamp(info.time);
			lfh.modification_time = dos_timestamp.dos_time;
			lfh.modification_date = dos_timestamp.dos_date;

			info.header_offset = ftell(storage_file_);
			if (sizeof(lfh) != fwrite(&lfh, 1, sizeof(lfh), storage_file_))
				return false;
			if (info.name.size() != fwrite(info.name.c_str(), 1, info.name.size(), storage_file_))
				return false;
			if ((int)'/' != fputc('/', storage_file_))
				return false;

			info.directory_saved = true;
		}
	}

	// write central directory
	uint32_t central_directory_offset = (uint32_t)ftell(storage_file_);
	for(FileMap::iterator i = files_.begin(); i != files_.end(); i++)
	{
		FileInfo &info = i->second;
		CentralDirectoryFileHeader cdfh;
		cdfh.min_version = 20;
		cdfh.offset = info.header_offset;
		cdfh.compressed_size = cdfh.uncompressed_size = info.size;
		cdfh.crc32 = info.crc32;
		cdfh.filename_length = (uint16_t)info.name.size();
		if (info.is_directory)
			cdfh.filename_length++;
		DOSTimestamp dos_timestamp(info.time);
		cdfh.modification_time = dos_timestamp.dos_time;
		cdfh.modification_date = dos_timestamp.dos_date;

		// write header
		if (sizeof(cdfh) != fwrite(&cdfh, 1, sizeof(cdfh), storage_file_))
			return false;

		// write name
		if (info.name.size() != fwrite(info.name.c_str(), 1, info.name.size(), storage_file_))
			return false;
		if (info.is_directory)
			if ((int)'/' != fputc('/', storage_file_))
				return false;
	}

	// end of central directory
	EndOfCentralDirectory ecd;
	ecd.offset = central_directory_offset;
	ecd.current_records = ecd.total_records = files_.size();
	ecd.size = ftell(storage_file_) - central_directory_offset;
	String comment = encode_history(HistoryRecord(prev_storage_size_));
	ecd.comment_length = comment.size();

	// write header
	if (sizeof(ecd) != fwrite(&ecd, 1, sizeof(ecd), storage_file_))
		return false;

	// write comment
	if (ecd.comment_length > 0
	 && ecd.comment_length != fwrite(comment.c_str(), 1, ecd.comment_length, storage_file_))
	{
		return false;
	}

	prev_storage_size_ = ftell(storage_file_);
	fflush(storage_file_);
	changed_ = false;
	return true;
}

void FileContainerZip::close()
{
	if (!is_opened()) return;

	// close opened file if need
	file_close();

	save();

	// close storage file and clead variables
	fclose(storage_file_);
	storage_file_ = NULL;
	files_.clear();
	prev_storage_size_ = 0;
	file_reading_ = false;
	file_writing_ = false;
	changed_ = false;
}

bool FileContainerZip::is_opened()
{
	return storage_file_ != NULL;
}

bool FileContainerZip::is_file(const String &filename)
{
	if (!is_opened()) return false;
	FileMap::const_iterator i = files_.find(fix_slashes(filename));
	return i != files_.end() && !i->second.is_directory;
}

bool FileContainerZip::is_directory(const String &filename)
{
	if (!is_opened()) return false;
	if (filename.empty()) return true;
	FileMap::const_iterator i = files_.find(fix_slashes(filename));
	return i != files_.end() && i->second.is_directory;
}

bool FileContainerZip::directory_check_name(const String &dirname)
{
	return dirname.size() <= (1 << 16) - 2 - sizeof(CentralDirectoryFileHeader);
}

bool FileContainerZip::directory_create(const String &dirname)
{
	if (!is_opened()) return false;
	if (is_file(dirname)) return false;
	if (is_directory(dirname)) return true;
	if (!directory_check_name(dirname)) return false;

	FileInfo info;
	info.name = fix_slashes(dirname);
	info.split_name();
	info.is_directory = true;
	info.time = time(NULL);
	if (info.name_part_localname.empty()
	 || !is_directory(info.name_part_directory)) return false;

	changed_ = true;
	files_[info.name] = info;
	return true;
}

bool FileContainerZip::directory_scan(const String &dirname, FileList &out_files)
{
	out_files.clear();
	if (!is_directory(dirname)) return false;
	for(FileMap::iterator i = files_.begin(); i != files_.end(); i++)
		if (i->second.name_part_directory == fix_slashes(dirname))
			out_files.push_back(i->second.name_part_localname);
	return true;
}

bool FileContainerZip::file_remove(const String &filename)
{
	if (is_directory(filename))
	{
		FileList files;
		directory_scan(filename, files);
		if (!files.empty()) return false;
		changed_ = true;
		files_.erase(fix_slashes(filename));
	}
	else
	if (is_file(filename))
	{
		if (file_is_opened() && file_->first == fix_slashes(filename))
			return false;
		changed_ = true;
		files_.erase(fix_slashes(filename));
	}
	return true;
}

bool FileContainerZip::file_check_name(const String &filename)
{
	return filename.size() <= (1 << 16) - 1 - sizeof(CentralDirectoryFileHeader);
}

bool FileContainerZip::file_open_read_whole_container()
{
	if (!is_opened() || file_is_opened()) return false;
	fseek(storage_file_, 0, SEEK_SET);
	file_reading_whole_container_ = true;
	file_processed_size_ = 0;
	return true;
}

bool FileContainerZip::file_open_read(const String &filename)
{
	if (!is_opened() || file_is_opened()) return false;
	file_ = files_.find(fix_slashes(filename));
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

bool FileContainerZip::file_open_write(const String &filename)
{
	if (!is_opened() || file_is_opened()) return false;
	if (!file_check_name(filename)) return false;

	file_ = files_.find(fix_slashes(filename));

	FileInfo new_info;
	if (file_ == files_.end())
	{
		// create new file
		new_info.name = fix_slashes(filename);
		new_info.split_name();
		if (new_info.name_part_localname.empty()
		 || !is_directory(new_info.name_part_directory)) return false;
	}
	else
	if (file_->second.is_directory)
		return false;

	FileInfo &info = file_ == files_.end() ? new_info : file_->second;

	// write header
	LocalFileHeader lfh;
	time_t t = time(NULL);
	lfh.version = 20;
	lfh.filename_length = info.name.size();
	DOSTimestamp dos_timestamp(t);
	lfh.modification_time = dos_timestamp.dos_time;
	lfh.modification_date = dos_timestamp.dos_date;

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
	info.compression = 0;
	info.crc32 = 0;
	info.time = t;
	if (file_ == files_.end())
	{
		files_[new_info.name] = new_info;
		file_ = files_.find(fix_slashes(filename));
	}
	file_writing_ = true;
	file_processed_size_ = 0;
	return true;
}

void FileContainerZip::file_close()
{
	if (file_is_opened_for_write())
	{
		LocalFileHeaderOverwrite lfho;
		lfho.crc32 = file_->second.crc32;
		lfho.compressed_size = lfho.uncompressed_size = file_->second.size;
		fseek(storage_file_, file_->second.header_offset + LocalFileHeaderOverwrite::offset_from_header(), SEEK_SET);
		fwrite(&lfho, 1, sizeof(lfho), storage_file_);
		file_writing_ = false;
		fflush(storage_file_);
	}
	file_reading_whole_container_ = false;
	file_reading_ = false;
	file_writing_ = false;
	file_processed_size_ = 0;

	// call base-class method to invalidate streams
	FileContainer::file_close();
}

bool FileContainerZip::file_is_opened_for_read()
{
	return is_opened() && (file_reading_ || file_reading_whole_container_);
}

bool FileContainerZip::file_is_opened_for_write()
{
	return is_opened() && file_writing_;
}

size_t FileContainerZip::file_read(void *buffer, size_t size)
{
	if (!file_is_opened_for_read()) return 0;
	file_size_t file_size = file_reading_whole_container_
	                      ? prev_storage_size_ : file_->second.size;
	file_size_t remain_size = file_size - file_processed_size_;
	size_t s = remain_size > (file_size_t)size ? size : (size_t)remain_size;
	s = fread(buffer, 1, s, storage_file_);
	file_processed_size_ += s;
	return s;
}

size_t FileContainerZip::file_write(const void *buffer, size_t size)
{
	if (!file_is_opened_for_write()) return 0;
	size_t s = fwrite(buffer, 1, size, storage_file_);
	file_processed_size_ += s;
	file_->second.size = file_processed_size_;
	file_->second.crc32 = crc32(file_->second.crc32, buffer, s);
	return s;
}

FileSystem::ReadStream::Handle FileContainerZip::get_read_stream(const String &filename)
{
	FileSystem::ReadStream::Handle stream = FileContainer::get_read_stream(filename);
	if (stream
	 && file_is_opened_for_read()
	 && !file_reading_whole_container_
	 && file_->second.compression > 0)
		return new ZReadStream(stream);
	return stream;
}

/* === E N T R Y P O I N T ================================================= */
