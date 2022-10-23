#include <vector>
#include <string>
#include <chrono>

struct LogItem
{
	LogItem(int id, std::chrono::time_point<std::chrono::steady_clock> timestamp) :
		ID(id), Timestamp(timestamp)
	{
	}

	int ID = 0;
	std::chrono::time_point<std::chrono::steady_clock> Timestamp;
};

class ThreadLog
{
public:
	ThreadLog() = default;

	ThreadLog(const std::string& name) : _name(name) 
	{
		ResetClock();
		_log.reserve(1024);
	}

	ThreadLog& operator=(const ThreadLog& other)
	{
		_name = other._name;
		_log = other._log;
		_start = other._start;

		return *this;
	}

	void ResetClock()
	{
		_start = std::chrono::steady_clock::now();
	}

	void Log(int id)
	{
		const auto now = std::chrono::steady_clock::now();
		_log.emplace_back(id, now);
	}

	const std::vector<LogItem>& GetLog() const
	{
		return _log;
	}

	std::vector<LogItem> MergeWith(const ThreadLog& other) const
	{
		std::vector<LogItem> result;

		auto comp = [](const LogItem& a, const LogItem& b)
		{
			return a.Timestamp < b.Timestamp;
		};

		std::merge(_log.begin(), _log.end(),
			   other._log.begin(), other._log.end(),
			   std::back_inserter(result), comp);

		return result;
	}

private:
	std::string _name;
	std::vector<LogItem> _log;
	std::chrono::time_point<std::chrono::steady_clock> _start;
};
