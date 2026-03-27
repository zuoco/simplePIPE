#include "geometry/ShapeMesher.h"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include <cmath>

namespace geometry {

MeshData ShapeMesher::mesh(const TopoDS_Shape& shape, double deflection) {
    // 三角化
    BRepMesh_IncrementalMesh mesher(shape, deflection, /*isRelative=*/false,
                                     /*theAngle=*/0.5, /*isParallel=*/false);
    mesher.Perform();

    MeshData result;

    for (TopExp_Explorer exp(shape, TopAbs_FACE); exp.More(); exp.Next()) {
        const TopoDS_Face& face = TopoDS::Face(exp.Current());
        TopLoc_Location    loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull() || tri->NbTriangles() == 0) continue;

        const bool    reversed = (face.Orientation() == TopAbs_REVERSED);
        const gp_Trsf trsf     = loc.Transformation();
        const bool    hasLoc   = !loc.IsIdentity();

        // 顶点基准偏移（相对于当前 result.vertices 末尾）
        const uint32_t baseIdx = static_cast<uint32_t>(result.vertices.size());

        // 提取节点（应用 location 变换）
        for (int i = 1; i <= tri->NbNodes(); ++i) {
            gp_Pnt p = tri->Node(i);
            if (hasLoc) p.Transform(trsf);
            result.vertices.push_back({static_cast<float>(p.X()),
                                       static_cast<float>(p.Y()),
                                       static_cast<float>(p.Z())});
            result.normals.push_back({0.f, 0.f, 0.f}); // 先占位，后填充
        }

        // 提取三角面：计算平面法线并填充
        for (int t = 1; t <= tri->NbTriangles(); ++t) {
            int n1, n2, n3;
            tri->Triangle(t).Get(n1, n2, n3);
            if (reversed) std::swap(n2, n3); // 反转法线方向

            // 计算面法线（cross product）
            gp_Pnt pA = tri->Node(n1);
            gp_Pnt pB = tri->Node(n2);
            gp_Pnt pC = tri->Node(n3);
            if (hasLoc) {
                pA.Transform(trsf);
                pB.Transform(trsf);
                pC.Transform(trsf);
            }

            gp_Vec u(pA, pB);
            gp_Vec v(pA, pC);
            gp_Vec normal = u.Crossed(v);
            double len    = normal.Magnitude();
            if (len > 1e-12) normal /= len;

            auto na = std::array<float, 3>{static_cast<float>(normal.X()),
                                           static_cast<float>(normal.Y()),
                                           static_cast<float>(normal.Z())};
            // 将法线累加到顶点（平均法线）
            for (int vi : {n1, n2, n3}) {
                auto& vn = result.normals[baseIdx + vi - 1];
                vn[0] += na[0];
                vn[1] += na[1];
                vn[2] += na[2];
            }

            // 追加索引（0-based）
            result.indices.push_back(baseIdx + n1 - 1);
            result.indices.push_back(baseIdx + n2 - 1);
            result.indices.push_back(baseIdx + n3 - 1);
        }
    }

    // 归一化法线
    for (auto& n : result.normals) {
        float len = std::sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
        if (len > 1e-12f) {
            n[0] /= len;
            n[1] /= len;
            n[2] /= len;
        }
    }

    return result;
}

} // namespace geometry
