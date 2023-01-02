#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	m_Target.Position = m_pFlock->GetAverageNeighborPos();

	steering = Seek::CalculateSteering(deltaT, pAgent);

	return steering;	
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	m_Target.Position = m_pFlock->GetAverageNeighborPos();

	steering = Flee::CalculateSteering(deltaT, pAgent);

	

	return steering;
	
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();

	return steering;
}