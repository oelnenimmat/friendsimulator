/*=============================================================================
Leo Tamminen

Animation System
=============================================================================*/

enum InterpolationMode
{
	INTERPOLATION_MODE_STEP,
	INTERPOLATION_MODE_LINEAR,
	INTERPOLATION_MODE_CUBICSPLINE,

	INTERPOLATION_MODE_COUNT
};

enum ChannelType
{
	ANIMATION_CHANNEL_TRANSLATION,
	ANIMATION_CHANNEL_ROTATION,
	ANIMATION_CHANNEL_SCALE,

	ANIMATION_CHANNEL_COUNT
};

struct AnimationChannel
{
	s32 				targetIndex;

	Array<f32> 			times;
	Array<v3> 			translations;
	Array<quaternion> 	rotations;

	ChannelType 		type;
	InterpolationMode 	interpolationMode;

	s32 keyframe_count() const { return times.count(); }
};

// Data
// Note(Leo): This is for complete animation, with different keyframes for different bones
struct Animation
{
	Array<AnimationChannel> channels;
	f32 					duration;

	bool 					loop = false;
};

enum { NOT_FOUND = (u32)-1 };
internal u32 find_channel(Animation const & animation, s32 targetIndex, ChannelType type)
{
	for (s32 channelIndex = 0; channelIndex < animation.channels.count(); ++channelIndex)
	{
		if (animation.channels[channelIndex].targetIndex == targetIndex
			&& animation.channels[channelIndex].type == type)
		{
			return channelIndex;
		}
	}
	return NOT_FOUND;
}

// Property/State
struct AnimationRig
{
	Transform3D * 		root;
	Array<Transform3D*> bones;
};

struct Animator
{
	// Properties
	AnimationRig rig;
	f32 playbackSpeed	= 1.0f;

	// State
	f32 				time;
	Animation const * 	animation;
};

struct AnimatedBone
{
	// State
	Transform3D boneSpaceTransform;

	// Properties
	Transform3D boneSpaceDefaultTransform;
	m44 		inverseBindMatrix;
	s32 		parent;
	SmallString name;

	bool is_root() const { return parent < 0; }
};

internal AnimatedBone
make_bone (Transform3D boneSpaceDefaultTransform, m44 inverseBindMatrix, s32 parent)
{
	AnimatedBone bone = {	Transform3D::identity(),
					boneSpaceDefaultTransform,
					inverseBindMatrix,
					parent};
	return bone;
}

struct AnimatedSkeleton
{
	Array<AnimatedBone> bones;
};

struct SkeletonAnimator
{
	AnimatedSkeleton 	skeleton;

	Animation const ** 	animations;
	f32 *				weights;
	s32					animationCount;
	f32 				animationTime;
};

v3 get_model_space_position(Array<AnimatedBone> const & skeleton, u32 boneIndex)
{
	v3 position = skeleton[boneIndex].boneSpaceTransform.position;
	if (skeleton[boneIndex].is_root() == false)
	{
		position += get_model_space_position(skeleton, skeleton[boneIndex].parent);
	}
	return position;
}

m44 get_model_space_transform(Array<AnimatedBone> const & skeleton, u32 boneIndex)
{
	AnimatedBone const & bone = skeleton[boneIndex];

	m44 transform = get_matrix(bone.boneSpaceTransform);
	if (bone.is_root() == false)
	{
		transform = get_model_space_transform(skeleton, bone.parent) * transform; 
	}
	return transform;
}

internal AnimationRig
make_animation_rig(Transform3D * root, Array<Transform3D*> bones)
{
	AnimationRig result 
	{
		.root 							= root,
		.bones 							= std::move(bones),
	};
	return result;
}

