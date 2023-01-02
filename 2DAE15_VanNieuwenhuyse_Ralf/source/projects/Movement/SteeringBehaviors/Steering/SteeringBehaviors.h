/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "../SteeringHelpers.h"

class SteeringAgent;
class Obstacle;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

///////////////////////////////////////
//Flee
//****
class Flee : public ISteeringBehavior
{
public:
	Flee()=default;
	virtual ~Flee() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	void SetFleeRadius(float radius) { m_FleeRadius = radius; }


private:

	float m_FleeRadius = 10.f;
};

///////////////////////////////////////
//Arrive
//****
class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

};

///////////////////////////////////////
//Face
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

};

///////////////////////////////////////
//Wander
//****
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetWanderAngleChange(float rad) { m_MaxAngleChange = rad; }

protected:
	float m_OffsetDistance{ 2.f };
	float m_Radius{ 6.f };
	float m_MaxAngleChange = Elite::ToRadians(60);
	float m_WanderAngle = 0;

};

///////////////////////////////////////
//Evade
//****
class Evade : public ISteeringBehavior
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	void SetEvadeRadius(float radius) {m_EvadeRadius = radius;}
	
private:

	float m_EvadeRadius;

};


#endif


