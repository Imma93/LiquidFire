/*
* Copyright (c) 2006-2010 Erin Catto http://www.box2d.org
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

#pragma once

#include <Box2D/Collision/Shapes/b2Shape.h>
#include <Box2D/Collision/Shapes/b2EdgeShape.h>

/// A chain shape is a free form sequence of line segments.
/// The chain has two-sided collision, so you can use inside and outside collision.
/// Therefore, you may use any winding order.
/// Since there may be many vertices, they are allocated using b2Alloc.
/// Connectivity information is used to create smooth collisions.
/// WARNING: The chain will not collide properly if there are self-intersections.
struct b2ChainShape : public b2Shape
{
	/// The vertices. Owned by this class.
	Vec2 m_vertices[b2_maxChainVertices];

	/// The vertex count.
	int32 m_count;

	Vec2 m_prevVertex, m_nextVertex;
	int32 m_hasPrevVertex, m_hasNextVertex;

	b2ChainShape();

	/// The destructor frees the vertices using b2Free.
	~b2ChainShape();

	/// Create a loop. This automatically adjusts connectivity.
	/// @param vertices an array of vertices, these are copied
	/// @param count the vertex count
	void CreateLoop(const Vec2* vertices, int32 count);

	/// Create a chain with isolated end vertices.
	/// @param vertices an array of vertices, these are copied
	/// @param count the vertex count
	void CreateChain(const Vec2* vertices, int32 count);

	/// Establish connectivity to a vertex that precedes the first vertex.
	/// Don't call this for loops.
	void SetPrevVertex(const Vec2& prevVertex);

	/// Establish connectivity to a vertex that follows the last vertex.
	/// Don't call this for loops.
	void SetNextVertex(const Vec2& nextVertex);

	/// Implement b2Shape. Vertices are cloned using b2Alloc.
	b2Shape* Clone(b2BlockAllocator* allocator) const;

	/// @see b2Shape::GetChildCount
	int32 GetChildCount() const;

	/// Get a child edge.
	void GetChildEdge(b2EdgeShape& edge, int32 index) const;

	/// This always return false.
	/// @see b2Shape::TestPoint
	bool TestPoint(const b2Transform& transform, const Vec3& p) const;

	// @see b2Shape::ComputeDistance
	void ComputeDistance(const b2Transform& xf, const Vec2& p, float32& distance, Vec2& normal, int32 childIndex) const;

	/// Implement b2Shape.
	bool RayCast(b2RayCastOutput& output, const b2RayCastInput& input,
					const b2Transform& transform, int32 childIndex) const;

	/// @see b2Shape::ComputeAABB
	void ComputeAABB(b2AABB& aabb, const b2Transform& transform, int32 childIndex) const;

	/// Chains have zero mass.
	/// @see b2Shape::ComputeMass
	b2MassData ComputeMass(float32 density, float32 surfaceThickness, float32 massMult) const;
};

struct AmpChainShape
{
	int32 _vfptr[3];
	b2Shape::Type m_type;
	float32 m_radius;
	float32 m_zPos;
	float32 m_height;
	float32 m_area;
	/// The vertices. Owned by this class.
	Vec2 m_vertices[b2_maxChainVertices];

	/// The vertex count.
	int32 m_count;

	Vec2 m_prevVertex, m_nextVertex;
	int32 m_hasPrevVertex, m_hasNextVertex;

	void GetChildEdge(AmpEdgeShape& edge, int32 index) const restrict(amp)
	{
		edge.m_type = b2Shape::e_edge;
		edge.m_radius = m_radius;

		edge.m_vertex1 = m_vertices[index + 0];
		edge.m_vertex2 = m_vertices[index + 1];

		if (index > 0)
		{
			edge.m_vertex0 = m_vertices[index - 1];
			edge.m_hasVertex0 = true;
		}
		else
		{
			edge.m_vertex0 = m_prevVertex;
			edge.m_hasVertex0 = m_hasPrevVertex;
		}
		if (index < m_count - 2)
		{
			edge.m_vertex3 = m_vertices[index + 2];
			edge.m_hasVertex3 = true;
		}
		else
		{
			edge.m_vertex3 = m_nextVertex;
			edge.m_hasVertex3 = m_hasNextVertex;
		}
	}

	void ComputeDistance(const b2Transform& xf, const Vec2& p,
		float32& distance, Vec2& normal, int32 childIndex) const restrict(amp)
	{
		AmpEdgeShape edge;
		GetChildEdge(edge, childIndex);
		edge.ComputeDistance(xf, p, distance, normal);
	}

	bool TestZ(const b2Transform& xf, float32 z) const restrict(amp)
	{
		z -= (m_zPos + xf.z);
		return z >= 0 && z <= m_height;
	}

	bool RayCast(b2RayCastOutput& output, const b2RayCastInput& input,
		const b2Transform& xf, int32 childIndex) const restrict(amp)
	{
		AmpEdgeShape edgeShape;

		int32 i1 = childIndex;
		int32 i2 = childIndex + 1;
		if (i2 == m_count)
			i2 = 0;

		edgeShape.m_vertex1 = m_vertices[i1];
		edgeShape.m_vertex2 = m_vertices[i2];

		return edgeShape.RayCast(output, input, xf);
	}
};

inline b2ChainShape::b2ChainShape()
{
	m_type = b2Shape::e_chain;
	m_radius = b2_polygonRadius;
	m_count = 0;
	m_hasPrevVertex = false;
	m_hasNextVertex = false;
}
