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

#include <synfig/filesystem.h>
#include <synfig/general.h>
#include <synfig/localization.h>

#if defined(HAVE_FORK) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)
# define UNIX_PIPE_TO_PROCESSES
# include <cstdlib> // for system()
# include <fcntl.h> // for O_ flags
# include <sys/wait.h> // for waitpid()
# include <unistd.h> // for popen()
#elif defined(_WIN32)
# define WIN32_PIPE_TO_PROCESSES
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <shellapi.h> // for ShellExecuteW
#else
# error There are no known APIs for creating child processes
#endif

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */
/* === G L O B A L S ======================================================= */

/* === C L A S S E S ======================================================= */

#ifdef WIN32_PIPE_TO_PROCCESSES
class RunPipeWin32;
#else
class RunPipeUnix;
#endif

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
OS::RunArgs::push(const std::vector<std::string>& args)
{
	run_args_.insert(run_args_.end(), args.begin(), args.end());
}

void
OS::RunArgs::push(const std::vector<const char *>& args)
{
	run_args_.insert(run_args_.end(), args.begin(), args.end());
}

void
OS::RunArgs::push(const std::string& item)
{
	run_args_.push_back(item);
}

void
OS::RunArgs::push(std::string&& item)
{
	run_args_.emplace_back(std::move(item));
}

void
OS::RunArgs::push(const char* __format, ...)
{
	va_list args;
	va_start(args, __format);
	run_args_.emplace_back(vstrprintf(__format, args));
	va_end(args);
}

void
OS::RunArgs::push_pair(const std::string& key, const std::string& value)
{
	run_args_.push_back(key);
	run_args_.push_back(value);
}

void
OS::RunArgs::push_pair_filename(const std::string& key, const std::string& filename)
{
	run_args_.push_back(key);
	push_filename(filename);
}

void
OS::RunArgs::push_filename(const std::string& filename)
{
#ifdef WIN32_PIPE_TO_PROCESSES
	run_args_.push_back('"' + filename + '"');
#else
	run_args_.push_back(filename);
#endif
}

void
OS::RunArgs::push_filename(std::string&& filename)
{
#ifdef WIN32_PIPE_TO_PROCESSES
	run_args_.emplace_back(std::move('"' + filename + '"'));
#else
	run_args_.emplace_back(std::move(filename));
#endif
}

void
OS::RunArgs::push_filename(const char* filename)
{
#ifdef WIN32_PIPE_TO_PROCESSES
	run_args_.emplace_back(strprintf("\"%s\"", filename));
#else
	run_args_.emplace_back(filename);
#endif
}

void
OS::RunArgs::clear()
{
	run_args_.clear();
}

std::string
OS::RunArgs::get_string() const
{
	std::string ret;
	for (const auto& arg : run_args_) {
		ret.append(arg + " ");
	}
	if (!ret.empty())
		ret.pop_back();
	return ret;
}

const std::string&
OS::RunArgs::operator[](RunArgs::size_type i) const
{
	return run_args_[i];
}

bool
OS::RunArgs::empty() const
{
	return run_args_.empty();
}

OS::RunArgs::size_type
OS::RunArgs::size() const
{
	return run_args_.size();
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
	FILE* file = nullptr;
	bool initialized = false;
	bool is_reader = false;
	bool is_writer = false;

	// RunPipe interface
public:
	RunPipeWin32() = default;
	~RunPipeWin32()
	{
		RunPipeWin32::close();
	}

	bool open(std::string binary_path, const OS::RunArgs& binary_args, OS::RunMode run_mode, const OS::RunRedirectionFiles& redir_files) override
	{
		if (initialized) {
			synfig::error("A pipe should not be initialized twice! Ignored...");
			return false;
		}
		initialized = true;

		String command = '"' + binary_path + '"';
		command += " " + binary_args.get_string();

		// This covers the dumb cmd.exe behavior.
		// See: http://eli.thegreenplace.net/2011/01/28/on-spaces-in-the-paths-of-programs-and-files-on-windows/
		command = "\"" + command + "\"";
		full_command_ = command;

		const wchar_t* wmode;
		switch (run_mode) {
			case OS::RUN_MODE_NONE:
				break;
			case OS::RUN_MODE_READ:
				wmode = L"rb";
				is_reader = true;
				break;
			case OS::RUN_MODE_WRITE:
				wmode = L"wb";
				is_writer = true;
				break;
			case OS::RUN_MODE_READWRITE:
				wmode = L"rwb";
				is_reader = true;
				is_writer = true;
				break;
		}

		const std::wstring wcommand = synfig::filesystem::Path(command).c_str(); // TODO: remove .c_str() when filesystem::Path() improves

		file = _wpopen(wcommand.c_str(), wmode);
		if (!file) {
			synfig::error(_("Error while trying to open a pipe: %s"), command.c_str());
			_wperror(wcommand.c_str());
		}
		return file != nullptr;
	}
	bool is_readable() const override { return is_reader && file != nullptr; }
	bool is_writable() const override { return is_writer && file != nullptr; }
	bool is_open() const override { return file != nullptr; };
	int close() override
	{
		int status = 0;
		if (file) {
			status = _pclose(file);
			file = nullptr;
		}
		initialized = false;
		return status;
	}
	void print(const std::string& text) override { fputs(text.c_str(), file); }
	size_t write(const void* ptr, size_t size, size_t n) override { return fwrite(ptr, size, n, file); }

	void flush() override { fflush(file); }

	std::string read_contents() override
	{
		if (!is_reader) {
			synfig::error("Should not try to readline() a non-readable pipe");
			return "";
		}
		std::string result;
		char buf[128];
		while(!feof(file)) {
			if(fgets(buf, 128, file) != nullptr)
				result += buf;
		}
		return result;
	}
	std::string read_contents(size_t max_bytes) override
	{
		if (!file) {
			synfig::error("Should not try to readline(size_t max_bytes) a non-readable pipe");
			return "";
		}
		std::string result;
		result.reserve(max_bytes);
		if(fgets(&result[0], max_bytes, file) != nullptr)
			return result;
		return "";
	}
	int getc() override { return fgetc(file); }
	int scanf(const char* __format, ...) override
	{
		va_list args;
		va_start(args, __format);
		int ret = vfscanf(file, __format, args);
		va_end(args);
		return ret;
	}
	bool eof() const override { return feof(file); };
};
#else
class RunPipeUnix : public OS::RunPipe
{
	FILE* read_file = nullptr;
	FILE* write_file = nullptr;
	bool initialized = false;
	pid_t pid = -1;

