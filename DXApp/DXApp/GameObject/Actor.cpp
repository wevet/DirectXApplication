#include "Actor.h"

using namespace GameObject;
using namespace DirectX;

Actor::Actor()
{
}


Actor::~Actor()
{
}

bool Actor::Create()
{
	return false;
}

void Actor::Destroy()
{
}

void Actor::Update(uint64_t dt)
{
}

void Actor::SetPosition(XMVECTOR position)
{
	Position = position;
}

void Actor::SetRotation(XMVECTOR rotation)
{
	Rotation = rotation;
}
