//****************************************************************************//
// coresubmesh.h                                                              //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <ostream>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include "cal3d/aabox.h"
#include "cal3d/color.h"
#include "cal3d/global.h"
#include "cal3d/memory.h"
#include "cal3d/vector.h"
#include "cal3d/vector4.h"
#include "cal3d/bonetransform.h"

// Whitespce change to attempt a recompile of cal3d headers
CAL3D_PTR(CalCoreMorphTarget);
CAL3D_PTR(CalCoreSkeleton);
class CalQuaternion;

enum CalMorphTargetType {
    CalMorphTargetTypeAdditive,
    CalMorphTargetTypeClamped,
    CalMorphTargetTypeAverage,
    CalMorphTargetTypeExclusive
};

enum SplitMeshBasedOnBoneLimitType {
    SplitMeshBoneLimitOK,
    SplitMeshBoneLimitVtxTrglMismatch,
    SplitMeshBoneLimitEmptyVertices
};

CAL3D_PTR(CalCoreSubmesh);

#define VECTOR_CONTAINS(a, b)       (std::find(a.begin(), a.end(), b)!=a.end())
#define VECTOR_REMOVE_VALUE(a, b)   (a.erase(std::find(a.begin(), a.end(), b)))
#define VECTOR_ADD_UNIQUE(a, b)     do {  if(std::find(a.begin(), a.end(), b)==a.end())  a.push_back(b); } while(0)

// NOTE: Influence extraction could probably be done more efficiently with fixed-length
// arrays, if we're willing to settle on a fixed number of influences.
class CAL3D_API CalWeightsBoneIdsPair {
public:
    std::vector<float> weights;
    std::vector<unsigned int> boneIds;

    bool operator==(const CalWeightsBoneIdsPair& rhs) const {
        return weights == rhs.weights && boneIds == rhs.boneIds;
    }
};

class CAL3D_API CalExportedInfluences {
public:
    unsigned int maximumInfluenceCount;
    std::vector<CalWeightsBoneIdsPair> weightsBoneIdsPairs;
    std::vector<unsigned int> usedBoneIds;
};

class CAL3D_API CalCoreSubmesh {
public:
    // TODO: replace with Vec2f
    struct TextureCoordinate {
        TextureCoordinate()
            : u(0)
            , v(0) 
        {}

        TextureCoordinate(float u, float v)
            : u(u)
            , v(v)
        {}

        float u, v;

        bool operator==(const TextureCoordinate& rhs) const {
            return u == rhs.u && v == rhs.v;
        }
    };

    struct Influence {
        Influence() {
            boneId = static_cast<unsigned>(-1);
            weight = 0.0f;
            lastInfluenceForThisVertex = 0;
        }

        Influence(unsigned b, float w, bool last) {
            boneId = b;
            weight = w;
            lastInfluenceForThisVertex = last ? 1 : 0;
        }

        unsigned boneId;
        float weight;
        unsigned lastInfluenceForThisVertex;

        bool operator==(const Influence& rhs) const {
            return std::make_pair(boneId, quantize(weight)) == std::make_pair(rhs.boneId, quantize(rhs.weight));
        }

        bool operator<(const Influence& rhs) const {
            return std::make_pair(boneId, quantize(weight)) < std::make_pair(rhs.boneId, quantize(rhs.weight));
        }

        // floating point equivalence is tricky
        static int quantize(float f) {
            return static_cast<int>(f * 10000.0);
        }
    };

    struct InfluenceSet {
        InfluenceSet() {
        }

        InfluenceSet(const std::vector<Influence>& vi)
            : influences(vi.begin(), vi.end()) {
        }

        std::set<Influence> influences;

        bool operator==(const InfluenceSet& rhs) const {
            if (influences.size() != rhs.influences.size()) {
                return false;
            }
            std::set<Influence>::const_iterator begin1 = influences.begin();
            std::set<Influence>::const_iterator begin2 = rhs.influences.begin();

            while (begin1 != influences.end()) {
                if (!(*begin1 == *begin2)) {
                    return false;
                }
                ++begin1;
                ++begin2;
            }

            return true;
        }
    };

    struct Triangle;
    struct reduxVertex;

    struct reduxVertex {
    public:
        CalVector       position; // location of point in euclidean space
        float           u, v; // texture coordinates
        int             id;       // place of vertex in original list
        std::vector<reduxVertex *>   neighbor; // adjacent vertices
        std::vector<Triangle *> face;     // adjacent triangles
        float            objdist;  // cached cost of collapsing edge
        reduxVertex *         collapse; // candidate vertex for collapse
        std::vector<reduxVertex *> &parent_vertices;
        bool isBorder(); // test if vertex is on border
        reduxVertex(std::vector<reduxVertex *> &vertices, const CalPoint4 &p, int _id);
        reduxVertex(std::vector<reduxVertex *> &vertices, const CalPoint4 &p, float _u, float _v, int _id);
        ~reduxVertex();
        void             RemoveIfNonNeighbor(reduxVertex *n);
    };

    struct Triangle {
    public:
        reduxVertex *         vertex[3]; // the 3 points that make this tri
        CalVector        normal;    // unit vector othogonal to this face
        CalVector        T;    // vector tangent (delta u)
        CalVector        B;    // vector bitangent (delta v)
        std::vector<reduxVertex *> &parent_vertices;
        std::vector<Triangle *> &parent_triangles;
        Triangle(std::vector<reduxVertex *> &vertices, std::vector<Triangle *> &triangles, reduxVertex *v0, reduxVertex *v1, reduxVertex *v2);
        ~Triangle();
        void             ComputeNormal();
        void             ComputeTB();
        void             ReplaceVertex(reduxVertex *vold, reduxVertex *vnew);
        int              HasVertex(reduxVertex *v);
    };

    
    CAL3D_ALIGN_HEAD(16)
    struct Vertex {
        CalPoint4 position;
        CalVector4 normal;

