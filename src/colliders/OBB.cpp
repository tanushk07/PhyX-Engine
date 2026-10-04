#include "OBB.h"
#include <algorithm>
#include <cfloat>

namespace
{
    // Sutherland-Hodgman, one plane: keep the part of polygon `in` where dot(p, nrm) <= off.
    int ClipPolygon(const Vec3* in, int count, const Vec3& nrm, float off, Vec3* out)
    {
        int n = 0;
        for (int i = 0; i < count; ++i)
        {
            const Vec3& a = in[i];
            const Vec3& b = in[(i + 1) % count];
            const float da = a.dot(nrm) - off;                     // <= 0 means inside
            const float db = b.dot(nrm) - off;
            if (da <= 0.f) out[n++] = a;                           // a is inside: keep it
            if ((da < 0.f && db > 0.f) || (da > 0.f && db < 0.f))  // edge a->b crosses the plane: keep the crossing
                out[n++] = a + (b - a) * (da / (da - db));
        }
        return n;
    }
}

void OBB::Intersect(const OBB& other, IntersectionData& Data) const
{
    const OBB& A = *this;
    const OBB& B = other;
    const Vec3 d = B.Position - A.Position;                        // centre to centre, A -> B
    Data.hasCollided = false;

    // ---- 1. SAT over 15 axes. Overlap on an axis = (radius A + radius B) - centre distance on it.
    //         Negative anywhere = a gap = separated. Keep the smallest overlap: that axis is the normal.
    float faceDepth = FLT_MAX; Vec3 faceAxis; int faceIndex = -1;  // 0..2 = A's faces, 3..5 = B's faces
    float edgeDepth = FLT_MAX; Vec3 edgeAxis; int edgeI = -1, edgeJ = -1;

    auto overlapOn = [&](const Vec3& L) { return A.ProjectedRadius(L) + B.ProjectedRadius(L) - fabsf(d.dot(L)); };

    for (int i = 0; i < 3; ++i)                                    // 6 face axes
    {
        const float oa = overlapOn(A.Axes[i]);
        if (oa < 0.f) return;
        if (oa < faceDepth) { faceDepth = oa; faceAxis = A.Axes[i]; faceIndex = i; }

        const float ob = overlapOn(B.Axes[i]);
        if (ob < 0.f) return;
        if (ob < faceDepth) { faceDepth = ob; faceAxis = B.Axes[i]; faceIndex = 3 + i; }
    }
    for (int i = 0; i < 3; ++i)                                    // 9 edge-pair axes
    for (int j = 0; j < 3; ++j)
    {
        Vec3 L = A.Axes[i].cross(B.Axes[j]);
        const float len = L.length();
        if (len < 1e-4f) continue;                                 // parallel edges: no new axis, and dividing would blow up
        L = L / len;                                               // unit, so the overlap is a real distance
        const float o = overlapOn(L);
        if (o < 0.f) return;
        if (o < edgeDepth) { edgeDepth = o; edgeAxis = L; edgeI = i; edgeJ = j; }
    }

    // ---- 2. Edge against edge. Only if clearly shallower than the best face: face contacts are far more stable.
    if (edgeDepth * 1.05f + 0.01f < faceDepth)
    {
        Vec3 n = edgeAxis;
        if (n.dot(d) < 0.f) n = -n;                                // A -> B

        // A's edge along Axes[edgeI] that reaches furthest toward B (through its midpoint)
        Vec3 pA = A.Position;
        for (int k = 0; k < 3; ++k)
            if (k != edgeI) pA += A.Axes[k] * (A.Axes[k].dot(n) > 0.f ? A.HalfExtent(k) : -A.HalfExtent(k));
        // B's edge along Axes[edgeJ] that reaches furthest toward A
        Vec3 pB = B.Position;
        for (int k = 0; k < 3; ++k)
            if (k != edgeJ) pB += B.Axes[k] * (B.Axes[k].dot(n) > 0.f ? -B.HalfExtent(k) : B.HalfExtent(k));

        // closest points between the lines  pA + s*uA  and  pB + t*uB
        const Vec3& uA = A.Axes[edgeI];
        const Vec3& uB = B.Axes[edgeJ];
        const Vec3 w = pA - pB;
        const float b  = uA.dot(uB), dA = uA.dot(w), dB = uB.dot(w);
        const float denom = 1.f - b * b;                           // not ~0: parallel pairs were skipped above
        float s = (b * dB - dA) / denom;
        float t = (dB - b * dA) / denom;
        s = std::max(-A.HalfExtent(edgeI), std::min(s, A.HalfExtent(edgeI)));   // stay on the actual edges
        t = std::max(-B.HalfExtent(edgeJ), std::min(t, B.HalfExtent(edgeJ)));

        Data.hasCollided        = true;
        Data.IntersectionNormal = n;
        Data.AddPoint((pA + uA * s + pB + uB * t) * 0.5f, edgeDepth);
        return;
    }

    // ---- 3. Face contact. Reference face = the face whose axis won. Incident face = the other box's
    //         face that points most directly back at it. Clip incident to the reference face's outline.
    const bool refIsA = faceIndex < 3;
    const OBB& ref = refIsA ? A : B;
    const OBB& inc = refIsA ? B : A;
    const int  r   = faceIndex % 3;

    Vec3 n = faceAxis;
    if (n.dot(d) < 0.f) n = -n;                                    // A -> B: what gets stored
    const Vec3 refN      = refIsA ? n : -n;                        // reference face's outward normal, toward inc
    const Vec3 refCentre = ref.Position + refN * ref.HalfExtent(r);

    int k = 0; float most = -1.f;
    for (int a = 0; a < 3; ++a)
    {
        const float dd = fabsf(inc.Axes[a].dot(refN));
        if (dd > most) { most = dd; k = a; }
    }
    const Vec3 incN      = inc.Axes[k] * (inc.Axes[k].dot(refN) > 0.f ? -1.f : 1.f);   // points back at ref
    const Vec3 incCentre = inc.Position + incN * inc.HalfExtent(k);
    const Vec3 e1 = inc.Axes[(k + 1) % 3] * inc.HalfExtent((k + 1) % 3);
    const Vec3 e2 = inc.Axes[(k + 2) % 3] * inc.HalfExtent((k + 2) % 3);

    Vec3 poly[16] = { incCentre + e1 + e2, incCentre - e1 + e2, incCentre - e1 - e2, incCentre + e1 - e2 };  // in order around the face
    Vec3 tmp[16];
    int count = 4;

    const int r1 = (r + 1) % 3, r2 = (r + 2) % 3;
    const Vec3  sides[4]  = { ref.Axes[r1], -ref.Axes[r1], ref.Axes[r2], -ref.Axes[r2] };
    const float limits[4] = { ref.HalfExtent(r1), ref.HalfExtent(r1), ref.HalfExtent(r2), ref.HalfExtent(r2) };
    for (int s = 0; s < 4 && count > 0; ++s)                       // the 4 side walls of the reference face
    {
        count = ClipPolygon(poly, count, sides[s], sides[s].dot(ref.Position) + limits[s], tmp);
        std::copy(tmp, tmp + count, poly);
    }

    ContactPointData found[16];
    int nFound = 0;
    for (int i = 0; i < count; ++i)
    {
        const float sep = (poly[i] - refCentre).dot(refN);         // < 0: below the reference face, i.e. touching
        if (sep < 0.f) found[nFound++] = { poly[i] - refN * (sep * 0.5f), -sep };   // halfway between the faces
    }
    if (nFound == 0) return;

    Data.hasCollided        = true;
    Data.IntersectionNormal = n;
    if (nFound <= 4)
    {
        for (int i = 0; i < nFound; ++i) Data.AddPoint(found[i].IntersectionPoint, found[i].IntersectionDepth);
        return;
    }

    // ---- 4. More than 4 (clipping can give up to 8). Keep 4 that span the area, not 4 bunched on one side:
    //         the deepest, the one farthest from it, then the farthest on each side of the line between them.
    int keep[4] = { 0, 0, 0, 0 };
    for (int i = 1; i < nFound; ++i)
        if (found[i].IntersectionDepth > found[keep[0]].IntersectionDepth) keep[0] = i;
    const Vec3 p0 = found[keep[0]].IntersectionPoint;
    float farthest = -1.f;
    for (int i = 0; i < nFound; ++i)
    {
        const float d2 = (found[i].IntersectionPoint - p0).lengthsqr();
        if (d2 > farthest) { farthest = d2; keep[1] = i; }
    }
    const Vec3 line = found[keep[1]].IntersectionPoint - p0;
    float hi = 0.f, lo = 0.f;
    keep[2] = keep[3] = -1;
    for (int i = 0; i < nFound; ++i)
    {
        const float side = line.cross(found[i].IntersectionPoint - p0).dot(refN);  // + one side, - the other
        if (side > hi) { hi = side; keep[2] = i; }
        if (side < lo) { lo = side; keep[3] = i; }
    }
    for (int i : keep)
        if (i >= 0) Data.AddPoint(found[i].IntersectionPoint, found[i].IntersectionDepth);
}



