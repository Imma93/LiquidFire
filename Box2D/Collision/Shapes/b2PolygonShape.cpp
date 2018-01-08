/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
* Copyright (c) 2013 Google, Inc.
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <new>

b2Shape* b2PolygonShape::Clone(b2BlockAllocator* allocator) const
{
	void* mem = allocator->Allocate(sizeof(b2PolygonShape));
	b2PolygonShape* clone = new (mem) b2PolygonShape;
	*clone = *this;
	return clone;
}

void b2PolygonShape::SetAsBox(float32 hx, float32 hy)
{
	m_count = 4;
	m_vertices[0].Set(-hx, -hy);
	m_vertices[1].Set( hx, -hy);
	m_vertices[2].Set( hx,  hy);
	m_vertices[3].Set(-hx,  hy);
	m_normals[0].Set(0.0f, -1.0f);
	m_normals[1].Set(1.0f, 0.0f);
	m_normals[2].Set(0.0f, 1.0f);
	m_normals[3].Set(-1.0f, 0.0f);
	m_normalsX[0] = 0.0f;
	m_normalsX[1] = 1.0f;
	m_normalsX[2] = 0.0f;
	m_normalsX[3] = -1.0f;
	m_normalsY[0] = -1.0f;
	m_normalsY[1] = 0.0f;
	m_normalsY[2] = 1.0f;
	m_normalsY[3] = 0.0f;
	m_centroid.SetZero();
}

void b2PolygonShape::SetAsBox(float32 hx, float32 hy, const b2Vec2& center, float32 angle)
{
	m_count = 4;
	m_vertices[0].Set(-hx, -hy);
	m_vertices[1].Set( hx, -hy);
	m_vertices[2].Set( hx,  hy);
	m_vertices[3].Set(-hx,  hy);
	m_normals[0].Set(0.0f, -1.0f);
	m_normals[1].Set(1.0f, 0.0f);
	m_normals[2].Set(0.0f, 1.0f);
	m_normals[3].Set(-1.0f, 0.0f);
	m_normalsX[0] = 0.0f;
	m_normalsX[1] = 1.0f;
	m_normalsX[2] = 0.0f;
	m_normalsX[3] = -1.0f;
	m_normalsY[0] = -1.0f;
	m_normalsY[1] = 0.0f;
	m_normalsY[2] = 1.0f;
	m_normalsY[3] = 0.0f;
	m_centroid = center;

	b2Transform xf;
	xf.p = center;
	xf.q.Set(angle);

	// Transform vertices and normals.
	for (int32 i = 0; i < m_count; ++i)
	{
		m_vertices[i] = b2Mul(xf, m_vertices[i]);
		m_normals[i] = b2Mul(xf.q, m_normals[i]);
		m_normalsX[i] = m_normals[i].x;
		m_normalsY[i] = m_normals[i].y;
	}
}

int32 b2PolygonShape::GetChildCount() const
{
	return 1;
}

static b2Vec2 ComputeCentroid(const b2Vec2* vs, int32 count)
{
	b2Assert(count >= 3);

	b2Vec2 c; c.Set(0.0f, 0.0f);
	float32 area = 0.0f;

	// pRef is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
	b2Vec2 pRef(0.0f, 0.0f);
#if 0
	// This code would put the reference point inside the polygon.
	for (int32 i = 0; i < count; ++i)
	{
		pRef += vs[i];
	}
	pRef *= 1.0f / count;
#endif

	const float32 inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < count; ++i)
	{
		// Triangle vertices.
		b2Vec2 p1 = pRef;
		b2Vec2 p2 = vs[i];
		b2Vec2 p3 = i + 1 < count ? vs[i+1] : vs[0];

		b2Vec2 e1 = p2 - p1;
		b2Vec2 e2 = p3 - p1;

		float32 D = b2Cross(e1, e2);

		float32 triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid
		c += triangleArea * inv3 * (p1 + p2 + p3);
	}

	// Centroid
	b2Assert(area > b2_epsilon);
	c *= 1.0f / area;
	return c;
}

