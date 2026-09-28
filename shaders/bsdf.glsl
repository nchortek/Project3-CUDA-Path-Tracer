#ifndef BSDF_GLSL
#define BSDF_GLSL

#include "gpu/shared.h"
#include "common.glsl"
#include "rng.glsl"
#include "math.glsl"
#include "sampleWarping.glsl"

vec3 f_diffuse(vec3 albedo)
{
    return albedo * kInvPi;
}

vec3 sample_f_diffuse(vec3 albedo, vec2 xi, vec3 nor,
    out vec3 wiW, out float diffusePdf, out uint sampledType)
{
    // Make sure we set wiW to a world-space ray direction,
    // since wo is in tangent space.
    vec3 wiL = squareToHemisphereCosine(xi);
    diffusePdf = squareToHemisphereCosinePDF(wiL);
    wiW = localToWorld(nor) * wiL;
    sampledType = kDiffuseRefl;
    return f_diffuse(albedo);
}

vec3 sample_f_specular_refl(vec3 albedo, vec3 nor, vec3 wo,
    out vec3 wiW, out uint sampledType)
{
    // Make sure we set wiW to a world-space ray direction,
    // since wo is in tangent space
    vec3 wiL = vec3(-1.f * wo.x, -1.f * wo.y, wo.z);
    wiW = localToWorld(nor) * wiL;
    sampledType = kSpecRefl;
    return albedo / abs(wiL.z);
}

vec3 sample_f_specular_trans(vec3 albedo, vec3 nor, vec3 wo,
    out vec3 wiW, out uint sampledType)
{
    wiW = vec3(0.f);
    sampledType = kSpecTrans;

    // Hard-coded to index of refraction of glass
    float etaA = 1.;
    float etaB = 1.55;

    // wo.z == dot(wo, vec3(0.f, 0.f, 1.f))
    bool isExitingSurface = wo.z < 0.f;

    vec3 refractNor = isExitingSurface ? vec3(0.f, 0.f, -1.f) : vec3(0.f, 0.f, 1.f);
    float relativeEta = isExitingSurface ? etaB / etaA : etaA / etaB;

    // Make sure we set wiW to a world-space ray direction,
    // since wo is in tangent space
    vec3 wiL = vec3(0.f);
    bool totalInternalReflection = !refractDir(normalize(wo), refractNor, relativeEta, wiL);

    if (totalInternalReflection)
    {
        return vec3(0.f);
    }

    wiW = localToWorld(nor) * wiL;
    sampledType = kSpecTrans;
    return albedo / abs(wiL.z);
}

