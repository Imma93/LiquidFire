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

#ifndef B2_WORLD_CALLBACKS_H
#define B2_WORLD_CALLBACKS_H

#include <Box2D/Common/b2Settings.h>

struct b2Vec2;
struct b2Transform;
class b2Fixture;
class b2Body;
class b2Joint;
class b2Contact;
class b2ParticleSystem;
struct b2ContactResult;
struct b2Manifold;
class b2ParticleGroup;
class b2BodyMaterial;
struct b2ParticleContact;
struct b2PartBodyContact;

/// Joints and fixtures are destroyed when their associated
/// body is destroyed. Implement this listener so that you
/// may nullify references to these joints and shapes.
class b2DestructionListener
{
public:
	virtual ~b2DestructionListener() {}

	/// Called when any joint is about to be destroyed due
	/// to the destruction of one of its attached bodies.
	virtual void SayGoodbye(b2Joint* joint) = 0;

	/// Called when any fixture is about to be destroyed due
	/// to the destruction of its parent body.
	virtual void SayGoodbye(b2Fixture& fixture) = 0;

	/// Called when any particle group is about to be destroyed.
	virtual void SayGoodbye(b2ParticleGroup* group)
	{
		B2_NOT_USED(group);
	}
	/// Called when any fixture material is about to be destroyed.
	virtual void SayGoodbye(b2BodyMaterial* mat)
	{
		B2_NOT_USED(mat);
	}


	/// Called when a particle is about to be destroyed.
	/// The index can be used in conjunction with
	/// b2ParticleSystem::GetUserDataBuffer() or
	/// b2ParticleSystem::GetParticleHandleFromIndex() to determine which
	/// particle has been destroyed.
	virtual void SayGoodbye(b2ParticleSystem* particleSystem, int32 index)
	{
		B2_NOT_USED(particleSystem);
		B2_NOT_USED(index);
	}
};
class AFDestructionListener
{
public:
	virtual ~AFDestructionListener() {}

	/// Called when any joint is about to be destroyed due
	/// to the destruction of one of its attached bodies.
	virtual void SayGoodbye(b2Joint* joint) = 0;

	/// Called when any fixture is about to be destroyed due
	/// to the destruction of its parent body.
	virtual void SayGoodbye(b2Fixture& fixture) = 0;

	/// Called when any particle group is about to be destroyed.
	virtual void SayGoodbye(int32 groupIdx)
	{
		B2_NOT_USED(groupIdx);
	}
	/// Called when any fixture material is about to be destroyed.
	virtual void SayGoodbye(b2BodyMaterial* mat)
	{
		B2_NOT_USED(mat);
	}


	/// Called when a particle is about to be destroyed.
	/// The index can be used in conjunction with
	/// b2ParticleSystem::GetUserDataBuffer() or
	/// b2ParticleSystem::GetParticleHandleFromIndex() to determine which
	/// particle has been destroyed.
	virtual void SayGoodbye(b2ParticleSystem* particleSystem)
	{
		B2_NOT_USED(particleSystem);
	}
};

/// Implement this class to provide collision filtering. In other words, you can implement
/// this class if you want finer control over contact creation.
class b2ContactFilter
{
public:
	virtual ~b2ContactFilter() {}

	/// Return true if contact calculations should be performed between these two shapes.
	/// @warning for performance reasons this is only called when the AABBs begin to overlap.
	virtual bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB);

	/// Return true if contact calculations should be performed between a
	/// fixture and particle.  This is only called if the
	/// b2_fixtureContactListenerParticle flag is set on the particle.
	virtual bool ShouldCollide(b2Fixture* fixture,
							   b2ParticleSystem* particleSystem,
							   int32 particleIndex)
	{
		B2_NOT_USED(fixture);
		B2_NOT_USED(particleIndex);
		B2_NOT_USED(particleSystem);
		return true;
	}

	/// Return true if contact calculations should be performed between two
	/// particles.  This is only called if the
	/// b2_particleContactListenerParticle flag is set on the particle.
	virtual bool ShouldCollide(b2ParticleSystem* particleSystem,
							   int32 particleIndexA, int32 particleIndexB)
	{
		B2_NOT_USED(particleSystem);
		B2_NOT_USED(particleIndexA);
		B2_NOT_USED(particleIndexB);
		return true;
	}
};

