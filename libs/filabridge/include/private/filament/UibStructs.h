/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILABRIDGE_UIBSTRUCTS_H
#define TNT_FILABRIDGE_UIBSTRUCTS_H

#include <math/mat4.h>
#include <math/vec4.h>

#include <private/filament/EngineEnums.h>

#include <utils/CString.h>

/*
 * Here we define all the UBOs known by filament as C structs. It is used by filament to
 * fill the uniform values and get the interface block names. It is also used by filabridge to
 * get the interface block names.
 */

namespace filament {

/*
 * These structures are only used to call offsetof() and make it easy to visualize the UBO.
 *
 * IMPORTANT NOTE: Respect std140 layout, don't update without updating getUib()
 */

struct PerViewUib { // NOLINT(cppcoreguidelines-pro-type-member-init)
    static constexpr utils::StaticString _name{ "FrameUniforms" };
    filament::math::mat4f viewFromWorldMatrix;
    filament::math::mat4f worldFromViewMatrix;
    filament::math::mat4f clipFromViewMatrix;
    filament::math::mat4f viewFromClipMatrix;
    filament::math::mat4f clipFromWorldMatrix;
    filament::math::mat4f worldFromClipMatrix;
    filament::math::mat4f lightFromWorldMatrix[CONFIG_MAX_SHADOW_CASCADES];

    // position of cascade splits, in world space (not including the near plane)
    // -Inf stored in unused components
    filament::math::float4 cascadeSplits;

    filament::math::float4 resolution; // viewport width, height, 1/width, 1/height

    // camera position in view space (when camera_at_origin is enabled), i.e. it's (0,0,0).
    // Always add worldOffset in the shader to get the true world-space position of the camera.
    filament::math::float3 cameraPosition;

    float time; // time in seconds, with a 1 second period

    filament::math::float4 lightColorIntensity; // directional light

    filament::math::float4 sun; // cos(sunAngle), sin(sunAngle), 1/(sunAngle*HALO_SIZE-sunAngle), HALO_EXP

    filament::math::float4 padding0;

    filament::math::float3 lightDirection;
    uint32_t fParamsX; // stride-x

    filament::math::float3 shadowBias; // unused, normal bias, unused
    float oneOverFroxelDimensionY;

    filament::math::float4 zParams; // froxel Z parameters

    filament::math::uint2 fParams; // stride-y, stride-z
    filament::math::float2 origin; // viewport left, viewport bottom

    float oneOverFroxelDimensionX;
    float iblLuminance;
    float exposure;
    float ev100;

    alignas(16) filament::math::float4 iblSH[9]; // actually float3 entries (std140 requires float4 alignment)

    filament::math::float4 userTime;  // time(s), (double)time - (float)time, 0, 0

    float iblRoughnessOneLevel;       // level for roughness == 1
    float cameraFar;                  // camera *culling* far-plane distance (projection far is at +inf)
    float refractionLodOffset;

    // bit 0: directional (sun) shadow enabled
    // bit 1: directional (sun) screen-space contact shadow enabled
    // bit 8-15: screen-space contact shadows ray casting steps
    uint32_t directionalShadows;

    filament::math::float3 worldOffset; // this is (0,0,0) when camera_at_origin is disabled
    float ssContactShadowDistance;

    // fog
    float fogStart;
    float fogMaxOpacity;
    float fogHeight;
    float fogHeightFalloff;         // falloff * 1.44269
    math::float3 fogColor;
    float fogDensity;               // (density/falloff)*exp(-falloff*(camera.y - fogHeight))
    float fogInscatteringStart;
    float fogInscatteringSize;
    float fogColorFromIbl;

    // bit 0-3: cascade count
    // bit 4: visualize cascades
    // bit 8-11: cascade has visible shadows
    uint32_t cascades;

    float aoSamplingQualityAndEdgeDistance;     // 0: bilinear, !0: bilateral edge distance
    float aoBentNormals;                        // 0: no AO bent normal, >0.0 AO bent normals
    float aoReserved2;
    float aoReserved3;

    math::float2 clipControl;
    math::float2 padding1;

    float vsmExponent;
    float vsmDepthScale;
    float vsmLightBleedReduction;
    float vsmReserved0;

    // bring PerViewUib to 2 KiB
    filament::math::float4 padding2[59];
};

// 2 KiB == 128 float4s
static_assert(sizeof(PerViewUib) == sizeof(filament::math::float4) * 128,
        "PerViewUib should be exactly 2KiB");

// PerRenderableUib must have an alignment of 256 to be compatible with all versions of GLES.
struct alignas(256) PerRenderableUib {
    static constexpr utils::StaticString _name{ "ObjectUniforms" };
    filament::math::mat4f worldFromModelMatrix;
    filament::math::mat3f worldFromModelNormalMatrix; // this gets expanded to 48 bytes during the copy to the UBO
    alignas(16) filament::math::float4 morphWeights;
    // TODO: we can pack all the boolean bellow
    int32_t skinningEnabled; // 0=disabled, 1=enabled, ignored unless variant & SKINNING_OR_MORPHING
    int32_t morphingEnabled; // 0=disabled, 1=enabled, ignored unless variant & SKINNING_OR_MORPHING
    uint32_t screenSpaceContactShadows; // 0=disabled, 1=enabled, ignored unless variant & SKINNING_OR_MORPHING
    // TODO: We need a better solution, this currently holds the average local scale for the renderable
    float userData;
};
static_assert(sizeof(PerRenderableUib) % 256 == 0, "sizeof(Transform) should be a multiple of 256");

struct LightsUib {
    static constexpr utils::StaticString _name{ "LightsUniforms" };
    filament::math::float4 positionFalloff;   // { float3(pos), 1/falloff^2 }
    filament::math::float4 colorIntensity;    // { float3(col), intensity }
    filament::math::float4 directionIES;      // { float3(dir), IES index }
    filament::math::float2 spotScaleOffset;   // { scale, offset }
    uint32_t               shadow;            // { shadow bits (see ShadowInfo) }
    uint32_t               type;              // { 0=point, 1=spot }
};
static_assert(sizeof(LightsUib) == 64, "the actual UBO is an array of 256 mat4");

// UBO for punctual (spot light) shadows.
struct ShadowUib {
    static constexpr utils::StaticString _name{ "ShadowUniforms" };
    filament::math::mat4f spotLightFromWorldMatrix[CONFIG_MAX_SHADOW_CASTING_SPOTS];
    filament::math::float4 directionShadowBias[CONFIG_MAX_SHADOW_CASTING_SPOTS]; // light direction, normal bias
};

// UBO froxel record buffer.
struct FroxelRecordUib {
    static constexpr utils::StaticString _name{ "FroxelRecordUniforms" };
    filament::math::uint4 records[1024];
};

// This is not the UBO proper, but just an element of a bone array.
struct PerRenderableUibBone {
    static constexpr utils::StaticString _name{ "BonesUniforms" };
    filament::math::quatf q = { 1, 0, 0, 0 };
    filament::math::float4 t = {};
    filament::math::float4 s = { 1, 1, 1, 0 };
    filament::math::float4 ns = { 1, 1, 1, 0 };
};

static_assert(CONFIG_MAX_BONE_COUNT * sizeof(PerRenderableUibBone) <= 16384,
        "Bones exceed max UBO size");

} // namespace filament

#endif // TNT_FILABRIDGE_UIBSTRUCTS_H
