#include "stdafx.h"

#include "App_FlowFields.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Shared/NavigationColliderElement.h"

using namespace Elite;

App_FlowFields::~App_FlowFields()
{
	SAFE_DELETE(m_pGraphEditor);
	SAFE_DELETE(m_pGraphRenderer);
	SAFE_DELETE(m_pGridGraph);

	for (auto& pAgent : m_pAgents )
	{
		SAFE_DELETE(pAgent);
	}
}

void App_FlowFields::Start()
{
	float worldWith{ static_cast<float>(COLUMNS * m_SizeCell) };
	float worldHeight{ static_cast<float>(ROWS * m_SizeCell) };


	for (int i{}; i < m_NrOfAgents; ++i)
	{
		m_pAgents.push_back(new SteeringAgent());
		m_pAgents[i]->SetMaxLinearSpeed(20.f);
		m_pAgents[i]->SetAutoOrient(true);	
		
		Vector2 randomStartPos{ static_cast<float>(rand() % static_cast<int>(worldWith-10)), 
			static_cast<float>(rand() % static_cast<int>(worldHeight-10)) };
		m_pAgents[i]->SetPosition(randomStartPos);
	}

	m_pGraphEditor = new GraphEditor();
	m_pGraphRenderer = new GraphRenderer();
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(80.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2{ (worldWith / 2.f)+15.f, worldHeight / 2.f });
	MakeGridGraph();
	ResetFields();

	m_EndNodeIndex = 91;


	MakePath();
	
}

void App_FlowFields::Update(float deltaTime)
{
	for (const auto& agent: m_pAgents)
	{
		agent->Update(deltaTime);
		
		const auto agentNode{ m_pGridGraph->GetNodeIdxAtWorldPos(agent->GetPosition()) };

		//Check if agent is invalid place
		if (agentNode == invalid_node_index)
		{	
			agent->SetPosition(Vector2{ static_cast<float>(COLUMNS * m_SizeCell) / 2.f, static_cast<float>(ROWS * m_SizeCell) / 2.f });
			continue;
		}

		switch (m_VectorField[agentNode])
		{
		case App_FlowFields::VectorDir::invalid:
			agent->SetLinearVelocity(Vector2{ 0.f,0.f }*agent->GetMaxLinearSpeed());
			break;

		case App_FlowFields::VectorDir::up:
			agent->SetLinearVelocity(Vector2{ 0.f,1.f }*agent->GetMaxLinearSpeed());
			break;

		case App_FlowFields::VectorDir::down:
			agent->SetLinearVelocity(Vector2{ 0.f,-1.f }*agent->GetMaxLinearSpeed());
			break;

		case App_FlowFields::VectorDir::left:
			agent->SetLinearVelocity(Vector2{ -1.f,0.f }*agent->GetMaxLinearSpeed());
			break;

		case App_FlowFields::VectorDir::right:
			agent->SetLinearVelocity(Vector2{ 1.f,0.f }*agent->GetMaxLinearSpeed());
			break;

		case App_FlowFields::VectorDir::upRight:
			agent->SetLinearVelocity(Vector2{ 1.f,1.f }*agent->GetMaxLinearSpeed());
			break;

		case App_FlowFields::VectorDir::upLeft:
			agent->SetLinearVelocity(Vector2{ -1.f,1.f }*agent->GetMaxLinearSpeed());
			break;

		case App_FlowFields::VectorDir::downRight:
			agent->SetLinearVelocity(Vector2{ 1.f,-1.f }*agent->GetMaxLinearSpeed());
			break;

		case App_FlowFields::VectorDir::downLeft:
			agent->SetLinearVelocity(Vector2{ -1.f,-1.f }*agent->GetMaxLinearSpeed());
			break;

		default:
			break;
		}
			
	}

	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeIdxAtWorldPos(mousePos);
				
		m_EndNodeIndex = closestNode;
		
		MakePath();		
		
	}

	UpdateImGui();	

	if (m_pGraphEditor->UpdateGraph(m_pGridGraph))
	{
		
		MakePath();
	}

}

