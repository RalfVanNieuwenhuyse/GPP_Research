#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			
			auto startTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
			auto endTriangle = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);
			if (startTriangle == nullptr)
			{
				return finalPath;
			}
			if (endTriangle == nullptr)
			{
				return finalPath;
			}
			
			if (startTriangle == endTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}

			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto graph{ pNavGraph->Clone() };			

			//Create extra node for the Start Node (Agent's position
			NavGraphNode* startNode{ new NavGraphNode(graph->GetNextFreeNodeIndex(),-1,startPos) };
			graph->AddNode(startNode);
			for (auto edge : startTriangle->metaData.IndexLines)
			{
				if (pNavGraph->GetNodeIdxFromLineIdx(edge) != invalid_node_index)
				{
					graph->AddConnection(new GraphConnection2D(startNode->GetIndex(), pNavGraph->GetNodeIdxFromLineIdx(edge),
						startPos.Distance(graph->GetNode(pNavGraph->GetNodeIdxFromLineIdx(edge))->GetPosition())));					
				}
			}

			//Create extra node for the endNode
			NavGraphNode* endNode{ new NavGraphNode(graph->GetNextFreeNodeIndex(),-1,endPos) };
			graph->AddNode(endNode);
			for (auto edge : endTriangle->metaData.IndexLines)
			{
				if (pNavGraph->GetNodeIdxFromLineIdx(edge) != invalid_node_index)
				{
					graph->AddConnection(new GraphConnection2D(endNode->GetIndex(), pNavGraph->GetNodeIdxFromLineIdx(edge),
						endPos.Distance(graph->GetNode(pNavGraph->GetNodeIdxFromLineIdx(edge))->GetPosition())));
				}
			}
			//Run A star on new graph
			auto pathFinder = AStar<NavGraphNode, GraphConnection2D>(graph.get(), Elite::HeuristicFunctions::Chebyshev);
			auto temppath = pathFinder.FindPath(startNode,endNode);

			for (auto path : temppath)
			{
				debugNodePositions.push_back(path->GetPosition());
			}
			//OPTIONAL BUT ADVICED: Debug Visualisation

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto portals = SSFA::FindPortals(temppath, pNavGraph->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(portals);
			debugPortals = portals;
			
			return finalPath;
		}
	};
}
