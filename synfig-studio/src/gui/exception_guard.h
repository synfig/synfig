#ifndef SYNFIG_EXCEPTION_GUARD_H
#define SYNFIG_EXCEPTION_GUARD_H

#include <ETL/stringf>
#include <synfig/general.h> // for synfig::error()

#define SYNFIG_EXCEPTION_GUARD_BEGIN() \
	{ \
		int _exception_guard_error_code = 0; \
		std::string _exception_guard_error_str; \
		try {

/// Normally, you shouldn't use this macro directly
#define SYNFIG_EXCEPTION_GUARD_END_COMMON \
		} \
		catch(int ret) \
		{ \
			_exception_guard_error_str = etl::strprintf("%s Uncaught Exception:int: %i", __FUNCTION__, ret); \
			_exception_guard_error_code = ret; \
		} \
		catch(std::string& str) \
		{ \
			_exception_guard_error_str = etl::strprintf("%s Uncaught Exception:string: %s", __FUNCTION__, str.c_str()); \
			_exception_guard_error_code = 1001; \
		} \
		catch(std::exception& x) \
		{ \
			_exception_guard_error_str = etl::strprintf("%s Standard Exception: %s", __FUNCTION__, x.what()); \
			_exception_guard_error_code = 1002; \
		} \
		catch(Glib::Exception& x) \
		{ \
			_exception_guard_error_str = etl::strprintf("%s GLib Exception: %s", __FUNCTION__, x.what().c_str()); \
			_exception_guard_error_code = 1003; \
		} \
		catch(...) \
		{ \
			_exception_guard_error_str = etl::strprintf("%s Uncaught Exception:unknown type", __FUNCTION__); \
			_exception_guard_error_code = 1100; \
		}

/// It should return (void), no matter what happens
#define SYNFIG_EXCEPTION_GUARD_END() \
	SYNFIG_EXCEPTION_GUARD_END_COMMON \
		if (!_exception_guard_error_str.empty()) { \
			synfig::error("%s", _exception_guard_error_str.c_str()); \
			return; \
		} else { \
			return; \
		} \
	}

/// It should return a boolean. On success, return success_value; inverse value otherwise
#define SYNFIG_EXCEPTION_GUARD_END_BOOL(success_value) \
	SYNFIG_EXCEPTION_GUARD_END_COMMON \
		if (!_exception_guard_error_str.empty()) { \
			synfig::error("%s", _exception_guard_error_str.c_str()); \
			return !success_value; \
		} else { \
			return success_value; \
		} \
	}

/// It should return an integer. On success, return success_value; internal code otherwise
#define SYNFIG_EXCEPTION_GUARD_END_INT(success_value) \
	SYNFIG_EXCEPTION_GUARD_END_COMMON \
		if (!_exception_guard_error_str.empty()) { \
			synfig::error("%s", _exception_guard_error_str.c_str()); \
			return _exception_guard_error_code; \
		} else { \
			return success_value; \
		} \
	}

/// It should return a pointer. On success, return success_value; nullptr otherwise
#define SYNFIG_EXCEPTION_GUARD_END_NULL(success_value) \
	SYNFIG_EXCEPTION_GUARD_END_COMMON \
		if (!_exception_guard_error_str.empty()) { \
			synfig::error("%s", _exception_guard_error_str.c_str()); \
			return nullptr; \
		} else { \
			return success_value; \
		} \
	}

#endif // SYNFIG_EXCEPTION_GUARD_H