internal Animation
copy_animation(MemoryArena & memoryArena, Animation const & original)
{
	Animation result = 
	{
		.channels = allocate_array<AnimationChannel>(memoryArena, original.channels.count()),
		.duration = original.duration
	};

	for (s32 childIndex = 0; childIndex < original.channels.count(); ++childIndex)
	{
		result.channels.push({original.channels[childIndex].targetIndex, 
								copy_array(memoryArena, original.channels[childIndex].times),
								copy_array(memoryArena, original.channels[childIndex].translations),
								copy_array(memoryArena, original.channels[childIndex].rotations) });
	}
	return result;
}

internal void
reverse_animation_clip(Animation & animation)
{
	for (AnimationChannel & channel : animation.channels)
	{
		switch(channel.type)
		{
			case ANIMATION_CHANNEL_TRANSLATION:
				reverse_array(channel.translations);
				break;

			case ANIMATION_CHANNEL_ROTATION:
				reverse_array(channel.rotations);
				break;

			default:
				logDebug(0) << FILE_ADDRESS << "Invalid or unimplemented animation channel type: " << channel.type;
				// Note(Leo): Continue here instead of break, we probably should not touch times either.
				continue;
		}

		reverse_array(channel.times);

		for(f32 & time : channel.times)
		{
			time = animation.duration - time;
		}
	}
}

internal f32
compute_duration (Array<AnimationChannel> const & channels)
{
	f32 duration = 0;
	for (auto & channel : channels)
	{
		duration = math::max(duration, channel.times.last());
	}
	return duration;
}

internal Animation
make_animation (Array<AnimationChannel> channels)
{
	f32 duration = compute_duration(channels);

	Animation result = 
	{
		.channels = std::move(channels),
		.duration = duration
	};
	return result;
}

s32 previous_keyframe(Array<f32> const & keyframeTimes, f32 time)
{
	/* Note(Leo): In a looping animation, we have two possibilities to return 
	the last keyframe. First, if actual time is before the first actual keyframe,
	we then return last keyframe (from last loop round). The other is, if time is 
	more than that keyframe's time, so that it was actually the last. */

	// Todo(Leo): Reduce branching maybe.

	// Note(Leo): Last keyframe from previous loop round
	if (time < keyframeTimes[0])
	{
		return keyframeTimes.count() - 1;
	}

	for (s32 i = 0; i < keyframeTimes.count() - 1; ++i)
	{
		f32 previousTime = keyframeTimes[i];
		f32 nextTime = keyframeTimes[i + 1];

		if (previousTime <= time && time <= nextTime)
		{
			return i;
		}
	}
	
	// Note(Leo): this loop rounds last keyframe 
	return keyframeTimes.count() - 1;
}

internal void
update_animator_system(game::Input * input, Array<Animator> & animators)
{
	for (auto & animator : animators)
	{
		if (animator.animation == nullptr)
			continue;

		f32 timeStep = animator.playbackSpeed * input->elapsedTime;
		animator.time += timeStep;

		for (auto const & channel : animator.animation->channels)
		{
			Transform3D & target = *animator.rig.bones[channel.targetIndex];

			// Before first keyframe
			if (animator.time < channel.times[0])
			{
				target.position = channel.translations[0];
			}
			// After last keyframe
			else if(bool isAfterLast = channel.times.last() < animator.time)
			{
				target.position = channel.translations.last();
			}
			else
			{
				s32 previousKeyframe 		= previous_keyframe(channel.times, animator.time);
				s32 nextKeyframe 			= previousKeyframe + 1;

				f32 previousKeyFrameTime 	= channel.times[previousKeyframe];
				f32 currentKeyFrameTime 	= channel.times[nextKeyframe];
				f32 relativeTime 			= (animator.time - previousKeyFrameTime) / (currentKeyFrameTime - previousKeyFrameTime);

				target.position = vector::interpolate(	channel.translations[previousKeyframe],
														channel.translations[nextKeyframe],
														relativeTime); 
			}
		}

		bool framesLeft = animator.time < animator.animation->duration;

		if (framesLeft == false)
		{
			animator.animation = nullptr;
		}
	}
}

