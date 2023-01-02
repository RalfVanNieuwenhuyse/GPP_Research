#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

#include <string>

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	m_CellHeight = m_SpaceHeight / rows;
	m_CellWidth = m_SpaceWidth / cols;

	Elite::Vector2 startPoint{0,0};

	for (int i = 0; i < m_NrOfRows ; i++)
	{
		for (int i = 0; i < m_NrOfCols; i++)
		{
			m_Cells.push_back(Cell(startPoint.x, startPoint.y, m_CellWidth, m_CellWidth));
			startPoint.x += m_CellWidth;
		}
		startPoint.x = 0;
		startPoint.y += m_CellWidth;
	}
	//m_Cells.resize(rows * cols);
	//std::cout << m_Cells.size() << '\n';
}

void CellSpace::AddAgent(SteeringAgent* agent)
{	
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	int oldIndex{ PositionToIndex(oldPos) };
	int newIndex{ PositionToIndex(agent->GetPosition()) };

	if (oldIndex != newIndex)
	{		
		m_Cells[oldIndex].agents.remove(agent);
		m_Cells[newIndex].agents.push_back(agent);
	}
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	m_NrOfNeighbors = 0;
	m_Neighbors.clear();

	int index{ PositionToIndex(agent->GetPosition()) };

	Elite::Rect neighbourhoodRect{ { agent->GetPosition().x - (queryRadius),agent->GetPosition().y - (queryRadius) }
		,queryRadius * 2.f,queryRadius * 2.f };

	for (int cellIndex = 0; cellIndex < m_Cells.size(); cellIndex++)
	{
		if (Elite::IsOverlapping(m_Cells[cellIndex].boundingBox, neighbourhoodRect))
		{			
			for (auto const& cellAgent : m_Cells[cellIndex].agents)
			{
				Elite::Vector2 DistanceBetween2Agents{ cellAgent->GetPosition() - agent->GetPosition() };
				if ((queryRadius * queryRadius) < DistanceBetween2Agents.MagnitudeSquared())
				{
					m_Neighbors.push_back(cellAgent);
					++m_NrOfNeighbors;
				}				
			}
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	for (size_t i = 0; i < m_Cells.size(); i++)
	{		

		
		std::string buffer{ std::to_string(m_Cells[i].agents.size()) };

		std::vector<Elite::Vector2> points = m_Cells[i].GetRectPoints();
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, {1,0,0,0.5f}, 0.4f);
		DEBUGRENDERER2D->DrawString(m_Cells[i].boundingBox.bottomLeft + Elite::Vector2{0.f,m_CellHeight}, buffer.c_str());
	}	
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	for (size_t i = 0; i < m_Cells.size(); i++)
	{
		if (IsPointInRect(m_Cells[i].boundingBox, pos))
		{
			return i;
		}
	}
	return 0;
}

bool CellSpace::IsPointInRect(Elite::Rect rectangle, Elite::Vector2 point) const
{
	bool leftCheckX{ point.x >= rectangle.bottomLeft.x };
	bool leftCheckY{ point.y >= rectangle.bottomLeft.y };
	bool rightCheckX{ point.x <= (rectangle.bottomLeft.x + rectangle.width) };
	bool rightCheckY{ point.y <= (rectangle.bottomLeft.y + rectangle.height) };
	return  leftCheckX && leftCheckY && rightCheckX && rightCheckY;
}