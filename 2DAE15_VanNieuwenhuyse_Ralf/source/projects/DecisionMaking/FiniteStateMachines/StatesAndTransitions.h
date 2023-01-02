/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------
namespace FSMStates
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() :FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard);	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() :FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard);		
	};

	class EvadeBiggerState : public Elite::FSMState
	{
	public:
		EvadeBiggerState() :FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard);
	};
}


//-----------------
//---TRANSITIONS---
//-----------------

namespace FSMConditions
{
	class FoodNearByCondition : public Elite::FSMCondition
	{
	public:
		FoodNearByCondition() : FSMCondition() {}
		
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const;
	};

	class  EvadeCondition : public Elite::FSMCondition
	{
	public:
		EvadeCondition() : FSMCondition() {}

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const;
	};

}

#endif