        bool operator==(const Vertex& rhs) const {
            return position == rhs.position && normal == rhs.normal;
        }
    }
    CAL3D_ALIGN_TAIL(16);

    struct Face {
        Face(){}
        Face(CalIndex v0, CalIndex v1, CalIndex v2){vertexId[0]=v0; vertexId[1]= v1; vertexId[2]=v2;}
        CalIndex vertexId[3];

        bool operator==(const Face& rhs) const {
            return std::equal(vertexId, vertexId + 3, rhs.vertexId);
        }
    };

    typedef std::vector<CalCoreMorphTargetPtr> MorphTargetArray;
    typedef std::vector<boost::shared_ptr<CalCoreSubmesh>> CalCoreSubmeshPtrVector;
    typedef std::vector<Face> VectorFace;
    typedef std::vector<TextureCoordinate> VectorTextureCoordinate;
    typedef cal3d::SSEArray<Vertex> VectorVertex;
    typedef std::vector<Influence> InfluenceVector;
    typedef std::set<int> VerticesSet;

    CalCoreSubmesh(int vertexCount, bool hasTextureCoordinates, int faceCount);

    size_t sizeInBytes() const;

    int coreMaterialThreadId;

    const VectorFace& getFaces() const {
        return m_faces;
    }

    size_t getFaceCount() const {
        return m_faces.size();
    }

    size_t getMinimumVertexBufferSize() const {
        return m_minimumVertexBufferSize;
    }
    void addFace(const Face&);

    bool hasTextureCoordinates() const {
        return !m_textureCoordinates.empty();
    }
    bool validateSubmesh() const;
    bool hasTextureCoordinatesOutside0_1() const;
    const VectorTextureCoordinate& getTextureCoordinates() const {
        return m_textureCoordinates;
    }

    const VectorVertex& getVectorVertex() const {
        return m_vertices;
    }

    const std::vector<CalColor32>& getVertexColors() const {
        return m_vertexColors;
    }

    size_t getVertexCount() const {
        return m_vertices.size();
    }

    void addVertex(const Vertex& vertex, CalColor32 vertexColor, const InfluenceVector& influences);
    void setTextureCoordinate(int vertexId, const TextureCoordinate& textureCoordinate);

    void addMorphTarget(const CalCoreMorphTargetPtr& morphTarget);
    const MorphTargetArray& getMorphTargets() const {
        return m_morphTargets;
    }
    
    void replaceMeshWithMorphTarget(const std::string& morphTargetName);

    void scale(float factor);
    void fixup(const CalCoreSkeletonPtr& skeleton);

    bool isStatic() const;
    BoneTransform getStaticTransform(const BoneTransform* bones) const;

    const InfluenceVector& getInfluences() const {
        return m_influences;
    }

    CalAABox getBoundingVolume() const {
        return m_boundingVolume;
    }
    
    void duplicateTriangles();
    void sortTris(CalCoreSubmesh&);
    bool simplifySubmesh(unsigned int tri_count, unsigned int quality);

    boost::shared_ptr<CalCoreSubmesh> emitSubmesh(VerticesSet & verticesSetThisSplit, VectorFace & trianglesThisSplit, SplitMeshBasedOnBoneLimitType& rc);

    SplitMeshBasedOnBoneLimitType splitMeshBasedOnBoneLimit(CalCoreSubmeshPtrVector& newSubmeshes, size_t boneLimit);

    void optimizeVertexCache();
    void optimizeVertexCacheSubset(unsigned int faceStartIndex, unsigned int faceCount);
    void renumberIndices();
    void normalizeNormals();
    void sortForBlending();

    CalExportedInfluences exportInfluences(unsigned int influenceLimit);

private:
    unsigned int m_currentVertexId;
    int m_isolateds;
    std::vector<reduxVertex *> vertices;
    std::vector<Triangle *> triangles;


    // The following arrays should always be the same size.
    VectorVertex m_vertices;

    std::vector<CalColor32> m_vertexColors;
 
    VectorTextureCoordinate m_textureCoordinates;

    MorphTargetArray m_morphTargets;

    bool m_isStatic;
    InfluenceSet m_staticInfluenceSet;

    InfluenceVector m_influences;
    CalAABox m_boundingVolume;

    VectorFace m_faces;
    size_t m_minimumVertexBufferSize;

    void addVertices(CalCoreSubmesh& submeshTo, unsigned submeshToVertexOffset, float normalMul);

    // internal simplification prototypes
    float ComputeEdgeCollapseCost(reduxVertex *u, reduxVertex *v);
    bool ComputeEdgeCostAtVertex(reduxVertex *v);
    bool ComputeAllEdgeCollapseCosts();
    void Collapse(reduxVertex *u, reduxVertex *v);
    void reduxAddFaces(const VectorFace &tri);
    void reduxAddVertices(const VectorVertex &vert, const VectorTextureCoordinate &tex);
    void ApplyFaces(const VectorFace &tris);
    bool matchFaces(Triangle *t0, Triangle *t1);
    reduxVertex *MinimumCostEdge();
};

inline std::ostream& operator<<(std::ostream& os, const CalCoreSubmesh::Influence& influence) {
    return os << "CalCoreSubmesh::Influence(" << influence.boneId << ", " << influence.weight << ", " << influence.lastInfluenceForThisVertex << ")";
}

CAL3D_API CalCoreSubmeshPtr MakeCube();