void App_FlowFields::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 200;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: target");
		ImGui::Text("RMB: start");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("Flow fields");
		ImGui::Spacing();

		ImGui::Text("Middle Mouse to change endpoint");
		ImGui::Text("Left mouse to change ground type select left corner");

		ImGui::Spacing();
		ImGui::Text("Debug");
		

		ImGui::Checkbox("Grid", &m_DebugSettings.DrawNodes);
		ImGui::Checkbox("NodeNumbers", &m_DebugSettings.DrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_DebugSettings.DrawConnections);
		ImGui::Checkbox("Connections Costs", &m_DebugSettings.DrawConnectionCosts);
		ImGui::Checkbox("Cost field", &m_DebugSettings.DrawCostField);
		ImGui::Checkbox("Integration field", &m_DebugSettings.DrawIntergartionField);
		ImGui::Checkbox("Vector field", &m_DebugSettings.DrawVectors);
		ImGui::Spacing();

		ImGui::LabelText(std::to_string(m_pAgents.size()).c_str(),"Amount of agents:");
		if (ImGui::Button("Add agent"))
		{
			AddAgent();
		}
		if (ImGui::Button("Add 10 agent"))
		{
			for (int i{0}; i < 10; i++)
			{
				AddAgent();
			}			
		}

		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_FlowFields::Render(float deltaTime) const
{
	m_pGraphRenderer->RenderGraph(m_pGridGraph, m_DebugSettings.DrawNodes, m_DebugSettings.DrawNodeNumbers,
		m_DebugSettings.DrawConnections, m_DebugSettings.DrawConnectionCosts);

	//Render end node
	if (m_EndNodeIndex != invalid_node_index)
	{
		m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(m_EndNodeIndex) }, END_NODE_COLOR);
	}

	if (m_DebugSettings.DrawCostField)
	{
		Vector2 textPos{ static_cast<float>(m_SizeCell) - 10, static_cast<float>(m_SizeCell) };
		for (int row{}; row < ROWS; ++row)
		{
			for (int col{}; col < COLUMNS; ++col)
			{
				DEBUGRENDERER2D->DrawString(textPos, std::to_string(m_CostField[row * COLUMNS + col]).c_str());
				textPos.x += m_SizeCell;
			}
			textPos.y += m_SizeCell;
			textPos.x = static_cast<float>(m_SizeCell) - 10;
		}
	}

	if (m_DebugSettings.DrawIntergartionField)
	{
		Vector2 textPos{ static_cast<float>(m_SizeCell) - 4, static_cast<float>(m_SizeCell) };
		for (int row{}; row < ROWS; ++row)
		{
			for (int col{}; col < COLUMNS; ++col)
			{
				DEBUGRENDERER2D->DrawString(textPos, std::to_string(m_IntegrationField[row * COLUMNS + col]).c_str());
				textPos.x += m_SizeCell;
			}
			textPos.y += m_SizeCell;
			textPos.x = static_cast<float>(m_SizeCell) - 4;
		}
	}

	if (m_DebugSettings.DrawVectors)
	{
		for (int i{}; i < m_VectorField.size(); ++i)
		{
			DrawVectorField(m_pGridGraph->GetNodeWorldPos(i), m_VectorField[i]);			
		}
		
	}
}

void App_FlowFields::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 1.5f);
}

void App_FlowFields::ResetFields()
{
	m_CostField.resize(ROWS * COLUMNS);
	m_IntegrationField.resize(ROWS * COLUMNS);
	m_VectorField.resize(ROWS * COLUMNS);
	for (auto& costCell : m_CostField)
	{
		costCell = 1;
	}

	for (auto& IntegrationCell : m_IntegrationField)
	{
		IntegrationCell = m_MaxIntegrationCost;
	}

	for (auto& vectorDir : m_VectorField)
	{
		vectorDir = VectorDir::invalid;
	}
}

void App_FlowFields::MakePath()
{
	if (m_EndNodeIndex == invalid_node_index)
	{
		return;
	}
	ResetFields();

	CalculateCostField();
	CalculateIntegrationField();
	CalculateVectorField();		
}

void App_FlowFields::CalculateCostField()
{
	for (const auto& node: m_pGridGraph->GetAllNodes())
	{
		switch (node->GetTerrainType())
		{
		case TerrainType::Mud:
			m_CostField[node->GetIndex()] = 5;
			break;
		case TerrainType::Water:
			m_CostField[node->GetIndex()] = m_MaxCost;
			break;
		default:
			
			break;
		}
	}	
}

void App_FlowFields::CalculateIntegrationField()
{
	std::deque<int> openList;

	//set EndNode cost to 0 
	m_IntegrationField[m_EndNodeIndex] = 0;
	openList.push_back(m_EndNodeIndex);

	
	while (!openList.empty())
	{		
		const int currentNode{ openList.front() };
		openList.pop_front();

		// loop over the connections of the current node 
		for (const auto& connectionsNode : m_pGridGraph->GetNodeConnections(currentNode))
		{
			const int currentNodeNeighbor{ connectionsNode->GetTo() };

			//ignore the Neighbor node if his cost is 255
			if (m_CostField[currentNodeNeighbor] == 255)
			{
				continue;
			}

			//For every neighbor, set their total cost to the sum of the current node's cost and the cost found in the cost field
			const int costNeighbor{ m_IntegrationField[currentNode] + m_CostField[currentNodeNeighbor] };
			
			// Add the neighbor to the back of the open list if the new calculated cost is lower than the old cost
			if (costNeighbor < m_IntegrationField[currentNodeNeighbor])
			{
				//check if the current node's Neighbor is not in the list
				if((std::find(begin(openList), end(openList), currentNodeNeighbor) == openList.end()))
				{
					openList.push_back(currentNodeNeighbor);
				}
				
				m_IntegrationField[currentNodeNeighbor] = costNeighbor;
			}
		}
	}	
}