void b2PolygonShape::Set(const b2Vec2* vertices, int32 count)
{
	b2Assert(3 <= count && count <= b2_maxPolygonVertices);
	if (count < 3)
	{
		SetAsBox(1.0f, 1.0f);
		return;
	}
	
	int32 n = b2Min(count, b2_maxPolygonVertices);

	// Perform welding and copy vertices into local buffer.
	b2Vec2 ps[b2_maxPolygonVertices];
	int32 tempCount = 0;
	for (int32 i = 0; i < n; ++i)
	{
		b2Vec2 v = vertices[i];

		bool unique = true;
		for (int32 j = 0; j < tempCount; ++j)
		{
			if (b2DistanceSquared(v, ps[j]) < 0.5f * b2_linearSlop)
			{
				unique = false;
				break;
			}
		}

		if (unique)
		{
			ps[tempCount++] = v;
		}
	}

	n = tempCount;
	if (n < 3)
	{
		// Polygon is degenerate.
		b2Assert(false);
		SetAsBox(1.0f, 1.0f);
		return;
	}

	// Create the convex hull using the Gift wrapping algorithm
	// http://en.wikipedia.org/wiki/Gift_wrapping_algorithm

	// Find the right most point on the hull
	int32 i0 = 0;
	float32 x0 = ps[0].x;
	for (int32 i = 1; i < n; ++i)
	{
		float32 x = ps[i].x;
		if (x > x0 || (x == x0 && ps[i].y < ps[i0].y))
		{
			i0 = i;
			x0 = x;
		}
	}

	int32 hull[b2_maxPolygonVertices];
	int32 m = 0;
	int32 ih = i0;

	for (;;)
	{
		hull[m] = ih;

		int32 ie = 0;
		for (int32 j = 1; j < n; ++j)
		{
			if (ie == ih)
			{
				ie = j;
				continue;
			}

			b2Vec2 r = ps[ie] - ps[hull[m]];
			b2Vec2 v = ps[j] - ps[hull[m]];
			float32 c = b2Cross(r, v);
			if (c < 0.0f)
			{
				ie = j;
			}

			// Collinearity check
			if (c == 0.0f && v.LengthSquared() > r.LengthSquared())
			{
				ie = j;
			}
		}

		++m;
		ih = ie;

		if (ie == i0)
		{
			break;
		}
	}
	
	m_count = m;

	// Copy vertices.
	for (int32 i = 0; i < m; ++i)
	{
		m_vertices[i] = ps[hull[i]];
	}

	// Compute normals. Ensure the edges have non-zero length.
	for (int32 i = 0; i < m; ++i)
	{
		int32 i1 = i;
		int32 i2 = i + 1 < m ? i + 1 : 0;
		b2Vec2 edge = m_vertices[i2] - m_vertices[i1];
		b2Assert(edge.LengthSquared() > b2_epsilon * b2_epsilon);
		m_normals[i] = b2Cross(edge, 1.0f);
		m_normals[i].Normalize();
		m_normalsX[i] = m_normals[i].x;
		m_normalsY[i] = m_normals[i].y;
	}

	// Compute the polygon centroid.
	m_centroid = ComputeCentroid(m_vertices, m);
}

bool b2PolygonShape::TestPoint(const b2Transform& xf, const b2Vec2& p) const
{
	b2Vec2 pLocal = b2MulT(xf.q, p - xf.p);

	for (int32 i = 0; i < m_count; ++i)
	{
		float32 dot = b2Dot(m_normals[i], pLocal - m_vertices[i]);
		if (dot > 0.0f)
		{
			return false;
		}
	}

	return true;
}
af::array b2PolygonShape::AFTestPoints(const b2Transform& xf, const af::array& px, const af::array& py) const
{
	af::array pLocalX = b2MulTX(xf.q, px - xf.p.x, py - xf.p.y);
	af::array pLocalY = b2MulTY(xf.q, px - xf.p.x, py - xf.p.y);

	af::array ret = af::constant(1, px.elements());
	for (int32 i = 0; i < m_count; ++i)
	{
		const b2Vec2& n = m_normals[i];
		const b2Vec2& v = m_vertices[i];
		af::array dot = b2Dot(n.x, n.y, pLocalX - v.x, pLocalY - v.y);
		ret(af::where(dot > 0.0f)) = false;
	}
	return ret;
}

