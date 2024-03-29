//-----------------------------------------------------------------------------
/** @file tiling.cpp
    @see tiling.h

    $Id: tiling.cpp,v 1.42 2003/06/11 19:20:56 emarkus Exp $
    $Source: /usr/cvsroot/project_pathfind/tiling.cpp,v $
*/
//-----------------------------------------------------------------------------

#include <assert.h>
#include "tiling.h"

#include <ctype.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "search.h"

#ifdef _WIN32
#define srandom srand
#endif // _WIN32
#ifdef _WIN32
#define random rand
#define _USE_MATH_DEFINES
#endif // _WIN32

#include <math.h>
#include <time.h>

using namespace std;
using namespace PathFind;

#define MAXLINE 2048

//-----------------------------------------------------------------------------

TilingNodeInfo::TilingNodeInfo()
    : m_isObstacle(false),
      m_column(-1),
      m_row(-1)
{
}

TilingNodeInfo::TilingNodeInfo(bool isObstacle, int row, int column)
    : m_isObstacle(isObstacle),
      m_column(column),
      m_row(row)
{
}

//-----------------------------------------------------------------------------

Tiling::Tiling(Type type, int rows, int columns)
{
		srandom(time(NULL));
    init(type, rows, columns);
}

Tiling::Tiling(const Tiling & tiling, int horizOrigin, int vertOrigin,
               int width, int height)
{
    // init builds everything, except for the obstacles...
    init(tiling.getType(), width, height);
    // so we now put the obstacles in place
    for (int col = 0; col < width; col++)
    for (int row = 0; row < height; row++)
    {
        // get the local node
        int localNodeId = getNodeId(row, col);
        TilingNodeInfo& localNodeInfo = m_graph.getNodeInfo(localNodeId);
        // get the initial tiling node
        int nodeId = tiling.getNodeId(vertOrigin + row, horizOrigin + col);
        const TilingNodeInfo& nodeInfo = tiling.getNodeInfo(nodeId);
        // set obstacle for the local node
        if (nodeInfo.isObstacle())
            localNodeInfo.setObstacle(true);
        else
            localNodeInfo.setObstacle(false);
    }
}

Tiling::Tiling(LineReader& reader)
{
    int columns = -1;
    int rows = -1;
    bool typeFound = false;
    Type type = TILE;
    bool done = false;
    while (! done)
    {
        string line = reader.readLine();
        if (line == "")
            continue;
        if (line.size() > 0 && line[0] == '#')
            continue;
        istringstream in(line);
        string attribute;
        in >> attribute;
        if (! in)
            throw reader.createError("Missing attribute.");
        if (attribute == "type")
        {
            string typeString;
            in >> typeString;
            if (! in)
                throw reader.createError("Missing type value.");
            if (typeString == "tile")
                type = TILE;
            else if (typeString == "octile")
                type = OCTILE;
            else if (typeString == "octile_unicost")
                type = OCTILE_UNICOST;
            else if (typeString == "hex")
                type = HEX;
            else
                throw reader.createError("Invalid type value.");
            typeFound = true;
        }
        else if (attribute == "width")
        {
            in >> columns;
            if (! in || columns <= 0 || columns > LineReader::MAX_LINE)
                throw reader.createError("Invalid width.");
        }
        else if (attribute == "height")
        {
            in >> rows;
            if (! in || rows <= 0)
                throw reader.createError("Invalid height.");
        }
        else if (attribute == "map")
        {
            done = true;
        }
        else
            throw reader.createError("Unknown attribute.");
    }
    if (columns == -1 || rows == -1)
        throw reader.createError("Map without valid width / height.");
    if (! typeFound)
        throw reader.createError("Map without type.");
    init(type, rows, columns);
    readObstacles(reader);
}

void Tiling::addOutEdge(int nodeId, int row, int col, int cost)
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_columns)
        return;
    m_graph.addOutEdge(nodeId, getNodeId(row, col), TilingEdgeInfo(cost));
}