internal Animator
make_animator(AnimationRig rig)
{
	Animator result = 
	{
		.rig = std::move(rig)
	};
	return result;
}


internal void
play_animation_clip(Animator * animator, Animation const * animation)
{
	animator->animation = animation;
	animator->time 		= 0.0f;
}



f32 loop_time(f32 time, f32 duration)
{
	if (duration == 0)
		time = 0;
	else
		time = modulo(time, duration);

	return time;
}


f32 get_relative_time(AnimationChannel const & 	channel,
						s32 						previousKeyframeIndex,
						s32 						nextKeyframeIndex,
						f32 						animationTime,
						f32 						animationDuration)
{
	f32 previousTime 	= channel.times[previousKeyframeIndex];
	f32 nextTime 		= channel.times[nextKeyframeIndex];

	f32 swagEpsilon = 0.000000001f; // Should this be in scale of frame time?

	// Note(Leo): these are times from previous so that we get [0 ... 1] range for interpolation
	f32 timeToNow = animationTime - previousTime;
	if (timeToNow < swagEpsilon)
	{
		timeToNow = animationDuration - timeToNow;
	} 

	f32 timeToNext = nextTime - previousTime;
	if (timeToNext < swagEpsilon)
	{
		timeToNext = animationDuration - timeToNext;
	}
	f32 relativeTime = timeToNow / timeToNext;

	return relativeTime;
}

