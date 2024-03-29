#pragma once

#include "Th10Base/Common.h"

#include "Th10Base/BB.h"

namespace th
{
	// 矩形轴对齐包围盒
	class AABB :
		public BB
	{
	public:
		// 投影轴，单位向量
		static constexpr vec2 PROJ_AXIS_X = vec2(_F(1.0), _F(0.0));
		static constexpr vec2 PROJ_AXIS_Y = vec2(_F(0.0), _F(1.0));

		Projection projectX() const;
		Projection projectY() const;
		bool collide(const AABB& other) const;
	};
}