void Tiling::clearObstacles()
{
    int numberNodes = getNumberNodes();
    for (int nodeId = 0; nodeId < numberNodes; ++nodeId)
    {
        TilingNodeInfo& nodeInfo = m_graph.getNodeInfo(nodeId);
        nodeInfo.setObstacle(false);
    }
}

void Tiling::createEdges()
{
    assert(m_type == TILE
           || m_type == OCTILE
           || m_type == OCTILE_UNICOST
           || m_type == HEX);
    for (int row = 0; row < m_rows; ++row)
        for (int col = 0; col < m_columns; ++col)
        {
            int nodeId = getNodeId(row, col);
            addOutEdge(nodeId, row - 1, col, COST_ONE);
            addOutEdge(nodeId, row + 1, col, COST_ONE);
            addOutEdge(nodeId, row, col - 1, COST_ONE);
            addOutEdge(nodeId, row, col + 1, COST_ONE);
            if (m_type == OCTILE)
            {
                addOutEdge(nodeId, row + 1, col + 1, COST_SQRT2);
                addOutEdge(nodeId, row + 1, col - 1, COST_SQRT2);
                addOutEdge(nodeId, row - 1, col + 1, COST_SQRT2);
                addOutEdge(nodeId, row - 1, col - 1, COST_SQRT2);
            }
            else if (m_type == OCTILE_UNICOST)
            {
                addOutEdge(nodeId, row + 1, col + 1, COST_ONE);
                addOutEdge(nodeId, row + 1, col - 1, COST_ONE);
                addOutEdge(nodeId, row - 1, col + 1, COST_ONE);
                addOutEdge(nodeId, row - 1, col - 1, COST_ONE);
            }
            else if (m_type == HEX)
            {
                if (col % 2 == 0)
                {
                    addOutEdge(nodeId, row - 1, col + 1, COST_ONE);
                    addOutEdge(nodeId, row - 1, col - 1, COST_ONE);
                }
                else
                {
                    addOutEdge(nodeId, row + 1, col + 1, COST_ONE);
                    addOutEdge(nodeId, row + 1, col - 1, COST_ONE);
                }
            }
        }
}

void Tiling::createNodes()
{
    for (int row = 0; row < m_rows; ++row)
        for (int col = 0; col < m_columns; ++col)
        {
            int nodeId = getNodeId(row, col);
            m_graph.addNode(nodeId, TilingNodeInfo(false, row, col));
        }
}

vector<char> Tiling::getCharVector() const
{
    vector<char> result;
    int numberNodes = getNumberNodes();
    result.reserve(numberNodes);
    for (int i = 0; i < numberNodes; ++i)
    {
        if (m_graph.getNodeInfo(i).isObstacle())
            result.push_back('@');
        else
            result.push_back('.');
    }
    return result;
}

int Tiling::getHeuristic(int start, int target) const
{
    int colStart = start % m_columns;
    int colTarget = target % m_columns;
    int rowStart = start / m_columns;
    int rowTarget = target / m_columns;
    int diffCol = abs(colTarget - colStart);
    int diffRow = abs(rowTarget - rowStart);
    switch (m_type)
    {
    case HEX:
        // Vancouver distance
        // See P.Yap: Grid-based Path-Finding (LNAI 2338 pp.44-55)
        {
            int correction = 0;
            if (diffCol % 2 != 0)
            {
                if (rowTarget < rowStart)
                    correction = colTarget % 2;
                else if (rowTarget > rowStart)
                    correction = colStart % 2;
            }
            // Note: formula in paper is wrong, corrected below.  
            int dist = max(0, diffRow - diffCol / 2 - correction) + diffCol;
            return dist * COST_ONE;
        }
    case OCTILE_UNICOST:
        return max(diffCol, diffRow) * COST_ONE;
    case OCTILE:
        int maxDiff;
        int minDiff;
        if (diffCol > diffRow)
        {
            maxDiff = diffCol;
            minDiff = diffRow;
        }
        else
        {
            maxDiff = diffRow;
            minDiff = diffCol;
        }
        return minDiff * COST_SQRT2 + (maxDiff - minDiff) * COST_ONE;
    case TILE:
        return (diffCol + diffRow) * COST_ONE;
    default:
        assert(false);
        return 0;
    }
}
        