// void update_skeleton_animator(AnimatedSkeleton & skeleton, Animation const ** animations, f32 * weights, s32 animationCount, f32 elapsedTime)
void update_skeleton_animator(SkeletonAnimator & animator, f32 elapsedTime)
{
	/*
	for bone in skeleton:
		reset bone

	for animation in animations:
		for bone in skeleton:
			if channel in animation for bone:
				apply animation by animations weight
			else
				apply default pose by animations weight		
	*/

	auto & skeleton 	= animator.skeleton;
	auto ** animations 	= animator.animations;
	f32 * weights 		= animator.weights;
	s32 animationCount 	= animator.animationCount;

	auto & bones = skeleton.bones;
	for (s32 boneIndex = 0; boneIndex < bones.count(); ++boneIndex)
	{
		bones[boneIndex].boneSpaceTransform = {};
	}

	f32 totalAppliedWeight = 0;

	animator.animationTime += elapsedTime;
	constexpr f32 resetInterval = 120; // Note(Leo): random amount to prevent us getting somewhere where floating point accuracy becomes problem
	animator.animationTime = modulo(animator.animationTime, resetInterval);

	for (s32 animationIndex = 0; animationIndex < animationCount; ++animationIndex)
	{

		Animation const & animation = *animations[animationIndex];

		f32 animationWeight = weights[animationIndex];
		f32 animationTime = loop_time(animator.animationTime, animation.duration);


		totalAppliedWeight += animationWeight;
		f32 relativeWeight = animationWeight / totalAppliedWeight;

		if (animationWeight < 0.00001f)
		{
			continue;
		}

		// ------------------------------------------------------------------------

		for (s32 boneIndex = 0; boneIndex < bones.count(); ++boneIndex)
		{
			auto & bone = bones[boneIndex];

			///////////////////////////////////////////
			/// 		ROTATION 					///
			///////////////////////////////////////////
			quaternion rotation = bone.boneSpaceDefaultTransform.rotation;
			u32 channelIndex = find_channel(animation, boneIndex, ANIMATION_CHANNEL_ROTATION);
			if (channelIndex != NOT_FOUND)
			{
				auto const & channel = animation.channels[channelIndex];

				switch(channel.interpolationMode)
				{
					case INTERPOLATION_MODE_STEP:
					{
						s32 previousKeyframeIndex 	= previous_keyframe(channel.times, animationTime);
						rotation 					= channel.rotations[previousKeyframeIndex]; 
					} break;

					case INTERPOLATION_MODE_LINEAR:
					{
						s32 previousKeyframeIndex	= previous_keyframe(channel.times, animationTime);
						s32 nextKeyframeIndex 		= (previousKeyframeIndex + 1) % channel.keyframe_count();
						f32 relativeTime 			= get_relative_time(channel, previousKeyframeIndex, nextKeyframeIndex, animationTime, animation.duration);
						rotation 					= interpolate(	channel.rotations[previousKeyframeIndex],
																	channel.rotations[nextKeyframeIndex],
																	relativeTime);
					} break;
					case INTERPOLATION_MODE_CUBICSPLINE:
					{
						s32 previousKeyframeIndex	= previous_keyframe(channel.times, animationTime);
						s32 nextKeyframeIndex 		= (previousKeyframeIndex + 1) % channel.keyframe_count();
						f32 relativeTime 			= get_relative_time(channel, previousKeyframeIndex, nextKeyframeIndex, animationTime, animation.duration);

						// Note(Leo): We don't actually support cubicspline rotations yet; this way we skip tangent values;
						previousKeyframeIndex 		= (previousKeyframeIndex * 3) + 1;
						nextKeyframeIndex 			= (nextKeyframeIndex * 3) + 1;

						rotation 					= interpolate(	channel.rotations[previousKeyframeIndex],
																	channel.rotations[nextKeyframeIndex],
																	relativeTime);
					} break;

					default:
						logDebug(1) << "Now thats a weird interpolation mode: " << channel.interpolationMode;
						break;
				}
			}
			bone.boneSpaceTransform.rotation = interpolate(bone.boneSpaceTransform.rotation, rotation, relativeWeight);

			// ----------------------------------------------------------------

			///////////////////////////////////////
			/// 		TRANSLATION 			///
			///////////////////////////////////////
			v3 translation = bone.boneSpaceDefaultTransform.position;
			channelIndex = find_channel(animation, boneIndex, ANIMATION_CHANNEL_TRANSLATION);
			if (channelIndex != NOT_FOUND)
			{
				auto const & channel = animation.channels[channelIndex];

				switch(channel.interpolationMode)
				{
					case INTERPOLATION_MODE_STEP:
					{
						s32 previousKeyframeIndex 	= previous_keyframe(channel.times, animationTime);
						translation 				= channel.translations[previousKeyframeIndex]; 
					} break;

					case INTERPOLATION_MODE_LINEAR:
					{
						s32 previousKeyframeIndex 	= previous_keyframe(channel.times, animationTime);
						s32 nextKeyframeIndex 		= (previousKeyframeIndex + 1) % channel.keyframe_count();
						f32 relativeTime 			= get_relative_time(channel, previousKeyframeIndex, nextKeyframeIndex, animationTime, animation.duration);
						translation 				= vector::interpolate(	channel.translations[previousKeyframeIndex],
																			channel.translations[nextKeyframeIndex],
																			relativeTime);
					} break;

					case INTERPOLATION_MODE_CUBICSPLINE:
					{

						s32 previousKeyframeIndex 	= previous_keyframe(channel.times, animationTime);
						s32 nextKeyframeIndex 		= (previousKeyframeIndex + 1) % channel.keyframe_count();
						f32 relativeTime 			= get_relative_time(channel, previousKeyframeIndex, nextKeyframeIndex, animationTime, animation.duration);


						// Note(Leo): We don't actually support cubicspline rotations yet; this way we skip tangent values;
						previousKeyframeIndex 		= (previousKeyframeIndex * 3) + 1;
						nextKeyframeIndex 			= (nextKeyframeIndex * 3) + 1;

						translation 				= vector::interpolate(	channel.translations[previousKeyframeIndex],
																			channel.translations[nextKeyframeIndex],
																			relativeTime);
					} break;

					default:
						logDebug(1) << "Now thats a weird interpolation mode: " << channel.interpolationMode;
						break;
				}
			}
			bone.boneSpaceTransform.position += translation * animationWeight;
		}
	}
}

