/* === S Y N F I G ========================================================= */
/*!	\file os.cpp
**	\brief OS specific methods
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2021 Rodolfo Ribeiro Gomes
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "os.h"

#include <cstdarg>
#include <cstdlib>

#include <synfig/filesystem.h>
#include <synfig/general.h>
#include <synfig/localization.h>

#if defined(HAVE_FORK) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)
# define UNIX_PIPE_TO_PROCESSES
# include <cstdlib> // for system()
# include <fcntl.h> // for O_ flags
# include <limits.h> // PATH_MAX
# include <sys/stat.h> // for S_ISDIR() and stat()
# include <sys/wait.h> // for waitpid()
# include <unistd.h> // for popen()
#elif defined(_WIN32)
# define WIN32_PIPE_TO_PROCESSES
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <fcntl.h> // for _O_ flags
# include <io.h> // for _open_osfhandle()
# include <shellapi.h> // for ShellExecuteW
#else
# error There are no known APIs for creating child processes
#endif

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */
/* === G L O B A L S ======================================================= */

synfig::filesystem::Path synfig::OS::fallback_binary_path;

/* === C L A S S E S ======================================================= */

#ifdef WIN32_PIPE_TO_PROCCESSES
class RunPipeWin32;
#else
class RunPipeUnix;
#endif

/* === P R O C E D U R E S ================================================= */

#ifdef _WIN32

std::string
utf16_to_utf8(const wchar_t* src, size_t src_length = 0)
{
	std::string utf8;

	if (!src)
		return utf8;

	if (src_length == 0)
		src_length = wcslen(src);

	const int length = WideCharToMultiByte(CP_UTF8, 0, src, src_length, 0, 0, nullptr, nullptr);
	utf8.resize(length + 1);

	WideCharToMultiByte(CP_UTF8, 0, src, src_length, &utf8[0], length, nullptr, nullptr);
	utf8[length] = 0;

	return utf8;
}

std::string
utf16_to_utf8(const std::wstring& src)
{
	return utf16_to_utf8(src.c_str(), src.size());
}

#endif

/* === M E T H O D S ======================================================= */

void
OS::RunArgs::push_back(const std::string& arg)
{
	std::vector<std::string>::push_back(arg);
}

void
OS::RunArgs::push_back(const char* arg)
{
	std::vector<std::string>::push_back(std::string(arg));
}

void
OS::RunArgs::push_back(const synfig::filesystem::Path& filename)
{
#ifdef WIN32_PIPE_TO_PROCESSES
	std::vector<std::string>::push_back('"' + filename.u8string() + '"');
#else
	std::vector<std::string>::push_back(filename.u8string());
#endif
}

void
OS::RunArgs::push_back(const std::pair<std::string, synfig::filesystem::Path>& arg_pair)
{
	std::vector<std::string>::push_back(arg_pair.first);
	push_back(arg_pair.second);
}

void
OS::RunArgs::push_back(const std::initializer_list<std::string>& args)
{
	insert(end(), args.begin(), args.end());
}

void
OS::RunArgs::push_back(const std::initializer_list<const char*>& args)
{
	insert(end(), args.begin(), args.end());
}

std::string
OS::RunArgs::get_string() const
{
	std::string ret;
	for (const auto& arg : *this) {
		ret.append(arg + " ");
	}
	if (!ret.empty())
		ret.pop_back();
	return ret;
}

void
OS::RunPipe::printf(const char* __format, ...)
{
	va_list args;
	va_start(args, __format);
	const std::string text = vstrprintf(__format, args);
	va_end(args);
	print(text);
}

const std::string&
OS::RunPipe::get_command() const
{
	return full_command_;
}

#ifdef WIN32_PIPE_TO_PROCESSES
class RunPipeWin32 : public OS::RunPipe
{
	bool initialized = false;
	bool is_reader = false;
	bool is_writer = false;