void b2PolygonShape::ComputeDistance(const b2Transform& xf, const b2Vec2& p, float32* distance, b2Vec2* normal, int32 childIndex) const
{
	B2_NOT_USED(childIndex);

	b2Vec2 pLocal = b2MulT(xf.q, p - xf.p);
	float32 maxDistance = -FLT_MAX;
	b2Vec2 normalForMaxDistance = pLocal;

	for (int32 i = 0; i < m_count; ++i)
	{
		float32 dot = b2Dot(m_normals[i], pLocal - m_vertices[i]);
		if (dot > maxDistance)
		{
			maxDistance = dot;
			normalForMaxDistance = m_normals[i];
		}
	}

	if (maxDistance > 0)
	{
		b2Vec2 minDistance = normalForMaxDistance;
		float32 minDistance2 = maxDistance * maxDistance;
		for (int32 i = 0; i < m_count; ++i)
		{
			b2Vec2 distance = pLocal - m_vertices[i];
			float32 distance2 = distance.LengthSquared();
			if (minDistance2 > distance2)
			{
				minDistance = distance;
				minDistance2 = distance2;
			}
		}

		*distance = b2Sqrt(minDistance2);
		*normal = b2Mul(xf.q, minDistance);
		normal->Normalize();
	}
	else
	{
		*distance = maxDistance;
		*normal = b2Mul(xf.q, normalForMaxDistance);
	}
}
void b2PolygonShape::AFComputeDistance(const b2Transform& xf, const af::array& px, const af::array& py, af::array& distance, af::array& normalX, af::array& normalY, int32 childIndex) const
{
	B2_NOT_USED(childIndex);
	
	af::array pLocalX = b2MulTX(xf.q, px - xf.p.x, py - xf.p.y);
	af::array pLocalY = b2MulTY(xf.q, px - xf.p.x, py - xf.p.y);
	af::array maxDistance = af::constant(-FLT_MAX, px.elements());
	af::array normalForMaxDistanceX = pLocalX;
	af::array normalForMaxDistanceY = pLocalY;

	for (int32 i = 0; i < m_count; ++i)
	{

		af::array dot = b2Dot(m_normals[i].x, m_normals[i].y, pLocalX - m_vertices[i].x, pLocalY - m_vertices[i].y);
		af::array condIdxs = af::where(dot > maxDistance);
		if (!condIdxs.isempty())
		{
			maxDistance(condIdxs) = dot(condIdxs);
			normalForMaxDistanceX(condIdxs) = m_normals[i].x;
			normalForMaxDistanceY(condIdxs) = m_normals[i].y;
		}
	}
	af::array cond = maxDistance > 0;
	af::array condIdxs = af::where(cond);
	if (!condIdxs.isempty())
	{
		pLocalX = pLocalY(condIdxs);
		pLocalY = pLocalX(condIdxs);
		af::array minDistanceX = normalForMaxDistanceX(condIdxs);
		af::array minDistanceY = normalForMaxDistanceY(condIdxs);
		af::array minDistance2 = maxDistance * maxDistance;
		for (int32 i = 0; i < m_count; ++i)
		{
			af::array distanceX = pLocalX - m_vertices[i].x;
			af::array distanceY = pLocalY - m_vertices[i].y;
			af::array distance2 = distanceX * distanceX + distanceY * distanceY;
			af::array cond2Idxs = af::where(minDistance2 > distance2);
			if (!cond2Idxs.isempty())
			{
				af::array idxs = condIdxs(cond2Idxs);
				minDistanceX(cond2Idxs) = distance(idxs);
				minDistanceY(cond2Idxs) = distance(idxs);
				minDistance2(cond2Idxs) = distance2(cond2Idxs);
			}
		}

		distance(condIdxs) = af::sqrt(minDistance2);
		normalX(condIdxs) = b2MulX(xf.q, minDistanceX, minDistanceY);
		normalY(condIdxs) = b2MulY(xf.q, minDistanceX, minDistanceY);

		//Normalize
		af::array nx = normalX(condIdxs);
		af::array ny = normalY(condIdxs);
		af::array length = af::sqrt(nx * nx + ny * ny);
		af::array toSmall = length < b2_epsilon;
		af::array toSmallIdxs = af::where(toSmall);
		if (!toSmallIdxs.isempty())
			length(toSmallIdxs) = 0.0f;
		af::array cond2Idxs = af::where(!toSmall);
		if (!cond2Idxs.isempty())
		{
			condIdxs = condIdxs(cond2Idxs);
			af::array invLength = 1.0f / length(cond2Idxs);
			normalX(condIdxs) *= invLength;
			normalY(condIdxs) *= invLength;
		}
	}
	af::array elseIdxs = af::where(!cond);
	if (!elseIdxs.isempty())
	{
		normalForMaxDistanceX = normalForMaxDistanceX(elseIdxs);
		normalForMaxDistanceY = normalForMaxDistanceY(elseIdxs);
		distance(elseIdxs) = maxDistance(elseIdxs);
		normalX(elseIdxs) = b2MulX(xf.q, normalForMaxDistanceX, normalForMaxDistanceY);
		normalY(elseIdxs) = b2MulY(xf.q, normalForMaxDistanceX, normalForMaxDistanceY);
	}
}

