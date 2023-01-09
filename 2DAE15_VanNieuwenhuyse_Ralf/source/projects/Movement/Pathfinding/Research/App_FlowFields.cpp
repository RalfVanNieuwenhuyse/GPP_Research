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
	m_pGraphEditor = new GraphEditor();
	m_pGraphRenderer = new GraphRenderer();
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(90.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(150.f, 75.f));
	MakeGridGraph();
	ResetFields();
}

void App_FlowFields::Update(float deltaTime)
{
	UpdateImGui();
	for (auto& costCell : m_FlowField)
	{
		std::cout << static_cast<int>(costCell) << " ";
	}
	std::cout << "\n\n";
}

void App_FlowFields::UpdateImGui()
{
}

void App_FlowFields::Render(float deltaTime) const
{
	m_pGraphRenderer->RenderGraph(m_pGridGraph, m_DebugSettings.DrawNodes, m_DebugSettings.DrawNodeNumbers,
		m_DebugSettings.DrawConnections, m_DebugSettings.DrawConnectionCosts);
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