int Tiling::getMaxCost() const
{
    if (m_type == OCTILE)
        return COST_SQRT2;
    return COST_ONE;
}

int Tiling::getMaxEdges(Type type)
{
    switch (type)
    {
    case HEX:
        return 6;
    case OCTILE:
    case OCTILE_UNICOST:
        return 8;
    case TILE:
        return 4;
    }
    assert(false);
    return 0;
}

int Tiling::getMinCost() const
{
    return COST_ONE;
}

int Tiling::getNumberNodes() const
{
    return m_rows * m_columns;
}

void Tiling::getSuccessors(int nodeId, int lastNodeId,
                           vector<Successor>& result) const
{
    result.reserve(m_maxEdges);
    result.clear();
    const TilingNode& node = m_graph.getNode(nodeId);
    const TilingNodeInfo& nodeInfo = node.getInfo();
    if (nodeInfo.isObstacle())
    {
        assert(result.size() == 0);
        return;
    }
    const vector<TilingEdge>& edges = node.getOutEdges(); 
    for (vector<TilingEdge>::const_iterator i = edges.begin();
         i != edges.end(); ++i)
    {
        int targetNodeId = i->getTargetNodeId();
        assert(isValidNodeId(targetNodeId));
        const TilingNode& targetNode = m_graph.getNode(targetNodeId);
        const TilingNodeInfo& targetNodeInfo = targetNode.getInfo();
        if (targetNodeInfo.isObstacle())
            continue;
        if (lastNodeId != NO_NODE)
            if (pruneNode(targetNodeId, lastNodeId))
                continue;
        result.push_back(Successor(targetNodeId, i->getInfo().getCost()));
    }
#ifndef NDEBUG
    int resultSize = result.size();
    assert(resultSize <= m_maxEdges);
    if (lastNodeId != NO_NODE)
        switch (m_type)
        {
        case HEX:
        case TILE:
            assert(resultSize <= 3);
            break;
        case OCTILE:
        case OCTILE_UNICOST:
            assert(resultSize <= 5);
            break;
        }
#endif
}

void Tiling::init(Type type, int rows, int columns)
{
    m_type = type;
    m_maxEdges = getMaxEdges(type);
    m_rows = rows;
    m_columns = columns;
    m_graph.clear();
    createNodes();
    createEdges();
}

bool Tiling::isValidNodeId(int nodeId) const
{
    return nodeId >= 0 && nodeId < getNumberNodes();
}

void Tiling::printFormatted(ostream& o) const
{
    printFormatted(o, getCharVector());
}

void Tiling::printFormatted(ostream& o, const vector<char>& chars) const
{
    for (int row = 0; row < m_rows; ++row)
    {
        for (int col = 0; col < m_columns; ++col)
        {
            int nodeId = getNodeId(row, col);
            o << chars[nodeId];
        }
        o << '\n';
    }
}

void Tiling::printFormatted(ostream& o, int start, int target) const
{
    vector<char> chars = getCharVector();
    chars[start] = 'S';
    chars[target] = 'T';
    printFormatted(o, chars);
}

void Tiling::printFormatted(ostream& o, const vector<int>& path) const
{
    vector<char> chars = getCharVector();
    if (path.size() > 0)
    {
        for (vector<int>::const_iterator i = path.begin();
             i != path.end(); ++i)
            chars[*i] = 'x';
        chars[*path.begin()] = 'T';
        chars[*(path.end() - 1)] = 'S';
    }
    printFormatted(o, chars);
}

void Tiling::printLabels(ostream& o, const vector<char>& labels) const
{
    assert(labels.size() == static_cast<unsigned int>(getNumberNodes()));
    vector<char> chars = getCharVector();
    vector<char>::iterator j = chars.begin();
    for (vector<char>::const_iterator i = labels.begin();
         i != labels.end(); ++i, ++j)
        if (*i != ' ')
            *j = *i;
    printFormatted(o, chars);
}


