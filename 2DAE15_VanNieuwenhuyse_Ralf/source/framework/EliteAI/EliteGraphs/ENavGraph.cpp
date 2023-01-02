#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{	
	//1. Go over all the edges of the navigationmesh and create nodes
	auto lines = m_pNavMeshPolygon->GetLines();
	
	for (auto line : lines)
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() > 1)
		{
			
			this->AddNode(new NavGraphNode(this->GetNextFreeNodeIndex(),line->index,
				Vector2((line->p1.x + line->p2.x) / 2.f, (line->p1.y + line->p2.y) / 2.f)));			
		}
	}

	auto triangles = m_pNavMeshPolygon->GetTriangles();
	for (auto tri: triangles)
	{
		std::vector<Line*> tempLines;
		for (auto lineIndex : tri->metaData.IndexLines)
		{
			//m_pNavMeshPolygon->GetTrianglesFromLineIndex(lineIndex);			
			
			if (this->GetNodeIdxFromLineIdx(lineIndex) != invalid_node_index)
			{
				tempLines.push_back(lines[lineIndex]);
			}
		}

		if (tempLines.size() == 2)
		{
			AddConnection(new GraphConnection2D(GetNodeIdxFromLineIdx(tempLines[0]->index), GetNodeIdxFromLineIdx(tempLines[1]->index)));
		}
		else if(tempLines.size() == 3)
		{
			AddConnection(new GraphConnection2D(GetNodeIdxFromLineIdx(tempLines[0]->index), GetNodeIdxFromLineIdx(tempLines[1]->index)));
			AddConnection(new GraphConnection2D(GetNodeIdxFromLineIdx(tempLines[1]->index), GetNodeIdxFromLineIdx(tempLines[2]->index)));
			AddConnection(new GraphConnection2D(GetNodeIdxFromLineIdx(tempLines[2]->index), GetNodeIdxFromLineIdx(tempLines[0]->index)));
			
		}
		SetConnectionCostsToDistance();
		tempLines.clear();				
	}
	//2. Create connections now that every node is created
	
	//3. Set the connections cost to the actual distance
}

