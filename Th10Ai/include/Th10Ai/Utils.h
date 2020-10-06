#pragma once

#include "Th10Ai/Common.h"

#include "Th10Ai/Point.h"
#include "Th10Ai/Size.h"
#include "Th10Ai/Rect.h"

namespace th
{
	struct FootPoint
	{
		float_t k;
		Pointf pos;
	};

	class Utils
	{
	public:
		static float_t CalcDistance(const Pointf& A, const Pointf& B);
		// AB��AC�ļн�
		static float_t CalcAngle(const Pointf& A, const Pointf& B, const Pointf& C);
		// ��C���߶�AB�Ĵ���
		static FootPoint CalcFootPoint(const Pointf& A, const Pointf& B, const Pointf& C);
	};
}