/// Contact impulses for reporting. Impulses are used instead of forces because
/// sub-step forces may approach infinity for rigid body collisions. These
/// match up one-to-one with the contact points in b2Manifold.
struct b2ContactImpulse
{
	float32 normalImpulses[b2_maxManifoldPoints];
	float32 tangentImpulses[b2_maxManifoldPoints];
	int32 count;
};

/// Implement this class to get contact information. You can use these results for
/// things like sounds and game logic. You can also get contact results by
/// traversing the contact lists after the time step. However, you might miss
/// some contacts because continuous physics leads to sub-stepping.
/// Additionally you may receive multiple callbacks for the same contact in a
/// single time step.
/// You should strive to make your callbacks efficient because there may be
/// many callbacks per time step.
/// @warning You cannot create/destroy Box2D entities inside these callbacks.
class b2ContactListener
{
public:
	virtual ~b2ContactListener() {}

	/// Called when two fixtures begin to touch.
	virtual void BeginContact(b2Contact* contact) { B2_NOT_USED(contact); }

	/// Called when two fixtures cease to touch.
	virtual void EndContact(b2Contact* contact) { B2_NOT_USED(contact); }

	/// Called when a fixture and particle start touching if the
	/// b2_fixtureContactFilterParticle flag is set on the particle.
	virtual void BeginContact(b2ParticleSystem* particleSystem,
							  b2PartBodyContact* particleBodyContact)
	{
		B2_NOT_USED(particleSystem);
		B2_NOT_USED(particleBodyContact);
	}

	/// Called when a fixture and particle stop touching if the
	/// b2_fixtureContactFilterParticle flag is set on the particle.
	virtual void EndContact(int32 fixtureIdx,
							b2ParticleSystem* particleSystem, int32 index)
	{
		B2_NOT_USED(fixtureIdx);
		B2_NOT_USED(particleSystem);
		B2_NOT_USED(index);
	}

	/// Called when two particles start touching if
	/// b2_particleContactFilterParticle flag is set on either particle.
	virtual void BeginContact(b2ParticleSystem* particleSystem,
							  b2ParticleContact* particleContact)
	{
		B2_NOT_USED(particleSystem);
		B2_NOT_USED(particleContact);
	}

	/// Called when two particles start touching if
	/// b2_particleContactFilterParticle flag is set on either particle.
	virtual void EndContact(b2ParticleSystem* particleSystem,
							int32 indexA, int32 indexB)
	{
		B2_NOT_USED(particleSystem);
		B2_NOT_USED(indexA);
		B2_NOT_USED(indexB);
	}

	/// This is called after a contact is updated. This allows you to inspect a
	/// contact before it goes to the solver. If you are careful, you can modify the
	/// contact manifold (e.g. disable contact).
	/// A copy of the old manifold is provided so that you can detect changes.
	/// Note: this is called only for awake bodies.
	/// Note: this is called even when the number of contact points is zero.
	/// Note: this is not called for sensors.
	/// Note: if you set the number of contact points to zero, you will not
	/// get an EndContact callback. However, you may get a BeginContact callback
	/// the next step.
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		B2_NOT_USED(contact);
		B2_NOT_USED(oldManifold);
	}

	/// This lets you inspect a contact after the solver is finished. This is useful
	/// for inspecting impulses.
	/// Note: the contact manifold does not include time of impact impulses, which can be
	/// arbitrarily large if the sub-step is small. Hence the impulse is provided explicitly
	/// in a separate data structure.
	/// Note: this is only called for contacts that are touching, solid, and awake.
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		B2_NOT_USED(contact);
		B2_NOT_USED(impulse);
	}
};

