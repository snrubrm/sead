#pragma once

#include <math/seadVector.h>

namespace sead
{
/// The plane dot(normal, p) == d.
template <typename T>
class Plane3
{
public:
    Plane3() = default;
    Plane3(const Vector3<T>& normal, T d) : mNormal(normal), mD(d) {}

    const Vector3<T>& getNormal() const { return mNormal; }
    T getD() const { return mD; }

private:
    Vector3<T> mNormal;
    T mD;
};

using Plane3f = Plane3<f32>;

}  // namespace sead