bool b2PolygonShape::RayCast(b2RayCastOutput* output, const b2RayCastInput& input,
								const b2Transform& xf, int32 childIndex) const
{
	B2_NOT_USED(childIndex);

	// Put the ray into the polygon's frame of reference.
	b2Vec2 p1 = b2MulT(xf.q, input.p1 - xf.p);
	b2Vec2 p2 = b2MulT(xf.q, input.p2 - xf.p);
	b2Vec2 d = p2 - p1;

	float32 lower = 0.0f, upper = input.maxFraction;

	int32 index = -1;

	for (int32 i = 0; i < m_count; ++i)
	{
		// p = p1 + a * d
		// dot(normal, p - v) = 0
		// dot(normal, p1 - v) + a * dot(normal, d) = 0
		float32 numerator = b2Dot(m_normals[i], m_vertices[i] - p1);
		float32 denominator = b2Dot(m_normals[i], d);

		if (denominator == 0.0f)
		{	
			if (numerator < 0.0f)
			{
				return false;
			}
		}
		else
		{
			// Note: we want this predicate without division:
			// lower < numerator / denominator, where denominator < 0
			// Since denominator < 0, we have to flip the inequality:
			// lower < numerator / denominator <==> denominator * lower > numerator.
			if (denominator < 0.0f && numerator < lower * denominator)
			{
				// Increase lower.
				// The segment enters this half-space.
				lower = numerator / denominator;
				index = i;
			}
			else if (denominator > 0.0f && numerator < upper * denominator)
			{
				// Decrease upper.
				// The segment exits this half-space.
				upper = numerator / denominator;
			}
		}

		// The use of epsilon here causes the assert on lower to trip
		// in some cases. Apparently the use of epsilon was to make edge
		// shapes work, but now those are handled separately.
		//if (upper < lower - b2_epsilon)
		if (upper < lower)
		{
			return false;
		}
	}

	b2Assert(0.0f <= lower && lower <= input.maxFraction);

	if (index >= 0)
	{
		output->fraction = lower;
		output->normal = b2Mul(xf.q, m_normals[index]);
		return true;
	}

	return false;
}
af::array b2PolygonShape::AFRayCast(afRayCastOutput* output, const afRayCastInput& input,
	const b2Transform& xf, int32 childIndex) const
{
	B2_NOT_USED(childIndex);
	
	// Put the ray into the polygon's frame of reference.
	const af::array& p1x = b2MulTX(xf.q, input.p1x - xf.p.x, input.p1y - xf.p.y);
	const af::array& p1y = b2MulTY(xf.q, input.p1x - xf.p.x, input.p1y - xf.p.y);
	const af::array& p2x = b2MulTX(xf.q, input.p1x - xf.p.x, input.p2y - xf.p.y);
	const af::array& p2y = b2MulTY(xf.q, input.p1x - xf.p.x, input.p2y - xf.p.y);
	const af::array& dx = p2x - p1x;
	const af::array& dy = p2y - p1y;

	int32 count = p1x.elements();
	af::array lower = af::constant(0.0f, count),
		      upper = af::constant(input.maxFraction, count);

	af::array index = af::constant(-1, count, af::dtype::s32);

	af::array ret = af::constant(true, count, af::dtype::b8);

	for (int32 i = 0; i < m_count; ++i)
	{
		// p = p1 + a * d
		// dot(normal, p - v) = 0
		// dot(normal, p1 - v) + a * dot(normal, d) = 0
		const af::array& numerator = b2Dot(m_normals[i].x, m_normals[i].y, m_vertices[i].x - p1x, m_vertices[i].y - p1y);
		const af::array& denominator = b2Dot(m_normals[i].x, m_normals[i].y, dx, dy);

		const af::array& cond = denominator == 0.0f;
		const af::array& k = af::where(cond);
		const af::array& nk = af::where(!cond);
		if (!k.isempty())
		{
			ret((af::array)k(af::where(numerator < 0.0f))) = false;
		}
		if (!nk.isempty())
		{
			// Note: we want this predicate without division:
			// lower < numerator / denominator, where denominator < 0
			// Since denominator < 0, we have to flip the inequality:
			// lower < numerator / denominator <==> denominator * lower > numerator.
			const af::array& cond = denominator < 0.0f && numerator < lower * denominator;
			const af::array& k2 = af::where(cond);
			const af::array& k2else = af::where(!cond && denominator > 0.0f && numerator < upper * denominator);
			if (!k2.isempty())
			{
				// Increase lower.
				// The segment enters this half-space.
				const af::array& k = nk(k2);
				lower(k) = numerator(k) / denominator(k);
				index(k) = i;
			}
			if (!k2else.isempty())
			{
				// Decrease upper.
				// The segment exits this half-space.
				const af::array& k = nk(k2else);
				upper(k) = numerator(k) / denominator(k);
			}
		}

		// The use of epsilon here causes the assert on lower to trip
		// in some cases. Apparently the use of epsilon was to make edge
		// shapes work, but now those are handled separately.
		//if (upper < lower - b2_epsilon)
		ret(af::where(upper < lower)) = false;
	}

	//b2Assert(0.0f <= lower && lower <= input.maxFraction);

	const af::array validIndex = index >= 0;
	ret(af::where(!validIndex)) = false;
	
	const af::array k = af::where(validIndex);
	if (!k.isempty())
	{
		output->fraction(k) = lower(k);
		index = index(k);
		af::array normalsX(m_count, m_normalsX, af::source::afHost);
		af::array normalsY(m_count, m_normalsY, af::source::afHost);
		normalsX = normalsX(index);
		normalsY = normalsY(index);
		output->normalX(k) = b2MulX(xf.q, normalsX, normalsY);
		output->normalY(k) = b2MulY(xf.q, normalsX, normalsY);
	}
	
	return ret;
}