// Here lies some proof for way we add animations, particularly rotations, together
// a, b, c, d
// x, y, z, w

// --> t = a * x / (x + y + z + w)
// 		+ b * y / (x + y + z + w)
// 		+ c * z / (x + y + z + w)
// 		+ d * w / (x + y + z + w)

// x + y + z + w = 1

// --> t = a * x
// 		+ b * y
// 		+ c * z 
// 		+ d * w

// t = 0

// x = 0.1
// y = 0.4
// z = 0.3
// y = 0.2

// t = lerp(t, a, 0.1/0.1)		-> a: 1
// t = lerp(t, b, 0.4/0.5)		-> a: 0.2 b: 0.8
// t = lerp(t, c, 0.3/0.8)		-> a: 0.125 b: 0.5 c: 0.375
// t = lerp(t, d, 0.2/1.0)		-> a: 0.1 b: 0.4 c: 0.3 d: 0.2

// t = 0

// t = lerp(t, a, x / x)				-> a0: x / x
// t = lerp(t, b, y / (x + y)) 		-> a1: a0 * x / (x + y)
// t = lerp(t, c, z / (x + y + z))		-> a2: a1 * (x + y) / (x + y + z)
// t = lerp(t, d, w / (x + y + z + w))	-> a3: a2 * (x + y + z) / (x + y + z + w)

// a = (((x / x) * x / (x + y)) * (x + y) / (x + y + z)) * (x + y + z) / (x + y + z + w)

// // n = previous_round_value * previous_round_total_weight / current_total_weight

// t = lerp(t, b, y / (x + y)) 		-> b0: y / (x + y)
// t = lerp(t, c, z / (x + y + z))		-> b1: b0 * (x + y) / (x + y + z)
// t = lerp(t, d, w / (x + y + z + w))	-> b2: b1 * (x + y + z) / (x + y + z + w)

// b = ((y / (x + y)) * (x + y) / (x + y + z)) * (x + y + z) / (x + y + z + w)



// t = lerp(t, c, z / (x + y + z))		-> c0: z / (x + y + z)
// t = lerp(t, d, w / (x + y + z + w))	-> c1: c0 * (x + y + z) / (x + y + w + z)

// c = z / (x + y + z) * (x + y + z) / (x + y + w + z)

// t = lerp(t, d, w /(x + y + z + w)) 	-> d0: w / (x + y + z + w) 

// d = w / (x + y + z + w)

// ------------

// 	a = (((x / x) * x / (x + y)) * (x + y) / (x + y + z)) * (x + y + z) / (x + y + z + w)
// 	b = ((y / (x + y)) * (x + y) / (x + y + z)) * (x + y + z) / (x + y + z + w)
// 	c = z / (x + y + z) * (x + y + z) / (x + y + w + z)
// 	d = w / (x + y + z + w)

// (x + y + z + w) = 1

// 	// Canceling pairs and substituting (x + y + z + w) = 1 leaves us back to correct values :)
// 	a = x / /*x * x*/ / /*(x + y) * (x + y)*/ / /*(x + y + z) * (x + y + z)*/ / 1
// 	b = y / /*(x + y) * (x + y)*/ / /*(x + y + z) * (x + y + z)*/ / 1
// 	c = z / /*(x + y + z) * (x + y + z)*/ / 1
// 	d = w / 1
// 	// mot



// t = 0
// t = lerp(t, a, 1)			-> a: 1
// t = lerp(t, b, 0.5)			-> a: 0.5, b: 0.5
// t = lerp(t, c, 0.33)		-> a: 0.33, b: 0.33, c: 0.33
// t = lerp(t, d, 0.25)		-> a: 0.25, b: 0.25, c: 0.25, d: 0.25