vec3 fresnelDielectricEval(float cosThetaI)
{
    // We will hard-code the indices of refraction to be
    // those of glass
    float etaI = 1.;
    float etaT = 1.55;
    cosThetaI = clamp(cosThetaI, -1.f, 1.f);

    bool entering = cosThetaI > 0.f;

    if (!entering)
    {
        float temp = etaI;
        etaI = etaT;
        etaT = temp;
        cosThetaI = abs(cosThetaI);
    }

    float sinThetaI = sqrt(max(0.f, 1.f - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;

    float cosThetaT = sqrt(max(0.f, 1.f - sinThetaT * sinThetaT));

    float rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
    float rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) / ((etaI * cosThetaI) + (etaT * cosThetaT));
    float eval = (rparl * rparl + rperp * rperp) / 2.f;

    return vec3(eval);
}

vec3 sample_f_glass(vec3 albedo, vec3 nor, vec2 xi, vec3 wo,
    out vec3 wiW, out uint sampledType)
{
    float random = rng();
    if (random < 0.5)
    {
        // Have to double contribution b/c we only sample
        // reflection BxDF half the time
        vec3 R = sample_f_specular_refl(albedo, nor, wo, wiW, sampledType);
        sampledType = kSpecRefl;
        return 2 * fresnelDielectricEval(dot(nor, normalize(wiW))) * R;
    }
    else
    {
        // Have to double contribution b/c we only sample
        // transmit BxDF half the time
        vec3 T = sample_f_specular_trans(albedo, nor, wo, wiW, sampledType);
        sampledType = kSpecTrans;
        return 2 * (vec3(1.) - fresnelDielectricEval(dot(nor, normalize(wiW)))) * T;
    }
}

// These are used for microfacet reflections
vec3 sample_wh(vec3 wo, vec2 xi, float roughness)
{
    vec3 wh;

    float ct = 0;
    float phi = kTwoPi * xi[1];
    // We'll only handle isotropic microfacet materials
    float tanTheta2 = roughness * roughness * xi[0] / (1.0f - xi[0]);
    ct = 1 / sqrt(1 + tanTheta2);

    float sinTheta =
        sqrt(max(0.f, 1.f - ct * ct));

    wh = vec3(sinTheta * cos(phi), sinTheta * sin(phi), ct);
    if (!sameHemisphere(wo, wh))
    {
        wh = -wh;
    }

    return wh;
}

float trowbridgeReitzD(vec3 wh, float roughness)
{
    float t2 = tan2Theta(wh);
    if (isinf(t2)) return 0.f;

    float cos4Theta = cos2Theta(wh) * cos2Theta(wh);

    float e =
        (cos2Phi(wh) / (roughness * roughness)
            + sin2Phi(wh) / (roughness * roughness))
        * t2;

    return 1 / (kPi * roughness * roughness * cos4Theta * (1 + e) * (1 + e));
}

float lambda(vec3 w, float roughness)
{
    float absTanTheta = abs(tanTheta(w));
    if (isinf(absTanTheta)) return 0.;

    // Compute alpha for direction w
    float alpha =
        sqrt(cos2Phi(w) * roughness * roughness + sin2Phi(w) * roughness * roughness);
    float alpha2Tan2Theta = (roughness * absTanTheta) * (roughness * absTanTheta);
    return (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2;
}

float trowbridgeReitzG(vec3 wo, vec3 wi, float roughness)
{
    return 1 / (1 + lambda(wo, roughness) + lambda(wi, roughness));
}

float trowbridgeReitzPdf(vec3 wo, vec3 wh, float roughness)
{
    return trowbridgeReitzD(wh, roughness) * absCosTheta(wh);
}

vec3 f_microfacet_refl(vec3 albedo, vec3 wo, vec3 wi, float roughness)
{
    float cosThetaO = absCosTheta(wo);
    float cosThetaI = absCosTheta(wi);
    vec3 wh = wi + wo;

    // Handle degenerate cases for microfacet reflection
    if (cosThetaI == 0 || cosThetaO == 0)
    {
        return vec3(0.f);
    }

    if (wh.x == 0 && wh.y == 0 && wh.z == 0)
    {
        return vec3(0.f);
    }

    wh = normalize(wh);

    // Handle different Fresnel coefficients
    vec3 F = vec3(1.);//fresnel->Evaluate(glm::dot(wi, wh));
    float D = trowbridgeReitzD(wh, roughness);
    float G = trowbridgeReitzG(wo, wi, roughness);
    return albedo * D * G * F /
        (4 * cosThetaI * cosThetaO);
}

vec3 sample_f_microfacet_refl(vec3 albedo, vec3 nor, vec2 xi, vec3 wo, float roughness,
    out vec3 wiW, out float microfacetPdf, out uint sampledType)
{
    wiW = vec3(0.f);
    microfacetPdf = 0.f;
    sampledType = kMicrofacetRefl;

    if (wo.z == 0)
    {
        return vec3(0.f);
    }

    vec3 wh = sample_wh(wo, xi, roughness);
    vec3 wi = reflect(-wo, wh);
    wiW = localToWorld(nor) * wi;
    if (!sameHemisphere(wo, wi)) return vec3(0.f);

    // Compute PDF of _wi_ for microfacet reflection
    microfacetPdf = trowbridgeReitzPdf(wo, wh, roughness) / (4 * dot(wo, wh));
    return f_microfacet_refl(albedo, wo, wi, roughness);
}

vec3 computeAlbedo(SurfaceInteraction surface)
{
    vec3 albedo = surface.material.baseColor;
    return albedo;
}

vec3 computeNormal(SurfaceInteraction surface)
{
    vec3 nor = surface.isect.worldShadingNor;
    return nor;
}

float computeRoughness(SurfaceInteraction surface)
{
    float roughness = surface.material.roughness;
    return roughness;
}

// Computes the overall light scattering properties of a point on a Material,
// given the incoming and outgoing light directions.
vec3 f(SurfaceInteraction surface, vec3 woW, vec3 wiW)
{
    // Convert the incoming and outgoing light rays from
    // world space to local tangent space
    vec3 nor = computeNormal(surface);
    vec3 wo = worldToLocal(nor) * woW;
    vec3 wi = worldToLocal(nor) * wiW;

    // If the outgoing ray is parallel to the surface,
    // we know we can return black b/c the Lambert term
    // in the overall Light Transport Equation will be 0.
    if (wo.z == 0) return vec3(0.f);

    // Since GLSL does not support classes or polymorphism,
    // we have to handle each material type with its own function.
    if (surface.material.type == kDiffuseRefl)
    {
        return f_diffuse(computeAlbedo(surface));
    }

    // There is a 0% chance that a randomly chosen wi will be the perfect
    // reflection / refraction of wo, so any specular material will have
    // a BSDF of 0 when wi is chosen independently of the material.
    else if (surface.material.type == kSpecRefl
        || surface.material.type == kSpecTrans
        || surface.material.type == kSpecGlass)
    {
        return vec3(0.);
    }
    // NCHORTEK TODO: Replace with my 5610 hw8/9 shader code
    /*
    else if (surface.material.type == kMicrofacetRefl)
    {
        return f_microfacet_refl(computeAlbedo(surface),
            wo, wi,
            computeRoughness(surface));
    }
    */
    // Default case, unhandled material
    else
    {
        return vec3(1, 0, 1);
    }
}

// Sample_f() returns the same values as f(), but importantly it
// only takes in a wo. Note that wiW is declared as an "out vec3";
// this means the function is intended to compute and write a wi
// in world space (the trailing "W" indicates world space).
// In other words, Sample_f() evaluates the BSDF *after* generating
// a wi based on the SurfaceInteraction's material properties, allowing
// us to bias our wi samples in a way that gives more consistent
// light scattered along wo.
vec3 sample_f(SurfaceInteraction surface, vec3 woW, vec2 xi,
    out vec3 wiW, out float bsdfPdf, out uint sampledType)
{
    wiW = vec3(0.f);
    bsdfPdf = 0.f;
    sampledType = 0u;

    // Convert wo to local space from world space.
    // The various Sample_f()s output a wi in world space,
    // but assume wo is in local space.
    vec3 nor = computeNormal(surface);
    vec3 wo = worldToLocal(nor) * woW;

    if (surface.material.type == kDiffuseRefl)
    {
        return sample_f_diffuse(computeAlbedo(surface), xi, nor, wiW, bsdfPdf, sampledType);
    }
    else if (surface.material.type == kSpecRefl)
    {
        bsdfPdf = 1.;
        return sample_f_specular_refl(computeAlbedo(surface), nor, wo, wiW, sampledType);
    }
    else if (surface.material.type == kSpecTrans)
    {
        bsdfPdf = 1.;
        return sample_f_specular_trans(computeAlbedo(surface), nor, wo, wiW, sampledType);
    }
    else if (surface.material.type == kSpecGlass)
    {
        bsdfPdf = 1.;
        return sample_f_glass(computeAlbedo(surface), nor, xi, wo, wiW, sampledType);
    }
    // NCHORTEK TODO: Replace with my 5610 hw8/9 shader code
    /*
    else if (surface.material.type == kMicrofacetRefl)
    {
        return sample_f_microfacet_refl(computeAlbedo(surface),
            nor, xi, wo,
            computeRoughness(surface),
            wiW, bsdfPdf,
            sampledType);
    }
    */
    // Default case, unhandled material
    else
    {
        return vec3(1, 0, 1);
    }
}

// Compute the PDF of wi with respect to wo and the SurfaceInteraction's
// material properties.
float pdf(SurfaceInteraction surface, vec3 woW, vec3 wiW)
{
    vec3 nor = computeNormal(surface);
    vec3 wo = worldToLocal(nor) * woW;
    vec3 wi = worldToLocal(nor) * wiW;

    if (wo.z == 0) return 0.; // The cosine of this vector would be zero

    if (surface.material.type == kDiffuseRefl)
    {
        // Get the PDF of a Lambertian material
        return squareToHemisphereCosinePDF(wi);
    }
    else if (surface.material.type == kSpecRefl
        || surface.material.type == kSpecTrans
        || surface.material.type == kSpecGlass)
    {
        return 0.;
    }
    // NCHORTEK TODO: Replace with my 5610 hw8/9 shader code
    /*
    else if (surface.material.type == kMicrofacetRefl)
    {
        vec3 wh = normalize(wo + wi);
        return trowbridgeReitzPdf(wo, wh, computeRoughness(surface)) / (4 * dot(wo, wh));
    }
    */
    // Default case, unhandled material
    else
    {
        return 0.;
    }
}

#endif