void Tiling::printPathAndLabels(ostream& o, const vector<int>& path,
                                const vector<char>& labels) const
{
    assert(labels.size() == static_cast<unsigned int>(getNumberNodes()));

    vector<char> chars = getCharVector();

    vector<char>::iterator j = chars.begin();
    for (vector<char>::const_iterator i = labels.begin();
         i != labels.end(); ++i, ++j)
        if (*i != ' ')
            *j = *i;

    if (path.size() > 0)
    {
        for (vector<int>::const_iterator i = path.begin();
             i != path.end(); ++i)
            chars[*i] = 'x';
        chars[*path.begin()] = 'T';
        chars[*(path.end() - 1)] = 'S';
    }

    printFormatted(o, chars);
}


bool Tiling::pruneNode(int targetNodeId, int lastNodeId) const
{
    if (targetNodeId == lastNodeId)
        return true;
    if (m_type == TILE)
        return false;
    const TilingNode& lastNode = m_graph.getNode(lastNodeId);
    const vector<TilingEdge>& edges = lastNode.getOutEdges(); 
    for (vector<TilingEdge>::const_iterator i = edges.begin();
         i != edges.end(); ++i)
    {
        if (i->getTargetNodeId() == targetNodeId)
            return true;
    }
    return false;
}

void Tiling::readObstacles(LineReader& reader)
{
    for (int row = 0; row < m_rows; ++row)
    {
        string line = reader.readLine();
        istringstream in(line);
        for (int col = 0; col < m_columns; ++col)
        {
            bool isObstacle;
            char c;
            in.get(c);
            if (! in)
                throw reader.createError("Unexpected end of stream.");
            if (c == '@')
                isObstacle = true;
            else if (c == '.')
                isObstacle = false;
            else
                throw reader.createError("Unknown charcter.");
            int nodeId = getNodeId(row, col);
            TilingNodeInfo& nodeInfo = m_graph.getNodeInfo(nodeId);
            if (! nodeInfo.isObstacle())
                nodeInfo.setObstacle(isObstacle);
            
        }
    }
}


bool Tiling::conflictDiag(int row, int col, int roff, int coff)
{
    // Avoid generating cofigurations like:
    //
    //    @   or   @
    //     @      @
    //
    // that favor one grid topology over another.
    if ( (row+roff < 0) || (row+roff >= m_rows) || 
         (col+coff < 0) || (col+coff >= m_columns) )
        return false;

    if ( (m_graph.getNodeInfo(getNodeId(row+roff,col+coff))).isObstacle() )
    {
        if ( !m_graph.getNodeInfo(getNodeId(row,col+coff)).isObstacle() &&
             !m_graph.getNodeInfo(getNodeId(row+roff,col)).isObstacle() )
        return true;
    }
    return false;
} 


void Tiling::setObstacles(float obstaclePercentage, bool avoidDiag )
{
    clearObstacles();
    int numberNodes = getNumberNodes();
    int numberObstacles = static_cast<int>(obstaclePercentage * numberNodes);
    for (int count = 0; count < numberObstacles; )
    {
        int nodeId = random() / (0x7fff / numberNodes + 1);
        TilingNodeInfo& nodeInfo = m_graph.getNodeInfo(nodeId);
        if (! nodeInfo.isObstacle())
        {
            if ( avoidDiag )
            {
                int row = nodeInfo.getRow();
                int col = nodeInfo.getColumn();

                if ( !conflictDiag(row,col,-1,-1) && 
                     !conflictDiag(row,col,-1,+1) &&
                     !conflictDiag(row,col,+1,-1) &&
                     !conflictDiag(row,col,+1,+1) )
                {
                    nodeInfo.setObstacle(true);
                    ++count;
                }
            }
            else 
            {
                nodeInfo.setObstacle(true);
                ++count;
            }
        }
    }
}

//-----------------------------------------------------------------------------
