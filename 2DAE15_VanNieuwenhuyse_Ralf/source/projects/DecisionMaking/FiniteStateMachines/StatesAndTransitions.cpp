#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

void FSMStates::WanderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	bool isValid = pBlackboard->GetData("Agent", pAgent);
	std::cout << "wander state\n";
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	pAgent->SetToWander();

}

void FSMStates::SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	bool isValid = pBlackboard->GetData("Agent", pAgent);
	std::cout << "Seek state\n";
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	AgarioFood* nearestFood;
	if (!pBlackboard->GetData("NearestFood", nearestFood) || !nearestFood)
	{
		return;
	}

	//TODO:
	DEBUGRENDERER2D->DrawPoint(nearestFood->GetPosition(), 30.f, Color{ 1.f,1.f,1.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());
	pAgent->SetToSeek(nearestFood->GetPosition());
}

void FSMStates::EvadeBiggerState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	
}

bool FSMConditions::FoodNearByCondition::Evaluate(Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;
	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}
	if (pBlackboard->GetData("FoodVec",pFoodVec) == false || pFoodVec == nullptr)
	{
		return false;
	}

	const float radius{ 10.f };

	Vector2 agentPos = pAgent->GetPosition();

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.f,0.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());

	auto isCloser = [agentPos](AgarioFood* pFood1, AgarioFood* pFood2)
	{
		float dist1 = pFood1->GetPosition().DistanceSquared(agentPos);
		float dist2 = pFood2->GetPosition().DistanceSquared(agentPos);

		return dist1 < dist2;
	};
	
	auto closestElementIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), isCloser);
	
	if (closestElementIt == pFoodVec->end())
	{
		return false;
	}

	AgarioFood* closestFood = *closestElementIt;

	if (closestFood->GetPosition().DistanceSquared(agentPos) < radius * radius)
	{
		std::cout << closestFood->GetPosition() << '\n';
		pBlackboard->ChangeData("NearestFood", closestFood);
		return true;
	}

	return false;
}

bool FSMConditions::EvadeCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioAgent*>* pAgentVec;
	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}
	if (pBlackboard->GetData("FoodVec", pAgentVec) == false || pAgentVec == nullptr)
	{
		return false;
	}
	return false;
}