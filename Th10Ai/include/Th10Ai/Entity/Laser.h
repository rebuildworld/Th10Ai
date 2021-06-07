#pragma once

#include "Th10Ai/Common.h"

#include "Th10Ai/Entity/OBB.h"

namespace th
{
	class Entity;

	// ����ṹ
	//  (x,y)
	// +--*--+
	// |     |
	// |     |
	// |     |
	// |     | h
	// |     |
	// |     |
	// |     |
	// +-----+
	//    w
	class Laser :
		public OBB
	{
	public:
		Laser();
		Laser(const vec2& pos0, const vec2& delta0, const vec2& size0, float_t arc0);

		void updateExtra();
		bool willCollideWith(const AABB& other) const;

		float_t distance(const Entity& entity) const;
		void advance(int_t frame);

	private:
		vec2 getTopLeft() const;
		vec2 getTopRight() const;
		vec2 getBottomLeft() const;
		vec2 getBottomRight() const;
		float_t getRadian() const;
		void updateOBB();

	public:
		vec2 pos;
		vec2 delta;
		vec2 size;
		float_t arc;

		vec2 deltaV;
		Projection projV;
	};
}
