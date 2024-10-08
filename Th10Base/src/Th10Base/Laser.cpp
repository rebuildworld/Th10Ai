#include "Th10Base/Laser.h"

#include <cmath>
#include <numbers>

#include "Th10Base/Entity.h"

namespace th
{
	Laser::Laser() :
		arc()
	{
	}

	Laser::Laser(const LaserRaw* raw)
	{
		pos.x = raw->x;
		pos.y = raw->y;
		delta.x = raw->dx;
		delta.y = raw->dy;
		size.x = raw->width;
		size.y = raw->height;
		arc = raw->arc;
	}

	void Laser::updateExtra()
	{
		updateOBB();

		if (!delta.isZero())
		{
			deltaV = delta.verticalize().normalize();
			projV = projectWithNormalized(deltaV);
		}
	}

	bool Laser::willCollideWith(const AABB& other) const
	{
		if (!delta.isZero())
		{
			//vec2 deltaV = delta.verticalize().normalize();
			//return projectWithNormalized(deltaV).overlap(other.projectWithNormalized(deltaV));
			return projV.overlap(other.projectWithNormalized(deltaV));
		}
		else
		{
			return collide(other);
		}
	}

	vec2::value_t Laser::distance(const Entity& entity) const
	{
		return (pos - entity.pos).length();
	}

	void Laser::setPosition(const vec2& pos0)
	{
		pos = pos0;
		updateOBB();
	}

	void Laser::advance(int_t frame)
	{
		vec2 offset = delta * static_cast<float_t>(frame);
		pos += offset;
		translate(offset);
	}

	vec2 Laser::getTopLeft() const
	{
		return vec2(pos.x - size.x / _F(2.0), pos.y);
	}

	vec2 Laser::getTopRight() const
	{
		return vec2(pos.x + size.x / _F(2.0), pos.y);
	}

	vec2 Laser::getBottomLeft() const
	{
		return vec2(pos.x - size.x / _F(2.0), pos.y + size.y);
	}

	vec2 Laser::getBottomRight() const
	{
		return vec2(pos.x + size.x / _F(2.0), pos.y + size.y);
	}

	float_t Laser::getRadian() const
	{
		// emmm...你说这个谁懂啊？
		float_t radian = arc - static_cast<float_t>(std::numbers::pi) * _F(5.0) / _F(2.0);
		return radian;
	}

	void Laser::updateOBB()
	{
		float_t radian = getRadian();
		float_t sinVal = std::sin(radian);
		float_t cosVal = std::cos(radian);
		topLeft = pos + (getTopLeft() - pos).rotate(sinVal, cosVal);
		topRight = pos + (getTopRight() - pos).rotate(sinVal, cosVal);
		bottomLeft = pos + (getBottomLeft() - pos).rotate(sinVal, cosVal);
		bottomRight = pos + (getBottomRight() - pos).rotate(sinVal, cosVal);
		projAxisX = vec2(_F(1.0), _F(0.0)).rotate(sinVal, cosVal);
		projAxisY = vec2(_F(0.0), _F(1.0)).rotate(sinVal, cosVal);
	}
}
