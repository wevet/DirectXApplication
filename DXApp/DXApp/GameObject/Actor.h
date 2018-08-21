#pragma once

#include <cassert>
#include <map>
#include <DirectXMath.h>

class Component;

namespace GameObject
{
	class Actor
	{
	public:
		Actor();
		virtual ~Actor();

		bool Create();
		void Destroy();

		void Update(uint64_t dt);
		void SetId(uint16_t id) 
		{
			Id = id;
		}
		uint64_t GetId() const { return Id; }

		void SetPosition(DirectX::XMVECTOR position);
		void SetRotation(DirectX::XMVECTOR rotation);
		
	protected:
		uint64_t Id;
		DirectX::XMVECTOR Position;
		DirectX::XMVECTOR Rotation;
	};

}