void b2PolygonShape::ComputeAABB(b2AABB* aabb, const b2Transform& xf, int32 childIndex) const
{
	B2_NOT_USED(childIndex);

	b2Vec2 lower = b2Mul(xf, m_vertices[0]);
	b2Vec2 upper = lower;

	for (int32 i = 1; i < m_count; ++i)
	{
		b2Vec2 v = b2Mul(xf, m_vertices[i]);
		lower = b2Min(lower, v);
		upper = b2Max(upper, v);
	}

	b2Vec2 r(m_radius, m_radius);
	aabb->lowerBound = lower - r;
	aabb->upperBound = upper + r;
}

void b2PolygonShape::ComputeMass(b2MassData* massData, float32 density) const
{
	// Polygon mass, centroid, and inertia.
	// Let rho be the polygon density in mass per unit area.
	// Then:
	// mass = rho * int(dA)
	// centroid.x = (1/mass) * rho * int(x * dA)
	// centroid.y = (1/mass) * rho * int(y * dA)
	// I = rho * int((x*x + y*y) * dA)
	//
	// We can compute these integrals by summing all the integrals
	// for each triangle of the polygon. To evaluate the integral
	// for a single triangle, we make a change of variables to
	// the (u,v) coordinates of the triangle:
	// x = x0 + e1x * u + e2x * v
	// y = y0 + e1y * u + e2y * v
	// where 0 <= u && 0 <= v && u + v <= 1.
	//
	// We integrate u from [0,1-v] and then v from [0,1].
	// We also need to use the Jacobian of the transformation:
	// D = cross(e1, e2)
	//
	// Simplification: triangle centroid = (1/3) * (p1 + p2 + p3)
	//
	// The rest of the derivation is handled by computer algebra.

	b2Assert(m_count >= 3);

	b2Vec2 center; center.Set(0.0f, 0.0f);
	float32 area = 0.0f;
	float32 I = 0.0f;

	// s is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
	b2Vec2 s(0.0f, 0.0f);

	// This code would put the reference point inside the polygon.
	for (int32 i = 0; i < m_count; ++i)
	{
		s += m_vertices[i];
	}
	s *= 1.0f / m_count;

	const float32 k_inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < m_count; ++i)
	{
		// Triangle vertices.
		b2Vec2 e1 = m_vertices[i] - s;
		b2Vec2 e2 = i + 1 < m_count ? m_vertices[i+1] - s : m_vertices[0] - s;

		float32 D = b2Cross(e1, e2);

		float32 triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid
		center += triangleArea * k_inv3 * (e1 + e2);

		float32 ex1 = e1.x, ey1 = e1.y;
		float32 ex2 = e2.x, ey2 = e2.y;

		float32 intx2 = ex1*ex1 + ex2*ex1 + ex2*ex2;
		float32 inty2 = ey1*ey1 + ey2*ey1 + ey2*ey2;

		I += (0.25f * k_inv3 * D) * (intx2 + inty2);
	}

	// Total mass
	massData->mass = density * area;

	// Center of mass
	b2Assert(area > b2_epsilon);
	center *= 1.0f / area;
	massData->center = center + s;

	// Inertia tensor relative to the local origin (point s).
	massData->I = density * I;
	
	// Shift to center of mass then to original body origin.
	massData->I += massData->mass * (b2Dot(massData->center, massData->center) - b2Dot(center, center));
}

bool b2PolygonShape::Validate() const
{
	for (int32 i = 0; i < m_count; ++i)
	{
		int32 i1 = i;
		int32 i2 = i < m_count - 1 ? i1 + 1 : 0;
		b2Vec2 p = m_vertices[i1];
		b2Vec2 e = m_vertices[i2] - p;

		for (int32 j = 0; j < m_count; ++j)
		{
			if (j == i1 || j == i2)
			{
				continue;
			}

			b2Vec2 v = m_vertices[j] - p;
			float32 c = b2Cross(e, v);
			if (c < 0.0f)
			{
				return false;
			}
		}
	}

	return true;
}
