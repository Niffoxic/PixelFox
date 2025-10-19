#include "pch.h"
#include "dependency_resolver.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/exceptions/base_exception.h"

using namespace pixel_engine;

_Use_decl_annotations_
void DependencyResolver::Register(IManager* instance)
{
	if (instance && not m_registeredManagers.contains(instance))
	{
		m_registeredManagers.insert_or_assign(instance, true);
	}
}

void DependencyResolver::Clear()
{
	m_connections		.clear();
	m_initOrder			.clear();
	m_managerNames		.clear();
	m_registeredManagers.clear();
}

_Use_decl_annotations_
bool DependencyResolver::Init()
{
	m_initOrder = GraphSort();

	if (m_initOrder.empty())
	{
		logger::error("Sorting Failed on init order make sure the connections are correct");
		THROW_MSG("Failed to Initialize dependency resolver");
		return false;
	}

	logger::progress_begin(0, "Dependency Initialization", m_initOrder.size());
	for (int i = 0; i < m_initOrder.size(); i++)
	{
		if (!m_initOrder[i]->OnInit())
		{
			logger::error("Failed to initialize manager: {}", m_initOrder[i]->GetManagerName());
			logger::progress_end(0, false);
			return false;
		}
		else
		{
			logger::progress_update(0, i + 1,
				m_initOrder[i]->GetManagerName() + "initialized!");
		}
	}
	logger::progress_end(0, true);
	return true;
}

_Use_decl_annotations_
bool DependencyResolver::UpdateLoopStart(float deltaTime) const
{
	static int first_update = false;
	if (first_update) logger::debug("Update Loop Start!");
	for (int i = m_initOrder.size() - 1; i >= 0; i--) 
	{
		if (first_update)
		{
			logger::debug("Updating: {}", m_initOrder[i]->GetManagerName());
		}
		m_initOrder[i]->OnLoopStart(deltaTime);
	}
	first_update = true;
	return true;
}

bool DependencyResolver::UpdateLoopEnd() const
{
	static int first_update = false;
	if (first_update) logger::debug("Update Loop End!");
	for (int i = m_initOrder.size() - 1; i >= 0; i--)
	{
		if (first_update)
		{
			logger::debug("Updating: {}", m_initOrder[i]->GetManagerName());
		}
		m_initOrder[i]->OnLoopEnd();
	}
	first_update = true;
	return true;
}

bool DependencyResolver::Shutdown()
{
	bool flag = true;
	for (int i = m_initOrder.size() - 1; i >= 0; i--)
	{
		if (m_initOrder[i])
		{
			auto managerName = m_initOrder[i]->GetManagerName();
			if (!m_initOrder[i]->OnRelease())
			{
				flag = false;
				logger::error("Failed to properly destroy: {}", managerName);
			}
		}
	}
	return flag;
}

fox::vector<IManager*> DependencyResolver::GraphSort()
{
	fox::unordered_map<IManager*, bool> visited;
	fox::unordered_map<IManager*, bool> recursionStack;
	fox::vector<IManager*>				sorted;

	
}

_Use_decl_annotations_
void DependencyResolver::GraphDFS(
	const IManager* node,
	fox::unordered_map<IManager*, bool>& visited,
	fox::unordered_map<IManager*, bool>& stack,
	fox::vector<IManager*> sorted)
{
}