	HANDLE child_STDIN_Read = INVALID_HANDLE_VALUE;
	HANDLE child_STDIN_Write = INVALID_HANDLE_VALUE;
	FILE* read_file = nullptr;
	HANDLE child_STDOUT_Write = INVALID_HANDLE_VALUE;
	HANDLE child_STDERR_Read = INVALID_HANDLE_VALUE;
	HANDLE child_STDERR_Write = INVALID_HANDLE_VALUE;

	// RunPipe interface
public:
	RunPipeWin32() = default;
	~RunPipeWin32()
	{
		RunPipeWin32::close();
	}

	bool open(const filesystem::Path& binary_path, const OS::RunArgs& binary_args, OS::RunMode run_mode, const OS::RunRedirectionFiles& redir_files) override
	{
		if (initialized) {
			synfig::error(_("A pipe should not be initialized twice! Ignored..."));
			return false;
		}
		initialized = true;

		is_reader = run_mode == OS::RUN_MODE_READ || run_mode == OS::RUN_MODE_READWRITE;
		is_writer = run_mode == OS::RUN_MODE_WRITE || run_mode == OS::RUN_MODE_READWRITE;

		String command = '"' + binary_path.u8string() + '"';
		command += " " + binary_args.get_string();

		full_command_ = command;

		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		if (!redir_files.std_err.empty()) {
			child_STDERR_Write = CreateFileW(redir_files.std_err.c_str(),
											 GENERIC_WRITE,
											 FILE_SHARE_READ,
											 &saAttr,
											 CREATE_ALWAYS,
											 FILE_ATTRIBUTE_NORMAL,
											 NULL
								 );
			if (child_STDERR_Write == INVALID_HANDLE_VALUE) {
				synfig::error(_("Unable to open file to be stdout: %s [errno: %d]"), redir_files.std_err.u8_str(), errno);
				return false;
			}
		}

		if (!redir_files.std_out.empty()) {
			child_STDOUT_Write = CreateFileW(redir_files.std_out.c_str(),
											 GENERIC_WRITE,
											 FILE_SHARE_READ,
											 &saAttr,
											 CREATE_ALWAYS,
											 FILE_ATTRIBUTE_NORMAL,
											 NULL
								 );
			if (child_STDOUT_Write == INVALID_HANDLE_VALUE) {
				synfig::error(_("Unable to open file to be stdout: %s [errno: %d]"), redir_files.std_out.u8_str(), errno);
				return false;
			}
		} else if (is_reader) {
			HANDLE child_STDOUT_Read = INVALID_HANDLE_VALUE;
			if (!CreatePipe(&child_STDOUT_Read, &child_STDOUT_Write, &saAttr, 0))
				synfig::error("synfig::OS::pipe: child_STDOUT CreatePipe");

			// Ensure the read handle to the pipe for STDOUT is not inherited.

			if (!SetHandleInformation(child_STDOUT_Read, HANDLE_FLAG_INHERIT, 0))
				synfig::error("synfig::OS::pipe: child_STDOUT_Read SetHandleInformation");

			// Convert to C stdio FILE*
			int child_stdout_read_osfh = _open_osfhandle(intptr_t(child_STDOUT_Read), 0);
			if (child_stdout_read_osfh == -1) {
				synfig::error("synfig::OS::pipe: _open_osfhandle (stdout-read)");
				return false;
			}
			read_file = _fdopen(child_stdout_read_osfh, "rb");
			if (!read_file) {
				synfig::error("synfig::OS::pipe: _fdopen (stdout-read)");
				return false;
			}
			child_STDOUT_Read = INVALID_HANDLE_VALUE;
		}

		if (!redir_files.std_in.empty()) {
			child_STDIN_Read = CreateFileW(redir_files.std_in.c_str(),
											 GENERIC_READ,
											 0,
											 &saAttr,
											 OPEN_EXISTING,
											 FILE_ATTRIBUTE_NORMAL,
											 NULL
								 );
			if (child_STDIN_Read == INVALID_HANDLE_VALUE) {
				synfig::error(_("Unable to open file to be stdin: %s [errno: %d]"), redir_files.std_in.u8_str(), errno);
				return false;
			}
		} else if (is_writer) {
			if (!CreatePipe(&child_STDIN_Read, &child_STDIN_Write, &saAttr, 0))
				synfig::error("synfig::OS::pipe: child_STDIN CreatePipe");

			// Ensure the write handle to the pipe for STDIN is not inherited.

			if (!SetHandleInformation(child_STDIN_Write, HANDLE_FLAG_INHERIT, 0))
				synfig::error("synfig::OS::pipe: child_STDIN_Write SetHandleInformation");
		}

		return create_child_process(command);
	}
	bool is_readable() const override { return is_reader && read_file; }
	bool is_writable() const override { return is_writer && child_STDIN_Write != INVALID_HANDLE_VALUE; }
	bool is_open() const override { return read_file || child_STDIN_Write != INVALID_HANDLE_VALUE; };
	int close() override
	{
		int status = 0;
		close_and_invalidate_handle(child_STDIN_Read);
		close_and_invalidate_handle(child_STDIN_Write);
		if (read_file) {
			fclose(read_file);
			read_file = nullptr;
		}
		close_and_invalidate_handle(child_STDOUT_Write);
		close_and_invalidate_handle(child_STDERR_Read);
		close_and_invalidate_handle(child_STDERR_Write);

		//WaitForSingleObject(hThread, INFINITE);
		//CloseHandle(hThread);

		initialized = false;
		return status;
	}
	void print(const std::string& text) override
	{
		DWORD num_written = 0;
		BOOL ok = WriteFile(child_STDIN_Write, text.c_str(), text.size(), &num_written, NULL);
		if (!ok)
			synfig::error(_("synfig::OS::Pipe : fail to print"));
	}
	size_t write(const void* ptr, size_t size, size_t n) override
	{
		DWORD num_written = 0;
		BOOL ok = WriteFile(child_STDIN_Write, ptr, size * n, &num_written, NULL);
		if (!ok)
			synfig::error(_("synfig::OS::Pipe : fail to write"));
		return num_written;
	}