void OBB::Intersect(const BoundingSphere& other, IntersectionData& Data) const
{
    // 1. Into the box's frame. The sphere centre is a POINT: subtract the centre, then un-rotate.
    const Vec3 localCentre = PointToLocal(other.getPosition());

    // 2. Here the box is just an AABB centred at the origin. Run the test you already trust.
    const AABB localBox{ -HalfExtents, HalfExtents };
    IntersectionData local;
    localBox.Intersect(BoundingSphere{ localCentre, other.getRadius() }, local);
    Data.hasCollided = local.hasCollided;
    if (!local.hasCollided) { return; }
    
    // 3. Back to the world. Normal is a DIRECTION (rotate only); point is a POINT (rotate + move).
    Data.IntersectionNormal = DirToWorld(local.IntersectionNormal);
    for (int i = 0; i < local.Count; ++i)
    {
        Data.AddPoint(PointToWorld(local.Points[i].IntersectionPoint), local.Points[i].IntersectionDepth);
    }
}
void OBB::Intersect(const AABB& other, IntersectionData& Data) const
{
    const Vec3 c = (other.getMinExtend() + other.getMaxExtend()) * 0.5f;
    const Vec3 h = (other.getMaxExtend() - other.getMinExtend()) * 0.5f;
    Intersect(OBB{ c, Quat{}, h }, Data);
}