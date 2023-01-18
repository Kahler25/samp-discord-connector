#pragma once

#include <samplog/samplog.hpp>
#include "Singleton.hpp"
#include "Error.hpp"

#include <fmt/format.h>

using samplog::PluginLogger_t;
using samplog::samplog_LogLevel;
using samplog::AmxFuncCallInfo;


class DebugInfoManager : public Singleton<DebugInfoManager>
{
	friend class Singleton<DebugInfoManager>;
	friend class ScopedDebugInfo;
private:
	DebugInfoManager() = default;
	~DebugInfoManager() = default;

private:
	bool m_Available = false;

	AMX *m_Amx = nullptr;
	std::vector<AmxFuncCallInfo> m_Info;
	const char *m_NativeName = nullptr;

private:
	void Update(AMX * const amx, const char *func);
	void Clear();

public:
	inline AMX * GetCurrentAmx()
	{
		return m_Amx;
	}
	inline const decltype(m_Info) &GetCurrentInfo()
	{
		return m_Info;
	}
	inline bool IsInfoAvailable()
	{
		return m_Available;
	}
	inline const char *GetCurrentNativeName()
	{
		return m_NativeName;
	}
};


class Logger : public Singleton<Logger>
{
	friend class Singleton<Logger>;
	friend class ScopedDebugInfo;
private:
	Logger() :
		m_Logger("discord-connector")
	{ }
	~Logger() = default;

public:
	inline bool IsLogLevel(samplog_LogLevel level)
	{
		return false;
	}

	template<typename... Args>
	inline void Log(samplog_LogLevel level, const char *msg)
	{
		return;
	}

	template<typename... Args>
	inline void Log(samplog_LogLevel level, const char *format, Args &&...args)
	{
		return;
	}

	template<typename... Args>
	inline void Log(samplog_LogLevel level, std::vector<AmxFuncCallInfo> const &callinfo,
		const char *msg)
	{
		return;
	}

	template<typename... Args>
	inline void Log(samplog_LogLevel level, std::vector<AmxFuncCallInfo> const &callinfo,
		const char *format, Args &&...args)
	{
		return;
	}

	// should only be called in native functions
	template<typename... Args>
	void LogNative(samplog_LogLevel level, const char *fmt, Args &&...args)
	{
		if (!IsLogLevel(level))
			return;

		if (DebugInfoManager::Get()->GetCurrentAmx() == nullptr)
			return; //do nothing, since we're not called from within a native func

		auto msg = fmt::format("{:s}: {:s}",
			DebugInfoManager::Get()->GetCurrentNativeName(),
			fmt::format(fmt, std::forward<Args>(args)...));

		if (DebugInfoManager::Get()->IsInfoAvailable())
			Log(level, DebugInfoManager::Get()->GetCurrentInfo(), msg.c_str());
		else
			Log(level, msg.c_str());
	}

	template<typename T>
	inline void LogNative(const CallbackError<T> &error)
	{
		LogNative(samplog_LogLevel::ERROR, "{} error: {}", error.module(), error.msg());
	}

private:
	PluginLogger_t m_Logger;

};


class ScopedDebugInfo
{
public:
	ScopedDebugInfo(AMX * const amx, const char *func,
		cell * const params, const char *params_format = "");
	~ScopedDebugInfo()
	{
		DebugInfoManager::Get()->Clear();
	}
	ScopedDebugInfo(const ScopedDebugInfo &rhs) = delete;
};
