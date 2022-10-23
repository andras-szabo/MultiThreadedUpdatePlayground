#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

#include "ThreadLog.h"

std::mutex inputMutex;
std::mutex renderMutex;
std::mutex cvMutex;

std::condition_variable frameStartCV;

std::atomic<bool> isRunning = false;
std::atomic<int> frameCount = 0;
std::atomic<bool> isVsyncOn = true;

std::unordered_map<int, std::string> methodIDLookup;

ThreadLog mainLog("Main");
ThreadLog sideLog("Side");


inline void Sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void SpinLock(int ms)
{
	const auto start = std::chrono::steady_clock::now();
	int elapsed = 0;
	do
	{
		const auto now = std::chrono::steady_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
					(now - start).count();
	} while (elapsed < ms);
}

void ProcessInput(ThreadLog& log)
{
	const std::lock_guard<std::mutex> lock(inputMutex);

	log.Log(10);
	SpinLock(1);
	log.Log(11);
}

void PrepareRendering(ThreadLog& log)
{
	const std::lock_guard<std::mutex> lock(renderMutex);

	log.Log(20);
	SpinLock(1);
	log.Log(21);
}

void RenderOutput(ThreadLog& log)
{
	log.Log(30);

	const int wait = isVsyncOn ? 14 : 2;
	SpinLock(wait);

	log.Log(31);
}

void MainThread()
{
	ThreadLog log("MainThread");

	while (isRunning)	
	{

		ProcessInput(log);
		PrepareRendering(log);

		{
			std::lock_guard<std::mutex> frameLock(cvMutex);
			frameStartCV.notify_one();
		}

		RenderOutput(log);

		frameCount++;
	}

	mainLog = log;
}

void Update(ThreadLog& log)
{
	const std::lock_guard<std::mutex> inputLock(inputMutex);
	const std::lock_guard<std::mutex> renderLock(renderMutex);

	log.Log(0);
	SpinLock(3);
	log.Log(1);
}

void SideThread()
{
	ThreadLog log("SideThread");

	int currentFrame = -1;

	while (isRunning)
	{
		if (currentFrame != frameCount)
		{
			std::unique_lock<std::mutex> frameLock(cvMutex);
			frameStartCV.wait(frameLock);
		}

		currentFrame = frameCount;
		Update(log);	
	}

	sideLog = log;
}

int main(int argc, char* argv[])
{
	isRunning = true;
	isVsyncOn = (argc > 1);

	if (isVsyncOn)
	{
		std::cout << "Vsync is ON.\n";
	}
	else
	{
		std::cout << "Vsync is OFF.\n";
	}

	std::thread mainThread(MainThread);
	std::thread sideThread(SideThread);

	Sleep(500);
	isRunning = false;

	sideThread.join();
	mainThread.join();
	
	std::cout << "End.\n";

	const auto logs = mainLog.MergeWith(sideLog);

	methodIDLookup[0] = "UpdateStart";
	methodIDLookup[1] = "UpdateEnd";
	methodIDLookup[10] = "InputStart";
	methodIDLookup[11] = "InputEnd";
	methodIDLookup[20] = "PreRenderStart";
	methodIDLookup[21] = "PreRenderEnd";
	methodIDLookup[30] = "RenderStart";
	methodIDLookup[31] = "RenderEnd";
	
	for (const auto& item : logs)
	{
		std::string name = "???";

		const auto lookup = methodIDLookup.find(item.ID);

		if (lookup != methodIDLookup.end())
		{
			name = lookup->second;
		}

		std::cout << item.Timestamp.time_since_epoch() << " ----- ID: " << item.ID << " -- " << name << "\n";
	}

	std::cout << "----\n";
}
