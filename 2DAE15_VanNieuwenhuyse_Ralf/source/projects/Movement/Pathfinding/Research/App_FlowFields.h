#pragma once
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include "framework\EliteAI\EliteNavigation\ENavigation.h"

class SteeringAgent;
class NavigationColliderElement;

class App_FlowFields final : public IApp
{
public:
	//Constructor & Destructor
	App_FlowFields() = default;
	virtual ~App_FlowFields();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	struct DebugSettings
	{
		bool DrawNodes{ true };
		bool DrawNodeNumbers{ false };
		bool DrawConnections{ false };
		bool DrawConnectionCosts{ false };
	};

	const bool ALLOW_DIAGONAL_MOVEMENT = true;	

	int m_EndNodeIndex{ invalid_node_index };

	//Grid datamembers
	static const int COLUMNS = 20;
	static const int ROWS = 10;
	unsigned int m_SizeCell = 15;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;

	Elite::GraphEditor* m_pGraphEditor{ nullptr };
	Elite::GraphRenderer* m_pGraphRenderer{ nullptr };

	DebugSettings m_DebugSettings{};

	void MakeGridGraph();
	void UpdateImGui();
	
	int m_NrOfAgents = 50;
	std::vector<SteeringAgent*> m_pAgents;
};

