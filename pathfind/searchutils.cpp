//-----------------------------------------------------------------------------
/** @file searchutils.cpp
    @see searchutils.h

    $Id: searchutils.cpp,v 1.4 2003/06/09 20:39:51 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/searchutils.cpp,v $
*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include <stdlib.h>
#ifdef _WIN32
#define srandom srand
#endif // _WIN32
#ifdef _WIN32
#define random rand
#define _USE_MATH_DEFINES
#endif // _WIN32

#include "searchutils.h"

#include "search.h"



#include <math.h>

using namespace std;
using namespace PathFind;

//-----------------------------------------------------------------------------

bool SearchUtils::checkPathExists(const Environment& env,
                                  int start, int target)
{
    m_env = &env;
    m_target = target;
    m_mark.clear();
    m_mark.resize(env.getNumberNodes(), false);
    return searchPathExists(start, 0);
}

void SearchUtils::findRandomStartTarget(const Environment& env, int& start,
                                        int &target)
{
    int numberNodes = env.getNumberNodes();
    do
    {
        start = random() / (0x7fff / numberNodes + 1);
        target = random() / (0x7fff / numberNodes + 1);
    }
    while (! env.isValidNodeId(start)
           || ! env.isValidNodeId(target)
           || start == target
           || ! checkPathExists(env, start, target));
}

bool SearchUtils::searchPathExists(int node, int depth)
{
    assert(m_env->isValidNodeId(node));
    if (m_mark[node])
        return false;
    if (node == m_target)
        return true;
    m_mark[node] = true;
    assert(depth >= 0);
    if (m_successorStack.size() < static_cast<unsigned int>(depth + 1))
        m_successorStack.resize(depth + 1);
    assert(static_cast<unsigned int>(depth) < m_successorStack.size());
    vector<Environment::Successor>& successors = m_successorStack[depth];
    m_env->getSuccessors(node, NO_NODE, successors);
    int numberSuccessors = successors.size();
    for (int i = 0; i < numberSuccessors; ++i)
    {
        // Get reference on successor again, because resize could have
        // changed it.
        const Environment::Successor& successor = m_successorStack[depth][i];
        int targetNodeId = successor.m_target;
        assert(m_env->isValidNodeId(targetNodeId));
        if (searchPathExists(targetNodeId, depth + 1))
            return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
