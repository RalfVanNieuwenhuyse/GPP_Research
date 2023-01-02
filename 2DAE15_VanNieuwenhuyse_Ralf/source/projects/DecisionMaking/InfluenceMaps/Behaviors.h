/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		
		if (!pBlackboard->GetData("Agent",pAgent)||pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		pAgent->SetToWander();
		//std::cout << "set to wander\n";
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekFood(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		Elite::Vector2 targetpos;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Target", targetpos) )
		{
			return Elite::BehaviorState::Failure;
		}
		//std::cout << "set to seek\n";
		pAgent->SetToSeek(targetpos);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToEvadeBiggerAgents(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		
		AgarioAgent* bigBoy;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("AgentFleeTarget", bigBoy))
		{
			return Elite::BehaviorState::Failure;
		}

		pAgent->SetToFlee(bigBoy->GetPosition());

		//std::cout << bigBoy->GetPosition() << '\n';

		//std::cout << "Velagent " << pAgent->GetLinearVelocity() << "\n";

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToPursueSmallerAgents(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		Elite::Vector2 targetpos;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		if (!pBlackboard->GetData("Target", targetpos))
		{
			return Elite::BehaviorState::Failure;
		}
		//std::cout << "set to small\n";
		pAgent->SetToSeek(targetpos);

		return Elite::BehaviorState::Success;
	}
	
}

//-----------------------------------------------------------------
// Behaviors
//----------------------------------------------------------------

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		std::vector<AgarioFood*>* pFoodVec;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}

		if (pBlackboard->GetData("FoodVec",pFoodVec) == false|| pFoodVec == nullptr)
		{
			return false;
		}

		if (pFoodVec->empty())
		{
			return false;
		}

		const float searchRadius{ pAgent->GetRadius() + 20.f };

		float closestDistSq{searchRadius*searchRadius};
		AgarioFood* pClosestFood{ nullptr };

		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pFood : *pFoodVec)
		{
			float distSq = pFood->GetPosition().DistanceSquared(agentPos);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;
				pClosestFood = pFood;
			}
		}

		if (pClosestFood !=nullptr)
		{
			pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
			return true;
		}

		return false;
	}

	bool IsBiggerEnemyNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		std::vector<AgarioAgent*>* pAgentVec;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}

		if (pBlackboard->GetData("AgentsVec", pAgentVec) == false || pAgentVec == nullptr)
		{
			return false;
		}

		if (pAgentVec->empty())
		{
			return false;
		}

		const float fleeRadius{ pAgent->GetRadius() + 20.f };
				
		float closestDistSq{ fleeRadius * fleeRadius };
		AgarioAgent* pClosestEnemy{ nullptr };

		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pEnemy : *pAgentVec)
		{
			float distSq = pEnemy->GetPosition().DistanceSquared(agentPos);

			if (distSq < closestDistSq)
			{
				if (pEnemy->GetRadius() >= pAgent->GetRadius())
				{
					closestDistSq = distSq;
					pClosestEnemy = pEnemy;
				}				
			}
		}

		if (pClosestEnemy != nullptr)
		{
			pBlackboard->ChangeData("AgentFleeTarget", pClosestEnemy);
			return true;
		}

		return false;
	}

	bool IsSmallerEnemyNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		std::vector<AgarioAgent*>* pAgentVec;

		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}

		if (pBlackboard->GetData("AgentsVec", pAgentVec) == false || pAgentVec == nullptr)
		{
			return false;
		}

		if (pAgentVec->empty())
		{
			return false;
		}

		const float SeekRadius{ pAgent->GetRadius() + 20.f };

		float closestDistSq{ SeekRadius * SeekRadius };
		AgarioAgent* pClosestEnemy{ nullptr };

		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		for (auto& pEnemy : *pAgentVec)
		{
			float distSq = pEnemy->GetPosition().DistanceSquared(agentPos);

			if (distSq < closestDistSq)
			{
				if (pEnemy->GetRadius() < pAgent->GetRadius())
				{
					closestDistSq = distSq;
					pClosestEnemy = pEnemy;
				}
			}
		}

		if (pClosestEnemy != nullptr)
		{
			pBlackboard->ChangeData("Target", pClosestEnemy->GetPosition());
			return true;
		}

		return false;
	}
}



#endif