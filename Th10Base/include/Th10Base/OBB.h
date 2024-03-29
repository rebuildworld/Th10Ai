#pragma once

#include "Th10Base/Common.h"

#include "Th10Base/BB.h"

namespace th
{
	class AABB;

	// 矩形定向包围盒
	class OBB :
		public BB
	{
	public:
		bool collide(const AABB& other) const;

	public:
		vec2 projAxisX;		// 投影轴，单位向量
		vec2 projAxisY;
	};
}
