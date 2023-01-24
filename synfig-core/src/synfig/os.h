/* === S Y N F I G ========================================================= */
/*!	\file os.h
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

/* === S T A R T =========================================================== */

#ifndef SYNFIG_OS_H
#define SYNFIG_OS_H

/* === H E A D E R S ======================================================= */

#include <memory>
#include <string>
#include <vector>

#include <synfig/filesystem.h> // synfig::filesystem::Path

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === P R O C E D U R E S ================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

namespace OS {

/**
 * A list of arguments to a command that will be run via OS::RunPipe
 */
class RunArgs : public std::vector<std::string>
{
public:
	// Modifiers -------------------------
	void push_back(const std::string& arg);
	void push_back(const char* arg);
	void push_back(const synfig::filesystem::Path& file);
	void push_back(const std::initializer_list<std::string>& args);
	void push_back(const std::initializer_list<const char*>& args);
	void push_back(const std::pair<std::string, synfig::filesystem::Path>& arg_pair);

	// Query -----------------------------

	/**
	 * Join the arguments into a single string.
	 * It uses a single regular space as separator.
	 */
	std::string get_string() const;
	/**
	 * Similar to get_string() but with OS native encoding
	 * (UTF-16 for MS Windows, and UTF-8 for others)
	 */
	std::string native() const;
};

/**
 * What type of communication the current process
 * will have with the command run via RunPipe:
 * - RUN_MODE_NONE: None
 * - RUN_MODE_READ: current process reads the child process' stdout
 * - RUN_MODE_WRITE: current process writes to the child process' stdin
 */
enum RunMode {
	RUN_MODE_NONE,
	RUN_MODE_READ,
	RUN_MODE_WRITE,
	RUN_MODE_READWRITE,
};

/**
 * If a field is set, it would be the target file of the standard I/O stream
 * redirection, and it prevents (direct) communication between current and
 * child process.
 * Let a field empty/unset, and it will use system standards (stderr, stdout
 * and stdin file descriptors).
 */
struct RunRedirectionFiles
{
	std::string std_err;
	std::string std_out;
	std::string std_in;
};

/**
 * An object that allows to run an external software.
 * It's possible to transfer data between the current process and external software.
 *
 * Example code:
 *
 * auto my_pipe = OS::RunPipe::create();
 *
 * // Run mybinary.exe without any args.
 * // It will read stdout output from mybinary.exe
 * // Redirect stderr of mybinary.exe to stderr.log file.
 * if (!my_pipe->open("mybinary.exe", {}, OS::RUN_MODE_READ, {"stderr.log", "", ""})) {
 *   synfig::error("Can't run mybinary.exe!");
 *   return;
 * }
 *
 * // get (and wait for) all stdout contents from mybinary.exe
 * auto contents = my_pipe->read_contents();
 *
 * my_pipe->close();
 */
struct RunPipe {
	typedef std::unique_ptr<RunPipe> Handle;

	static Handle create();

	virtual ~RunPipe() = default;

	/**
	 * Run an external software.
	 * It is possible to read strings it sends to its stdout and
	 * write strings to its stdin to be read by the running external software.
	 * @see read_contents(), getc(), printf(), print(), write()
	 *
	 * Its stderr and stdout can be logged into files via @a redir_files.
	 * In the same way, its stdin can be piped from a file too.
	 * In any of these redirection cases, the equivalent read/write operation
	 * is not possible anymore.
	 * Example:
	 * 1) One can set mode as OS::RUN_MODE_READ and set a redirection file to
	 * stdout. It could then normally perform read_contents() or getc() calls.
	 * The stdout strings would be all written into the desired file.
	 * 2) One can set mode as OS::RUN_MODE_READWRITE and set a redirection file to
	 * stdout. It could then normally perform write() or print() calls.
	 * However, even though it should be also a readable pipe (to read data from
	 * its stdout), the stdout strings would be all written into the desired file,
	 * as it has been set. So read_contents(), getc() and alike would not work.
	 *
	 * You should @c close() this object before calling @c open() again.
	 *
	 * @param binary_path The external software path
	 * @param binary_args The arguments to be passed to external software
	 * @param mode The communication direction (read/write), if any, between the current process and the external software
	 * @param redir_files Filenames to log stderr or stdout contents if no direct communication is desired.
	 *                    If an filename is provided to stdin, it will be the data source for this channel.
	 * @return true if success; false otherwise.
	 */
	virtual bool open(std::string binary_path, const RunArgs& binary_args, RunMode mode, const RunRedirectionFiles& redir_files = {}) = 0;
	/**
	 * Stop running the external software.
	 * @return the exit status.
	 */
	virtual int close() = 0;

	// status methods
	virtual bool is_readable() const = 0;
	virtual bool is_writable() const = 0;
	virtual bool is_open() const = 0;

	// write methods
	virtual void printf(const char *__format, ...);
	virtual void print(const std::string& text) = 0;
	virtual size_t write(const void *ptr, size_t size, size_t n) = 0;
	virtual void flush() = 0;

	// read methods
	/** read everything coming from stdout (until get an EOF) and return them. */
	virtual std::string read_contents() = 0;
	/** read at most @a max_bytes coming from stdout and return them. */
	virtual std::string read_contents(size_t max_bytes) = 0;
	/** read a byte coming from stdout. */
	virtual int getc() = 0;
	virtual int scanf(const char *__format, ...) = 0;
	virtual bool eof() const = 0;

	// query methods
	/**
	 * The equivalent command line string:
	 *  binary_path followed by binary_args both provided when open() is called.
	 */
	const std::string& get_command() const;

protected:
	RunPipe() = default;

	std::string full_command_;
};

/** Run an executable binary with pipes for communication or stdout/stdin redirection to files */
RunPipe::Handle run_async(std::string binary_path, const RunArgs& binary_args, RunMode mode, const RunRedirectionFiles& redir_files = {});

/**
 * Run an executable binary.
 * It returns from the call after the program finishes only.
 *
 * Like system() call, but without wchar problems and the chance to log stdout/stderr via redir_files.
 * @see run_async()
 */
bool run_sync(std::string binary_path, const RunArgs& binary_args, const std::string& stdout_redir_file = "", const std::string& stderr_redir_file = "");

/** Launch a file with its default application */
bool launch_file_async(const std::string& file);

} // END of namespace OS

} // END of namespace synfig

/* === E N D =============================================================== */

#endif // SYNFIG_OS_H
