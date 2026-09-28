#ifndef MATH_GLSL
#define MATH_GLSL

// Numeric constants
const float kPi = 3.14159265358979323;
const float kTwoPi = 6.28318530717958648;
const float kFourPi = 12.5663706143591729;
const float kInvPi = 0.31830988618379067;
const float kInvTwoPi = 0.15915494309;
const float kInvFourPi = 0.07957747154594767;
const float kHalfPi = 1.57079632679489662;
const float kOneThird = 0.33333333333333333;
const float kE = 2.71828182845904524;
const float kOneMinusEpsilon = 0.99999994;
const float kFloatEqualityEpsilon = 0.000005;

// Data structures
struct Ray {
    vec3 origin;
    vec3 direction;
};

float absDot(vec3 a, vec3 b)
{
    return abs(dot(a, b));
}

float cosTheta(vec3 w)
{
    return w.z;
}

float cos2Theta(vec3 w)
{ 
    return w.z * w.z;

}
float absCosTheta(vec3 w)
{ 
    return abs(w.z);
}

float sin2Theta(vec3 w)
{
    return max(0.f, 1.f - cos2Theta(w));
}
float sinTheta(vec3 w)
{ 
    return sqrt(sin2Theta(w));
}

float tanTheta(vec3 w)
{ 
    return sinTheta(w) / cosTheta(w);
}

float tan2Theta(vec3 w)
{
    return sin2Theta(w) / cos2Theta(w);
}

float cosPhi(vec3 w)
{
    float st = sinTheta(w);
    return (st == 0) ? 1 : clamp(w.x / st, -1.f, 1.f);
}

float sinPhi(vec3 w)
{
    float st = sinTheta(w);
    return (st == 0) ? 0 : clamp(w.y / st, -1.f, 1.f);
}

float cos2Phi(vec3 w)
{ 
    return cosPhi(w) * cosPhi(w);
}

float sin2Phi(vec3 w)
{ 
    return sinPhi(w) * sinPhi(w);
}

Ray spawnRay(vec3 pos, vec3 wi)
{
    return Ray(pos + wi * 0.0001, wi);
}

bool refractDir(vec3 wi, vec3 n, float eta, out vec3 wt)
{
    // Compute cos theta using Snell's law
    float cosThetaI = dot(n, wi);
    float sin2ThetaI = max(float(0), float(1 - cosThetaI * cosThetaI));
    float sin2ThetaT = eta * eta * sin2ThetaI;

    // Handle total internal reflection for transmission
    if (sin2ThetaT >= 1)
    {
        return false;
    }

    float cosThetaT = sqrt(1 - sin2ThetaT);
    wt = eta * -wi + (eta * cosThetaI - cosThetaT) * n;
    return true;
}

vec3 faceForward(vec3 n, vec3 v)
{
    return (dot(n, v) < 0.f) ? -n : n;
}

bool sameHemisphere(vec3 w, vec3 wp)
{
    return w.z * wp.z > 0;
}

void coordinateSystem(vec3 v1, out vec3 v2, out vec3 v3)
{
    if (abs(v1.x) > abs(v1.y))
        v2 = vec3(-v1.z, 0, v1.x) / sqrt(v1.x * v1.x + v1.z * v1.z);
    else
        v2 = vec3(0, v1.z, -v1.y) / sqrt(v1.y * v1.y + v1.z * v1.z);
    v3 = cross(v1, v2);
}

mat3 localToWorld(vec3 nor)
{
    vec3 tan, bit;
    coordinateSystem(nor, tan, bit);
    return mat3(tan, bit, nor);
}

mat3 worldToLocal(vec3 nor)
{
    return transpose(localToWorld(nor));
}

float distanceSquared(vec3 p1, vec3 p2)
{
    return dot(p1 - p2, p1 - p2);
}

bool areEqual(float x, float y)
{
    return abs(x - y) < kFloatEqualityEpsilon;
}

vec3 getPointOnRay(Ray ray, float t)
{
    return ray.origin + t * ray.direction;
}

#endif