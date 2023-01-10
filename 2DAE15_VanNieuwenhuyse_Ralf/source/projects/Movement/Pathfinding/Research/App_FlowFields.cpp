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
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(60.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2{ worldWith / 2.f, worldHeight / 2.f });
	MakeGridGraph();
	ResetFields();

	m_EndNodeIndex = 20;
}

void App_FlowFields::Update(float deltaTime)
{
	for (const auto& agent: m_pAgents)
	{
		agent->Update(deltaTime);
		
	}

	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeIdxAtWorldPos(mousePos);
				
		m_EndNodeIndex = closestNode;
		std::cout << "End Node has changed\n";
		
	}

	UpdateImGui();	

	if (m_pGraphEditor->UpdateGraph(m_pGridGraph))
	{
		std::cout << "Graph has been edited\n";
	}

}

void App_FlowFields::UpdateImGui()
{
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
}

void App_FlowFields::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 1.5f);
}

void App_FlowFields::ResetFields()
{
	m_CostField.resize(ROWS * COLUMNS);
	m_IntegrationField.resize(ROWS * COLUMNS);
	m_FlowField.resize(ROWS * COLUMNS);
	for (auto& costCell : m_CostField)
	{
		costCell = 1;
	}

	for (auto& IntegrationCell : m_IntegrationField)
	{
		IntegrationCell = 60000;
	}

	for (auto& vectorDir : m_FlowField)
	{
		vectorDir = VectorDir::invalid;
	}
}
