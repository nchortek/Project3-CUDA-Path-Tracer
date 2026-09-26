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

bool areEqual(float x, float y)
{
    return abs(x - y) < kFloatEqualityEpsilon;
}

#endif