	void flush() override { }

	std::string read_contents() override
	{
		if (!read_file) {
			synfig::error(_("Should not try to readline() a non-readable pipe"));
			return "";
		}
		std::string result;
		char buf[128];
		while(!feof(read_file)) {
			if(fgets(buf, 128, read_file) != nullptr)
				result += buf;
		}
		return result;
	}
	std::string read_contents(size_t max_bytes) override
	{
		if (!read_file) {
			synfig::error(_("Should not try to readline(size_t max_bytes) a non-readable pipe"));
			return "";
		}
		std::string result;
		result.reserve(max_bytes);
		if (fgets(&result[0], max_bytes, read_file) != nullptr)
			return result;
		return "";
	}
	int getc() override
	{
		return fgetc(read_file);
	}
	int scanf(const char* __format, ...) override
	{
		va_list args;
		va_start(args, __format);
		int ret = vfscanf(read_file, __format, args);
		va_end(args);
		return ret;
	}
	bool eof() const override
	{
		return feof(read_file);
	}

private:
	bool create_child_process(const std::string& command)
	{
		PROCESS_INFORMATION piProcInfo;
		STARTUPINFOW siStartInfo;
		BOOL bSuccess = FALSE;

		ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

		ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.hStdError = child_STDERR_Write == INVALID_HANDLE_VALUE ? GetStdHandle(STD_ERROR_HANDLE) : child_STDERR_Write;
		siStartInfo.hStdOutput = child_STDOUT_Write == INVALID_HANDLE_VALUE ? GetStdHandle(STD_OUTPUT_HANDLE) : child_STDOUT_Write;
		siStartInfo.hStdInput = child_STDIN_Read == INVALID_HANDLE_VALUE ? GetStdHandle(STD_INPUT_HANDLE) : child_STDIN_Read;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

		auto native_command_line = synfig::filesystem::Path(command).native();
		native_command_line.reserve(MAX_PATH);

		bSuccess = CreateProcessW(NULL,
			&native_command_line[0],     // command line
			NULL,          // process security attributes
			NULL,          // primary thread security attributes
			TRUE,          // handles are inherited
			0,             // creation flags
			NULL,          // use parent's environment
			NULL,          // use parent's current directory
			&siStartInfo,  // STARTUPINFO pointer
			&piProcInfo);  // receives PROCESS_INFORMATION

		// If an error occurs, exit the application.
		if (!bSuccess) {
			synfig::error("synfig::OS::pipe: CreateProcess");

			close_and_invalidate_handle(child_STDERR_Read);
			close_and_invalidate_handle(child_STDERR_Write);
			close_and_invalidate_handle(child_STDOUT_Write);
			close_and_invalidate_handle(child_STDIN_Read);

			return false;
		}
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example.

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

		// Close handles to the stdin and stdout pipes no longer needed by the child process.
		// If they are not explicitly closed, there is no way to recognize that the child process has ended.

		close_and_invalidate_handle(child_STDERR_Write);
		close_and_invalidate_handle(child_STDOUT_Write);
		close_and_invalidate_handle(child_STDIN_Read);

		return true;
	}

