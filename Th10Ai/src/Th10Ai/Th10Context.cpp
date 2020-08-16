#include "Th10Ai/Common.h"
#include "Th10Ai/Th10Context.h"

#include <Base/ScopeGuard.h>
#include <Base/Clock.h>

#include "Th10Ai/DllInject.h"

namespace th
{
	namespace posix_time = boost::posix_time;

	Th10Context::Th10Context() :
		m_data(nullptr)
	{
		HWND window = FindWindowW(L"BASE", L"搶曽晽恄榐丂乣 Mountain of Faith. ver 1.00a");
		if (window == nullptr)
			BASE_THROW(Exception(u8"东方风神录未运行。"));
		DWORD processId = 0;
		DWORD threadId = GetWindowThreadProcessId(window, &processId);

		m_memory = interprocess::managed_windows_shared_memory(interprocess::create_only,
			"Th10SharedMemory", 8 * 1024 * 1024);
		m_data = m_memory.construct<Th10SharedData>("Th10SharedData")();
		if (m_data == nullptr)
			BASE_THROW(Exception(u8"Th10SharedData名称已被使用。"));

		m_data->window = window;
		m_data->hooked = false;
		m_data->unhooked = false;
		m_data->updated = false;
		m_data->exited = false;
		m_data->updateTime = 0;

		std::string dllName = Apis::GetModuleDir() + "/Th10Hook.dll";
		DllInject::EnableDebugPrivilege();
		DllInject::Inject(processId, dllName);

		if (!timedWaitHook(3000))
			BASE_THROW(Exception(u8"Th10Hook初始化失败，详细信息请查看Th10Hook.log。"));
	}

	Th10Context::~Th10Context()
	{
		if (m_data != nullptr)
			m_memory.destroy_ptr(m_data);
	}

	void Th10Context::activate()
	{
		if (!SetForegroundWindow(m_data->window))
			BASE_THROW(WindowsError());
	}

	bool Th10Context::timedWaitHook(time_t ms)
	{
		bool notTimeout = true;
		interprocess::scoped_lock<interprocess::interprocess_mutex> lock(m_data->hookMutex);
		if (!m_data->hooked)
		{
			posix_time::ptime absTime = interprocess::microsec_clock::universal_time()
				+ posix_time::milliseconds(ms);
			notTimeout = m_data->hookCond.timed_wait(lock, absTime);
		}
		return notTimeout;
	}

	void Th10Context::notifyUnhook()
	{
		interprocess::scoped_lock<interprocess::interprocess_mutex> lock(m_data->unhookMutex);
		m_data->unhooked = true;
		m_data->unhookCond.notify_one();
	}

	bool Th10Context::waitUpdate()
	{
		interprocess::scoped_lock<interprocess::interprocess_mutex> lock(m_data->updateMutex);
		if (!m_data->updated)
			m_data->updateCond.wait(lock);
		m_data->updated = false;

		//Clock clock;
		//clock.update();
		//std::cout << clock.getTime() - m_data->updateTime << std::endl;

		return !m_data->exited;
	}

	const RawStatus& Th10Context::getStatus() const
	{
		return m_data->status;
	}
}