	// RunPipe interface
public:
	RunPipeUnix() = default;
	~RunPipeUnix()
	{
		RunPipeUnix::close();
	}

	bool open(std::string binary_path, const OS::RunArgs& binary_args, OS::RunMode run_mode, const OS::RunRedirectionFiles& redir_files) override
	{
		if (initialized) {
			synfig::error("A pipe should not be initialized twice! Ignored...");
			return false;
		}
		initialized = true;

		full_command_ = binary_path + " " + binary_args.get_string();

		int p[2];

		if (pipe(p)) {
			synfig::error(_("Unable to open pipe to %s (no pipe)"), binary_path.c_str());
			return false;
		}

		pid = fork();

		if (pid == -1) {
			synfig::error(_("Unable to open pipe to %s (pid == -1)"), binary_path.c_str());
			return false;
		}
		const bool is_reader = run_mode == OS::RUN_MODE_READ || run_mode == OS::RUN_MODE_READWRITE;
		const bool is_writer = run_mode == OS::RUN_MODE_WRITE || run_mode == OS::RUN_MODE_READWRITE;

		if (pid == 0) {
			// Child process

			// If requested, redirect stderr to a file
			if (!redir_files.std_err.empty()) {
				mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
				int err_fd = ::open(redir_files.std_err.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
				if (err_fd == -1) {
					synfig::error(_("Unable to open file to be stderr: %s [errno: %d]"), redir_files.std_err.c_str(), errno);
					perror(_("System error message:"));
					exit(-8000);
				}
				// Map err_fd as stderr
				if (dup2(err_fd, STDERR_FILENO) == -1) {
					synfig::error(_("Unable to open pipe to %s (dup2( err_fd, STDERR_FILENO ) == -1)"), binary_path.c_str());
					exit(-9000);
				}
			}

			// Redirect stdout to writer-pipe or to a file
			if (is_reader || !redir_files.std_out.empty()) {
				if (!redir_files.std_out.empty()) {
					::close(p[1]);
					mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
					p[1] = ::open(redir_files.std_out.c_str(), O_WRONLY | O_CREAT | O_TRUNC, mode);
					if (p[1] == -1) {
						synfig::error(_("Unable to open file to be stdout: %s [errno: %d]"), redir_files.std_out.c_str(), errno);
						perror(_("System error message:"));
						exit(-1000);
					}
				}
				// Map pipeout as stdout
				if (dup2(p[1], STDOUT_FILENO) == -1) {
					synfig::error(_("Unable to open pipe to %s (dup2( p[1], STDOUT_FILENO ) == -1)"), binary_path.c_str());
					exit(-2000);
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
						synfig::error(_("Unable to open file to be stdin: %s [errno: %d]"), redir_files.std_in.c_str(), errno);
						perror(_("System error message:"));
						exit(-5000);
					}
				}
				if (dup2(p[0], STDIN_FILENO) == -1) {
					synfig::error(_("Unable to open pipe to %s (dup2( p[0], STDIN_FILENO ) == -1)"), binary_path.c_str());
					exit(-6000);
				}
			}
			// Close the unneeded pipein
			::close(p[0]);

			char *args[binary_args.size()+2];
			size_t idx = 0;
			args[idx++] = &binary_path[0];
			for (size_t i = 0; i < binary_args.size(); i++)
			{
				const auto& arg = binary_args[i];
				args[idx++] = const_cast<char*>(&arg[0]);
			}
			args[idx] = nullptr;

			execvp(binary_path.c_str(), args);

			// We should never reach here unless the exec failed
			synfig::error(_("Unable to open pipe to %s (exec failed)"), binary_path.c_str());
			return false;
		} else {
			// Parent process
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
			synfig::error("Should not try to readline() a non-readable pipe");
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
			synfig::error("Should not try to readline(size_t max_bytes) a non-readable pipe");
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
OS::run_async(std::string binary_path, const RunArgs& binary_args, RunMode mode, const RunRedirectionFiles& redir_files)
{
	auto run_pipe = OS::RunPipe::create();
	if (!run_pipe)
		return nullptr;
	run_pipe->open(binary_path, binary_args, mode, redir_files);
	if (!run_pipe->is_open())
		return nullptr;
	return run_pipe;
}

bool
OS::launch_file_async(const std::string& file)
{
#ifdef _WIN32
	return 32<= INT_PTR( ShellExecuteW(NULL, NULL, synfig::filesystem::Path(file).c_str(), NULL, NULL, SW_SHOW) );
	// https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutew#return-value
#elif defined(__APPLE__)
	return 0 == system(strprintf("open \"%s\"", file.c_str()).c_str());
#else
	return 0 == system(strprintf("xdg-open \"%s\"", file.c_str()).c_str());
#endif
}
