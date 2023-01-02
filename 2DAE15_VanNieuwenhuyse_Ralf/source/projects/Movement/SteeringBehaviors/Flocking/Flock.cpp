#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"


using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
{
	m_Agents.resize(m_FlockSize);
	m_AgentsOldPos.resize(m_FlockSize);
	// TODO: initialize the flock and the memory pool
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();
	m_pWanderBehavior->SetWanderOffset(3.f);
	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);	

	m_pBlendedSteering = new BlendedSteering({ { m_pSeekBehavior, 0.5f }, { m_pWanderBehavior,0.5f },
		{ m_pCohesionBehavior, 0.5f },{ m_pSeparationBehavior,0.5f },{ m_pVelMatchBehavior, 0.5f } });

	m_pEvadeBehavior = new Evade();
	m_pEvadeBehavior->SetEvadeRadius(15.f);
	
	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior,m_pBlendedSteering });


	const int rowsAndCols{ 20 };
	m_Cellspace = new CellSpace{ m_WorldSize,m_WorldSize,rowsAndCols ,rowsAndCols,m_FlockSize };

	int worldZize = static_cast<float>(worldSize);

	
	for (size_t i{ 0 }; i < m_Agents.size(); ++i)
	{
		Elite::Vector2 randPos{ float(rand() % worldZize),float(rand() % worldZize) };
		m_Agents[i] = new SteeringAgent();	
		m_Agents[i]->SetPosition(randPos);
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(15.f);
		m_Agents[i]->SetMass(0.f);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetBodyColor({ 1,1,0 });

		m_Cellspace->AddAgent(m_Agents[i]);		
	}
}

Flock::~Flock()
{
	// TODO: clean up any additional data

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);

	m_Cellspace->EmptyCells();

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();


	SAFE_DELETE(m_Cellspace);

}

void Flock::Update(float deltaT)
{
	// TODO: update the flock
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world

	TargetData evadeTarget;
	evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	evadeTarget.Position = m_pAgentToEvade->GetPosition();

	m_pEvadeBehavior->SetTarget(evadeTarget);

	float queryRadius{ 10.f };

	for (size_t i{ 0 }; i < m_Agents.size(); ++i)
	{
		if (!m_IsSpatialPartitioningActive)
		{
			RegisterNeighbors(m_Agents[i]);
		}
		else
		{
			m_Cellspace->UpdateAgentCell(m_Agents[i], m_AgentsOldPos[i]);
			m_Cellspace->RegisterNeighbors(m_Agents[i], queryRadius);
			m_AgentsOldPos[i] = m_Agents[i]->GetPosition();
		}
		
		TargetData evadeTarget;
		evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
		evadeTarget.Position = m_pAgentToEvade->GetPosition();

		m_pEvadeBehavior->SetTarget(evadeTarget);
						
		m_Agents[i]->Update(deltaT);
			
		if (m_TrimWorld)
		{
			m_Agents[i]->TrimToWorld(m_WorldSize);			
		}
	}

	

	if (m_CanDebugRender && m_IsSpatialPartitioningActive)
	{		
		DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), queryRadius, {0.f, 0.f, 1.f}, 0);
		std::vector<Elite::Vector2> points =
		{
			{ m_Agents[0]->GetPosition().x - (queryRadius),m_Agents[0]->GetPosition().y - (queryRadius) },
			{ m_Agents[0]->GetPosition().x - (queryRadius ),m_Agents[0]->GetPosition().y + (queryRadius) },
			{m_Agents[0]->GetPosition().x + (queryRadius ),m_Agents[0]->GetPosition().y + (queryRadius)  },
			{m_Agents[0]->GetPosition().x + (queryRadius),m_Agents[0]->GetPosition().y - (queryRadius )}
		};
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,1,0,1 }, 0.4f);
		m_Cellspace->RenderCells();				
	}

	m_Agents[0]->SetRenderBehavior(m_CanDebugRender);	

	if (m_TrimWorld)
	{
		m_pAgentToEvade->TrimToWorld(m_WorldSize);
	}
}


void Flock::Render(float deltaT)
{
	// TODO: render the flock
	/*for (size_t i{ 0 }; i < m_Agents.size(); ++i)
	{		
		if (m_Agents[i])
		{		
			m_Agents[i]->Render(deltaT);
					
		}				
	}*/

	if (m_TrimWorld)
	{
		std::vector<Elite::Vector2> points =
		{
			{ 0, m_WorldSize },
			{ m_WorldSize, m_WorldSize },
			{ m_WorldSize, 0 },
			{ 0, 0 }
		};
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);
	}
	
	
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
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
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// TODO: Implement checkboxes for debug rendering and weight sliders here
	ImGui::Checkbox("Using Space Partitioning", &m_IsSpatialPartitioningActive);
	ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
	ImGui::Checkbox("Trim World", &m_TrimWorld);
	if (m_TrimWorld)
	{
		ImGui::SliderFloat("Trim Size", &m_WorldSize, 0.f, 500.f, "%1.");
	}
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	// sliders
	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("VelMatch", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");	

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// TODO: Implement
	m_NrOfNeighbors = 0;
	m_Neighbors.clear();

	for (size_t i{0}; i < m_Agents.size(); ++i)
	{
		if (m_Agents[i] != pAgent)
		{
			Vector2 DistanceBetween2Agents{m_Agents[i]->GetPosition() - pAgent->GetPosition()};
			if ((m_NeighborhoodRadius * m_NeighborhoodRadius)< DistanceBetween2Agents.MagnitudeSquared())
			{
				m_Neighbors.push_back(m_Agents[i]);
				++m_NrOfNeighbors;
			}
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	// TODO: Implement
	if (m_NrOfNeighbors)
	{
		Vector2 sumOfAllPos{};
		for (int i = 0; i < m_NrOfNeighbors; i++)
		{
			sumOfAllPos += m_Neighbors[i]->GetPosition();
		}

		Vector2 avragePos{ sumOfAllPos / float(m_NrOfNeighbors) };
		return avragePos;
	}
	else
	{
		return Vector2{};
	}	
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	// TODO: Implement
	
	if (m_NrOfNeighbors)
	{
		Vector2 sumOfAllSpeed{};
		for (int i = 0; i < m_NrOfNeighbors; i++)
		{
			sumOfAllSpeed += m_Neighbors[i]->GetLinearVelocity();
		}

		Vector2 avrageSpeed{ sumOfAllSpeed / float(m_NrOfNeighbors) };

		return avrageSpeed;
	}
	else
	{
		return Vector2{};
	}
}

void Flock::SetTarget_Seek(TargetData target)
{
	// TODO: Set target for seek behavior
	m_pSeekBehavior->SetTarget(target);
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

