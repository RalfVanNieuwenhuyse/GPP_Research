//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

using namespace Elite;
//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}

	return steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 fromTarget = -m_Target.Position + pAgent->GetPosition();
	float distance = fromTarget.Magnitude();

	
	SteeringOutput steering = {};

	if (distance > m_FleeRadius)
	{
		steering.IsValid = false;
		return steering;
	}
		
	steering.LinearVelocity = fromTarget/fromTarget.MagnitudeSquared();
	if (steering.LinearVelocity.MagnitudeSquared() > pAgent->GetLinearVelocity().MagnitudeSquared()||true)
	{
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(),distance,{0,1,0}, DEBUGRENDERER2D->NextDepthSlice());
	}

	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const float arrivalRadius{ 1.f };
	const float slowRadius{ 15.f };	

	Vector2 toTarget = m_Target.Position - pAgent->GetPosition();
	const float distance = toTarget.Magnitude();
	if (distance < arrivalRadius)
	{
		pAgent->SetLinearVelocity({ 0.f,0.f });
		return steering;
	}
	
	auto velocity = toTarget;
	velocity.Normalize();
	if (distance < slowRadius)
	{
		velocity *= pAgent->GetMaxLinearSpeed() * (distance / slowRadius);
	}
	else
	{
		velocity *= pAgent->GetMaxLinearSpeed();
	}

	steering.LinearVelocity = velocity;

	return steering;

	

}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(false);
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}	

	Vector2 agentFrontVector{ 5 * cosf(pAgent->GetRotation()),5 * sinf(pAgent->GetRotation()) };
	Vector2 toTarget = m_Target.Position - pAgent->GetPosition();
		
	float cosalpha = (agentFrontVector.Dot(toTarget)) / (agentFrontVector.Magnitude() * toTarget.Magnitude());

	std::cout << cosalpha << '\n';

	if (cosalpha >= 0.9999f)
	{
		steering.AngularVelocity = 0;
	}
	else
	{
		steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
	}

	return steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 circleOrigin{};
	circleOrigin.x = pAgent->GetPosition().x + cosf(pAgent->GetRotation()) * m_OffsetDistance;
	circleOrigin.y = pAgent->GetPosition().y + sinf(pAgent->GetRotation()) * m_OffsetDistance;

	const float maxWanderAngle{ m_WanderAngle + m_MaxAngleChange + 360 };
	const float minWanderAngle{ m_WanderAngle - m_MaxAngleChange + 360 };

	m_WanderAngle = float(rand() % int(((maxWanderAngle - minWanderAngle + 1) + minWanderAngle) * 100)) / 100;
	
	while (m_WanderAngle > 360 || m_WanderAngle < -360)
	{
		if (m_WanderAngle > 360)
		{
			m_WanderAngle -= 360;
		}
		else
		{
			m_WanderAngle += 360;
		}
	}

	Vector2 randomPointOnCircle{ circleOrigin.x + cosf(ToRadians(m_WanderAngle)) * m_Radius, circleOrigin.y + sinf(ToRadians(m_WanderAngle)) * m_Radius };
	
	m_Target.Position = randomPointOnCircle;

	SteeringOutput    steering{};
	steering = Seek::CalculateSteering(deltaT, pAgent);

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(circleOrigin, m_Radius, { 0.f, 0.f, 1.f }, 0);

		DEBUGRENDERER2D->DrawPoint(randomPointOnCircle, 5.f, { 0.f, 1.f, 1.f });
	}

	return steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 predictPos{ m_Target.Position + (m_Target.LinearVelocity) };
	m_Target.Position = predictPos;

	Vector2 fromTarget = m_Target.Position - pAgent->GetPosition();
	float distance = fromTarget.Magnitude();


	SteeringOutput steering = {};

	if (distance > m_EvadeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	steering.LinearVelocity = -m_Target.Position + pAgent->GetPosition();
	
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed()*2;
	

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}

	return steering;
}