	void close_and_invalidate_handle(HANDLE& handle) {
		if (handle != INVALID_HANDLE_VALUE) {
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
		}
	}
};
#else
class RunPipeUnix : public OS::RunPipe
{
	FILE* read_file = nullptr;
	FILE* write_file = nullptr;
	bool initialized = false;
	pid_t pid = -1;

	enum class OpenError {
		NO_ERROR = 0,
		ON_SET_CLOEXEC = -10000,
		ON_OPEN_STDERR_REDIR_FILE = -8000,
		ON_DUP2_STDERR_FILENO = -9000,
		ON_OPEN_STDOUT_REDIR_FILE = -1000,
		ON_DUP2_STDOUT_FILENO = -2000,
		ON_OPEN_STDIN_REDIR_FILE = -5000,
		ON_DUP2_STDIN_FILENO = -6000,
		ON_EXEC = EXIT_FAILURE,
	};

	static void print_error_and_exit(int error_fd, const std::string& msg, OpenError exit_number, bool print_perror)
	{
		synfig::error(msg);
		if (print_perror)
			perror(_("System error message:"));
		::write(error_fd, &exit_number, sizeof(exit_number));
		_exit(static_cast<int>(exit_number));
	}

	// RunPipe interface
public:
	RunPipeUnix() = default;
	~RunPipeUnix()
	{
		RunPipeUnix::close();
	}

