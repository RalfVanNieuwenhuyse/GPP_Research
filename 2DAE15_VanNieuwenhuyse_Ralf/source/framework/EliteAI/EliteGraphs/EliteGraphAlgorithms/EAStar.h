#pragma once

#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;
		//NodeRecord currentRecord;

		NodeRecord currentRecord{ pStartNode,nullptr,0.f,GetHeuristicCost(pStartNode,pGoalNode) };

		openList.push_back(currentRecord);

		while (!openList.empty())
		{
			currentRecord = *std::min_element(openList.begin(), openList.end());
			if (currentRecord.pNode == pGoalNode)
			{
				break;
			}

			for (auto& connections : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				const float totalGCostSoFar{ currentRecord.costSoFar + connections->GetCost() };
				
				bool foundExisting{ false };

				for (auto closedListRecord : closedList)
				{
					if (connections->GetTo() == closedListRecord.pNode->GetIndex())
					{
						if (totalGCostSoFar >= closedListRecord.costSoFar )
						{
							foundExisting = true;
							continue;
							
						}
						else
						{
							NodeRecord existingRecord{closedListRecord};

							closedListRecord.pConnection = connections;
							closedListRecord.costSoFar = totalGCostSoFar;
							closedListRecord.estimatedTotalCost = totalGCostSoFar + GetHeuristicCost(closedListRecord.pNode, pGoalNode);

							

							closedList.erase(std::remove(closedList.begin(), closedList.end(), existingRecord));
						}
					}
				}
				//closedList.push_back(closedListRecord);				

				for (auto openListRecord : openList)
				{
					if (connections->GetTo() == openListRecord.pNode->GetIndex())
					{
						if (totalGCostSoFar >= openListRecord.costSoFar)
						{
							foundExisting = true;
							continue;
						}
						else
						{
							NodeRecord existingRecord{ openListRecord };

							openListRecord.pConnection = connections;
							openListRecord.costSoFar = totalGCostSoFar;
							openListRecord.estimatedTotalCost = totalGCostSoFar + GetHeuristicCost(openListRecord.pNode, pGoalNode);

							

							openList.erase(std::remove(openList.begin(), openList.end(), existingRecord));
						}

					}
				}//openList loop
				if (!foundExisting)
				{
					NodeRecord nodeRecord{};

					nodeRecord.pNode = m_pGraph->GetNode(connections->GetTo());

					nodeRecord.pConnection = connections;
					nodeRecord.costSoFar = totalGCostSoFar;
					nodeRecord.estimatedTotalCost = totalGCostSoFar + GetHeuristicCost(nodeRecord.pNode, pGoalNode);

					openList.push_back(nodeRecord);
				}
				

			}//conection loop

			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			closedList.push_back(currentRecord);
		}//while loop
		
		while (currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);				
				
			if (currentRecord.pConnection != nullptr)
			{
				for (auto listitem : closedList)
				{
					if (listitem.pNode->GetIndex() == currentRecord.pConnection->GetFrom())
					{
						currentRecord = listitem;
						break;
					}					
				}
			}			
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}