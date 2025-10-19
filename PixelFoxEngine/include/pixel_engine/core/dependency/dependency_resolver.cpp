#include "pch.h"
#include "dependency_resolver.h"

#include "pixel_engine/exceptions/base_exception.h"

using namespace pixel_engine;

_Use_decl_annotations_
void DependencyResolver::Register(IManager* instance)
{
    if (instance && !m_registeredManagers.contains(instance))
    {
        m_registeredManagers.insert_or_assign(instance, true);
    }
}

void DependencyResolver::Clear()
{
    m_connections.clear();
    m_initOrder.clear();
    m_managerNames.clear();
    m_registeredManagers.clear();
}

_Use_decl_annotations_
bool DependencyResolver::Init()
{
    m_initOrder = GraphSort();

    if (m_initOrder.empty())
    {
        logger::error("Sorting Failed on init order — make sure the connections are correct");
        THROW_MSG("Failed to Initialize dependency resolver");
        return false;
    }

    const std::size_t total = m_initOrder.size();
    logger::progress_begin(0, "Dependency Initialization", total);

    std::size_t i = 0;
    for (auto it = m_initOrder.begin(); it != m_initOrder.end(); ++it, ++i)
    {
        IManager* mgr = *it;
        if (!mgr->OnInit())
        {
            logger::error("Failed to initialize manager: {}", mgr->GetManagerName());
            logger::progress_end(0, false);
            return false;
        }
        else
        {
            logger::info("initializing: {}", mgr->GetManagerName());
            logger::progress_update(0, static_cast<unsigned>(i + 1),
                mgr->GetManagerName() + " initialized!");
        }
    }
    logger::progress_end(0, true);
    return true;
}

_Use_decl_annotations_
bool DependencyResolver::UpdateLoopStart(float deltaTime) const
{
    static bool first_update = true;
    if (first_update) logger::debug("Update Loop Start!");

    const std::size_t n = m_initOrder.size();
    if (n == 0) return true;

    IManager** arr = new IManager * [n];
    {
        std::size_t idx = 0;
        for (auto it = m_initOrder.begin(); it != m_initOrder.end(); ++it, ++idx)
            arr[idx] = *it;
    }

    for (std::size_t i = n; i-- > 0; )
    {
        if (first_update)
        {
            logger::debug("Updating: {}", arr[i]->GetManagerName());
        }
        arr[i]->OnLoopStart(deltaTime);
    }

    delete[] arr;
    first_update = false;
    return true;
}

bool DependencyResolver::UpdateLoopEnd() const
{
    static bool first_update = true;
    if (first_update) logger::debug("Update Loop End!");

    const std::size_t n = m_initOrder.size();
    if (n == 0) return true;

    IManager** arr = new IManager * [n];
    {
        std::size_t idx = 0;
        for (auto it = m_initOrder.begin(); it != m_initOrder.end(); ++it, ++idx)
            arr[idx] = *it;
    }

    for (std::size_t i = n; i-- > 0; )
    {
        if (first_update)
        {
            logger::debug("Updating: {}", arr[i]->GetManagerName());
        }
        arr[i]->OnLoopEnd();
    }

    delete[] arr;
    first_update = false;
    return true;
}

bool DependencyResolver::Shutdown()
{
    bool ok = true;
    if (m_initOrder.empty()) return true;

    fox::list<IManager*> rev; // TODO: add reverse iterator on list
    for (IManager* mgr : m_initOrder) rev.push_front(mgr);
    

    for (IManager* mgr : rev)
    {
        if (!mgr) { continue; }
        const auto name = mgr->GetManagerName();
        if (!mgr->OnRelease())
        {
            ok = false;
            logger::error("Failed to properly destroy: {}", name);
        }
    }

    return ok;
}

fox::list<IManager*> DependencyResolver::GraphSort()
{
    fox::unordered_map<IManager*, bool> visited;
    fox::unordered_map<IManager*, bool> stack;
    fox::list<IManager*>                post;
    fox::list<IManager*>                sorted;

    for (auto it = m_registeredManagers.begin(); it != m_registeredManagers.end(); ++it)
    {
        IManager* node = (*it).first;
        if (!visited.contains(node))
        {
            GraphDFS(node, visited, stack, post);
            if (post.empty()) // cycle detected!
            {
                sorted.clear();
                return sorted;
            }
        }
    }

    for (auto it = post.begin(); it != post.end(); ++it)
        sorted.push_front(*it);

    return sorted;
}

_Use_decl_annotations_
void DependencyResolver::GraphDFS(
    IManager*                            node,
    fox::unordered_map<IManager*, bool>& visited,
    fox::unordered_map<IManager*, bool>& stack,
    fox::list<IManager*>&                post)
{
    if (stack.contains(node))
    {
        post.clear(); // cycle found
        return;
    }
    if (visited.contains(node)) return;

    stack[node]   = true;
    visited[node] = true;

    if (m_connections.contains(node))
    {
        auto& deps = m_connections.at(node);
        for (auto it = deps.begin(); it != deps.end(); ++it)
        {
            IManager* early = *it;
            GraphDFS(early, visited, stack, post);
            if (post.empty()) return; // cycle found!
        }
    }

    stack.erase(node);
    post.push_front(node);
}