	bool open(const filesystem::Path& binary_path, const OS::RunArgs& binary_args, OS::RunMode run_mode, const OS::RunRedirectionFiles& redir_files) override
	{
		if (initialized) {
			synfig::error(_("A pipe should not be initialized twice! Ignored..."));
			return false;
		}
		initialized = true;

		full_command_ = binary_path.u8string() + " " + binary_args.get_string();

		int p[2];

		if (pipe(p)) {
			synfig::error(_("Unable to open pipe to %s (no pipe)"), binary_path.u8_str());
			return false;
		}

		// pipe to comunicate error to parent process
		int error_message_pipe[2];
		if (pipe(error_message_pipe)) {
			synfig::error(_("Unable to open exec error pipes to %s (no pipe)"), binary_path.u8_str());
			return false;
		}

		pid = fork();

		if (pid == -1) {
			synfig::error(_("Unable to open pipe to %s (pid == -1)"), binary_path.u8_str());
			return false;
		}
		const bool is_reader = run_mode == OS::RUN_MODE_READ || run_mode == OS::RUN_MODE_READWRITE;
		const bool is_writer = run_mode == OS::RUN_MODE_WRITE || run_mode == OS::RUN_MODE_READWRITE;

		if (pid == 0) {
			// Child process

			// Child process does not read, only writes on error message pipe
			::close(error_message_pipe[0]);
			// close error message pipe on exec in order to parent process know exec was succeed
			if (-1 == fcntl(error_message_pipe[1], F_SETFD, FD_CLOEXEC))
				print_error_and_exit(error_message_pipe[1], _("Can't set FD_CLOEXEC on error message pipe"), OpenError::ON_SET_CLOEXEC, true);

			// If requested, redirect stderr to a file
			if (!redir_files.std_err.empty()) {
				mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
				int err_fd = ::open(redir_files.std_err.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
				if (err_fd == -1) {
					auto msg = strprintf(_("Unable to open file to be stderr: %s [errno: %d]"), redir_files.std_err.u8_str(), errno);
					print_error_and_exit(error_message_pipe[1], msg, OpenError::ON_OPEN_STDERR_REDIR_FILE, true);
				}
				// Map err_fd as stderr
				if (dup2(err_fd, STDERR_FILENO) == -1) {
					auto msg = strprintf(_("Unable to open pipe to %s (dup2( err_fd, STDERR_FILENO ) == -1)"), binary_path.u8_str());
					print_error_and_exit(error_message_pipe[1], msg, OpenError::ON_DUP2_STDERR_FILENO, true);
				}
			}

			// Redirect stdout to writer-pipe or to a file
			if (is_reader || !redir_files.std_out.empty()) {
				if (!redir_files.std_out.empty()) {
					::close(p[1]);
					mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
					p[1] = ::open(redir_files.std_out.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
					if (p[1] == -1) {
						auto msg = strprintf(_("Unable to open file to be stdout: %s [errno: %d]"), redir_files.std_out.u8_str(), errno);
						print_error_and_exit(error_message_pipe[1], msg, OpenError::ON_OPEN_STDOUT_REDIR_FILE, true);
					}
				}
				// Map pipeout as stdout
				if (dup2(p[1], STDOUT_FILENO) == -1) {
					auto msg = strprintf(_("Unable to open pipe to %s (dup2( p[1], STDOUT_FILENO ) == -1)"), binary_path.u8_str());
					print_error_and_exit(error_message_pipe[1], msg, OpenError::ON_DUP2_STDOUT_FILENO, true);
				}
			}
			// Close the unneeded pipeout/file descriptor
			::close(p[1]);
			// Redirect reader-pipe or file as source to stdin
			if (is_writer || !redir_files.std_in.empty()) {
				// Dup pipein to stdin
				if (!redir_files.std_in.empty()) {
					::close(p[0]);
					mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
					p[0] = ::open(redir_files.std_in.c_str(), O_RDONLY, mode);
					if (p[0] == -1) {
						auto msg = strprintf(_("Unable to open file to be stdin: %s [errno: %d]"), redir_files.std_in.u8_str(), errno);
						print_error_and_exit(error_message_pipe[1], msg, OpenError::ON_OPEN_STDIN_REDIR_FILE, true);
					}
				}
				if (dup2(p[0], STDIN_FILENO) == -1) {
					auto msg = strprintf(_("Unable to open pipe to %s (dup2( p[0], STDIN_FILENO ) == -1)"), binary_path.u8_str());
					print_error_and_exit(error_message_pipe[1], msg, OpenError::ON_DUP2_STDIN_FILENO, true);
				}
			}
			// Close the unneeded pipein
			::close(p[0]);

			char *args[binary_args.size()+2];
			std::string copy_binary_path = binary_path.u8string();
			size_t idx = 0;
			args[idx++] = &copy_binary_path[0];
			for (size_t i = 0; i < binary_args.size(); i++)
			{
				const auto& arg = binary_args[i];
				args[idx++] = const_cast<char*>(&arg[0]);
			}
			args[idx] = nullptr;

			execvp(binary_path.c_str(), args);

			// We should never reach here unless the exec failed
			auto msg = strprintf(_("Unable to open pipe to %s (exec failed)"), binary_path.u8_str());
			print_error_and_exit(error_message_pipe[1], msg, OpenError::ON_EXEC, true);
			return false;
		} else {
			// Parent process

			// Parent process does not write to error message pipe
			::close(error_message_pipe[1]);

			// Check for any error message from pipe
			OpenError error_code = OpenError::NO_ERROR;
			auto r = ::read(error_message_pipe[0], &error_code, sizeof(error_code));
			::close(error_message_pipe[0]);
			if (r != 0 || error_code != OpenError::NO_ERROR) {
				synfig::error(_("Error #%i on executing \"%s\""), error_code, full_command_.c_str());
				return false;
			}

			// Close pipein, not needed
			if (is_reader)
				read_file = fdopen(p[0], "r");
			else
				::close(p[0]);
			// Save pipeout to file handle, will write to it later
			if (is_writer)
				write_file = fdopen(p[1], "w");
			else
				::close(p[1]);
			return (!is_reader || read_file != nullptr) && (!is_writer || write_file != nullptr);
		}
	}

	bool is_readable() const override { return read_file != nullptr; }
	bool is_writable() const override { return write_file != nullptr; }
	bool is_open() const override { return read_file != nullptr || write_file != nullptr; };
	int close() override
	{
		if (write_file) {
			fclose(write_file);
			write_file = nullptr;
		}
		if (read_file) {
			fclose(read_file);
			read_file = nullptr;
		}
		int status;
		waitpid(pid, &status, 0);

		initialized = false;
		return status;
	}

	void print(const std::string &text) override { fputs(text.c_str(), write_file); }

	size_t write(const void* ptr, size_t size, size_t n) override { return fwrite(ptr, size, n, write_file); }

	void flush() override { fflush(write_file); }

	std::string read_contents() override
	{
		if (!read_file) {
			synfig::error(_("Should not try to readline() a non-readable pipe"));
			return "";
		}
		std::string result;
		char buf[128];
		while(!feof(read_file)) {
			if(fgets(buf, 128, read_file) != nullptr)
				result += buf;
		}
		return result;
	}
	std::string read_contents(size_t max_bytes) override
	{
		if (!read_file) {
			synfig::error(_("Should not try to readline(size_t max_bytes) a non-readable pipe"));
			return "";
		}
		std::string result;
		result.reserve(max_bytes);
		if(fgets(&result[0], max_bytes, read_file) != nullptr)
			return result;
		return "";
	}
	int getc() override { return fgetc(read_file); }
	int scanf(const char* __format, ...) override
	{
		va_list args;
		va_start(args, __format);
		int ret = vfscanf(read_file, __format, args);
		va_end(args);
		return ret;
	}
	bool eof() const override { return feof(read_file); };
};
#endif

OS::RunPipe::Handle
OS::RunPipe::create()
{
#ifdef WIN32_PIPE_TO_PROCESSES
	return std::unique_ptr<RunPipe>(new RunPipeWin32());
#else
	return std::unique_ptr<RunPipe>(new RunPipeUnix());
#endif
}

OS::RunPipe::Handle
OS::run_async(const filesystem::Path& binary_path, const RunArgs& binary_args, RunMode mode, const RunRedirectionFiles& redir_files)
{
	auto run_pipe = OS::RunPipe::create();
	if (!run_pipe) {
		synfig::warning(_("Couldn't create pipe for %s %s"), binary_path.u8_str(), binary_args.get_string().c_str());
		return nullptr;
	}
	bool ok = run_pipe->open(binary_path, binary_args, mode, redir_files);
	if (!ok) {
		synfig::warning(_("Couldn't open pipe for %s %s"), binary_path.u8_str(), binary_args.get_string().c_str());
		return nullptr;
	}
	if (!run_pipe->is_open()) {
		synfig::warning(_("Couldn't really open pipe for %s %s"), binary_path.c_str(), binary_args.get_string().c_str());
		return nullptr;
	}
	return run_pipe;
}

bool
OS::run_sync(const filesystem::Path& binary_path, const RunArgs& binary_args, const filesystem::Path& stdout_redir_file, const filesystem::Path& stderr_redir_file)
{
	auto run_pipe = OS::RunPipe::create();
	if (!run_pipe)
		return false;
	bool ok = run_pipe->open(binary_path, binary_args, OS::RunMode::RUN_MODE_READ, {stderr_redir_file, stdout_redir_file, synfig::filesystem::Path()});
	if (!ok)
		return false;
	if (!run_pipe->is_open())
		return false;

	int status = run_pipe->close();
#ifdef _WIN32
	return status == 0; // :(
#else
	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
#endif
}

bool
OS::launch_file_async(const filesystem::Path& file)
{
#ifdef _WIN32
	return 32<= INT_PTR( ShellExecuteW(NULL, NULL, file.c_str(), NULL, NULL, SW_SHOW) );
	// https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutew#return-value
#elif defined(__APPLE__)
	return 0 == system(strprintf("open \"%s\"", file.u8_str()).c_str());
#else
	return 0 == system(strprintf("xdg-open \"%s\"", file.u8_str()).c_str());
#endif
}

const std::vector<std::string>&
OS::get_user_lang()
{
	static std::vector<std::string> language_list;
	if (!language_list.empty())
		return language_list;

	{
		const char* lang_env = getenv("LANGUAGE");
		std::string language_list_str = lang_env ? trim(lang_env) : "";
		if (!language_list_str.empty()) {
			std::string::size_type pos = 0, prev_pos = 0;
			while ((pos = language_list_str.find(':', pos)) != std::string::npos) {
				auto lang = trim(language_list_str.substr(prev_pos, pos - prev_pos));
				if (!lang.empty())
					language_list.push_back(lang);
				++pos;
				prev_pos = pos;
			}
			if (prev_pos != std::string::npos) {
				auto lang = trim(language_list_str.substr(prev_pos));
				if (!lang.empty())
					language_list.push_back(lang);
			}
		}
	}

	{
		const char* lang_env = getenv("LANG");
		std::string lang = lang_env ? trim(lang_env) : "";
		if (!lang.empty()) {
			// remove encoding info
			auto dot_pos = lang.find('.');
			if (dot_pos != std::string::npos)
				lang = lang.substr(0, dot_pos);
			if (!lang.empty())
				language_list.push_back(lang);
		}
	}

#ifdef _WIN32
	// https://learn.microsoft.com/en-us/windows/win32/intl/locale-names
	WCHAR lpLocaleName[LOCALE_NAME_MAX_LENGTH];

	if (0 != GetUserDefaultLocaleName(lpLocaleName, LOCALE_NAME_MAX_LENGTH)) {
		std::string language_str = utf16_to_utf8(lpLocaleName);
		auto first_dash_pos = language_str.find('-');
		auto second_dash_pos = first_dash_pos == std::string::npos ? std::string::npos : language_str.find('-', first_dash_pos + 1);
		if (second_dash_pos != std::string::npos) { // Script code
			// lang-Script-REGION
			language_list.push_back(language_str.substr(0, first_dash_pos) + '_' + language_str.substr(second_dash_pos + 1));
		} else if (first_dash_pos == std::string::npos) {
			// lang
			language_list.push_back(language_str);
		} else {
			// lang-REGION or lang-Script
			// script code has 4 chars... https://www.unicode.org/iso15924/iso15924-codes.html
			if (second_dash_pos - first_dash_pos >= 5) {
				// discard script code
				language_list.push_back(language_str.substr(0, first_dash_pos));
			} else {
				// lang-REGION -> lang_REGION
				language_str[first_dash_pos] = '_';
				language_list.push_back(language_str);
			}
		}
	}
#endif
	return language_list;
}

filesystem::Path
OS::get_current_working_directory()
{
#ifdef _WIN32
	DWORD length = GetCurrentDirectoryW(0, nullptr);

	std::vector<filesystem::Path::value_type> current_dir_str(length);
	if (GetCurrentDirectoryW(length, current_dir_str.data()) != length - 1)
		return filesystem::Path("/");

	return filesystem::Path::from_native(current_dir_str.data()).lexically_normal();
#else

	struct stat st;
	if (char* pwd = getenv("PWD")) {
		if (stat(pwd, &st) == 0 && S_ISDIR(st.st_mode)) {
			return filesystem::Path::from_native(pwd).lexically_normal();
		}
	}

	std::vector<char> buffer(PATH_MAX);
	char* ptr = nullptr;

	while (!ptr) {
		ptr = getcwd(buffer.data(), buffer.size());
		if (!ptr) {
			if (errno != ERANGE)
				return filesystem::Path("/");
			buffer.resize(buffer.size() * 2);
		}
	}
	return filesystem::Path::from_native(buffer.data()).lexically_normal();
#endif
}