void App_FlowFields::CalculateVectorField()
{
	
	for (int i{0}; i < m_IntegrationField.size();++i)
	{
		int lowestCostNeighbor{m_MaxIntegrationCost},
			neighborNode{invalid_node_index};
		
		for (const auto& connectionsNode : m_pGridGraph->GetNodeConnections(i))
		{
			if (m_IntegrationField[connectionsNode->GetTo()]< lowestCostNeighbor)
			{
				lowestCostNeighbor = m_IntegrationField[connectionsNode->GetTo()];
				neighborNode = connectionsNode->GetTo();
				
			}
		}

		// Determine the direction of the vector from current node to neighbor node
		if (lowestCostNeighbor != m_MaxIntegrationCost && neighborNode != invalid_node_index)
		{
			if (neighborNode == i + COLUMNS)
			{
				m_VectorField[i] = VectorDir::up;
			}
			else if(neighborNode == i - COLUMNS)
			{
				m_VectorField[i] = VectorDir::down;
			}
			else if (neighborNode == i - 1)
			{
				m_VectorField[i] = VectorDir::left;
			}
			else if (neighborNode == i + 1)
			{
				m_VectorField[i] = VectorDir::right;
			}
			else if (neighborNode == i + COLUMNS + 1)
			{
				m_VectorField[i] = VectorDir::upRight;
			}
			else if (neighborNode == i - COLUMNS + 1)
			{
				m_VectorField[i] = VectorDir::downRight;
			}
			else if (neighborNode == i + COLUMNS - 1)
			{
				m_VectorField[i] = VectorDir::upLeft;
			}
			else if (neighborNode == i - COLUMNS - 1)
			{
				m_VectorField[i] = VectorDir::downLeft;
			}

		}
	}//for loop nodes

	if (m_EndNodeIndex != invalid_node_index)
	{
		m_VectorField[m_EndNodeIndex] = VectorDir::invalid;
	}
}

void App_FlowFields::DrawVectorField(Vector2 cellPos, VectorDir direction) const
{
	Vector2 dir;
	
	switch (direction)
	{
	case App_FlowFields::VectorDir::invalid:		
		dir = Vector2{ 0.f,0.f };
		break;
	case App_FlowFields::VectorDir::up:
		dir = Vector2{ 0.f,1.f };
		break;

	case App_FlowFields::VectorDir::down:
		dir = Vector2{ 0.f,-1.f };
		break;

	case App_FlowFields::VectorDir::left:
		dir = Vector2{ -1.f,0.f };
		break;

	case App_FlowFields::VectorDir::right:
		dir = Vector2{ 1.f,0.f };
		break;

	case App_FlowFields::VectorDir::upRight:
		dir = Vector2{ 1.f,1.f };
		break;

	case App_FlowFields::VectorDir::upLeft:
		dir = Vector2{ -1.f,1.f };
		break;

	case App_FlowFields::VectorDir::downRight:
		dir = Vector2{ 1.f,-1.f };
		break;

	case App_FlowFields::VectorDir::downLeft:
		dir = Vector2{ -1.f,-1.f };
		break;

	default:
		break;
	}
	DEBUGRENDERER2D->DrawDirection(cellPos,dir , (m_SizeCell / 3.f), Color{ 1.f,1.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());
}

void App_FlowFields::AddAgent()
{
	float worldWith{ static_cast<float>(COLUMNS * m_SizeCell) };
	float worldHeight{ static_cast<float>(ROWS * m_SizeCell) };

	SteeringAgent* newAgent = new SteeringAgent();
	
	newAgent->SetMaxLinearSpeed(20.f);
	newAgent->SetAutoOrient(true);

	Vector2 randomStartPos{ static_cast<float>(rand() % static_cast<int>(worldWith - 10)),
		static_cast<float>(rand() % static_cast<int>(worldHeight - 10)) };
	newAgent->SetPosition(randomStartPos);

	m_pAgents.push_back(newAgent);
}