/// Callback class for AABB queries.
/// See b2World::Query
class b2QueryCallback
{
public:
	virtual ~b2QueryCallback() {}

	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	virtual bool ReportFixture(int32 fixtureIdx) = 0;

	/// Called for each particle found in the query AABB.
	/// @return false to terminate the query.
	virtual bool ReportParticle(const b2ParticleSystem* particleSystem,
								int32 index)
	{
		B2_NOT_USED(particleSystem);
		B2_NOT_USED(index);
		return false;
	}

	/// Cull an entire particle system from b2World::QueryAABB. Ignored for
	/// b2ParticleSystem::QueryAABB.
	/// @return true if you want to include particleSystem in the AABB query,
	/// or false to cull particleSystem from the AABB query.
	virtual bool ShouldQueryParticleSystem(
		const b2ParticleSystem* particleSystem)
	{
		B2_NOT_USED(particleSystem);
		return true;
	}
};
class afQueryCallback
{
public:
	virtual ~afQueryCallback() {}

	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	virtual bool AFReportFixture(int32 fixtureIdx) = 0;

	/// Cull an entire particle system from b2World::QueryAABB. Ignored for
	/// b2ParticleSystem::QueryAABB.
	/// @return true if you want to include particleSystem in the AABB query,
	/// or false to cull particleSystem from the AABB query.
	virtual bool AFShouldQueryParticleSystem(
		const b2ParticleSystem* particleSystem)
	{
		B2_NOT_USED(particleSystem);
		return true;
	}
};

/// Callback class for ray casts.
/// See b2World::RayCast
class b2RayCastCallback
{
public:
	virtual ~b2RayCastCallback() {}

	/// Called for each fixture found in the query. You control how the ray cast
	/// proceeds by returning a float:
	/// return -1: ignore this fixture and continue
	/// return 0: terminate the ray cast
	/// return fraction: clip the ray to this point
	/// return 1: don't clip the ray and continue
	/// @param fixture the fixture hit by the ray
	/// @param point the point of initial intersection
	/// @param normal the normal vector at the point of intersection
	/// @return -1 to filter, 0 to terminate, fraction to clip the ray for
	/// closest hit, 1 to continue
	virtual float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
									const b2Vec2& normal, float32 fraction) = 0;

	/// Called for each particle found in the query. You control how the ray
	/// cast proceeds by returning a float:
	/// return <=0: ignore the remaining particles in this particle system
	/// return fraction: ignore particles that are 'fraction' percent farther
	///   along the line from 'point1' to 'point2'. Note that 'point1' and
	///   'point2' are parameters to b2World::RayCast.
	/// @param particleSystem the particle system containing the particle
	/// @param index the index of the particle in particleSystem
	/// @param point the point of intersection bt the ray and the particle
	/// @param normal the normal vector at the point of intersection
	/// @param fraction percent (0.0~1.0) from 'point0' to 'point1' along the
	///   ray. Note that 'point1' and 'point2' are parameters to
	///   b2World::RayCast.
	/// @return <=0 to ignore rest of particle system, fraction to ignore
	/// particles that are farther away.
	virtual float32 ReportParticle(const b2ParticleSystem* particleSystem,
								   int32 index, const b2Vec2& point,
								   const b2Vec2& normal, float32 fraction)
	{
		B2_NOT_USED(particleSystem);
		B2_NOT_USED(index);
		B2_NOT_USED(&point);
		B2_NOT_USED(&normal);
		B2_NOT_USED(fraction);
		return 0;
	}

	/// Cull an entire particle system from b2World::RayCast. Ignored in
	/// b2ParticleSystem::RayCast.
	/// @return true if you want to include particleSystem in the RayCast, or
	/// false to cull particleSystem from the RayCast.
	virtual bool ShouldQueryParticleSystem(
		const b2ParticleSystem* particleSystem)
	{
		B2_NOT_USED(particleSystem);
		return true;
	}
};

#endif
