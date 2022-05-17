#include "TestPrologue.h"
#include <cal3d/buffersource.h>
#include <cal3d/coremesh.h>
#include <cal3d/loader.h>
#include <cal3d/saver.h>
#include <cal3d/skeleton.h>
#include <cal3d/submesh.h>
#include <cal3d/physique.h>
#include <cal3d/vector4.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/coremorphtarget.h>

#include <cmath>
#include <limits>

inline std::ostream& operator<<(std::ostream& os, const CalCoreSubmesh::Face& f) {
    return os << '(' << f.vertexId[0] << ", " << f.vertexId[1] << ", " << f.vertexId[2] << ')';
}

const CalColor32 BLACK = 0;

FIXTURE(SubmeshFixture) {
    static CalCoreSubmesh::Vertex makeVertex(int id) {
        CalCoreSubmesh::Vertex v;
        v.position = CalPoint4(id, id, id);
        v.normal = CalVector4(id, id, id);
        return v;
    }

    static CalCoreSubmesh makeMeshWithUnoptimizedVertexCache() {
        CalCoreSubmesh csm(7, true, 3);
        // Faces 0 and 2 share vertices, while face 1 does not.  We expect vertex
        // cache optimization will rearrange the triangles such that faces 0 and 2
        // are together, either preceded or followed by face 1.
        csm.addFace(CalCoreSubmesh::Face(0, 1, 2));
        csm.addFace(CalCoreSubmesh::Face(3, 4, 5));
        csm.addFace(CalCoreSubmesh::Face(2, 1, 6));

        std::vector<CalCoreSubmesh::Influence> inf(1);
        inf[0].weight = 1.0f;
        inf[0].boneId = 0;

        for (size_t i = 0; i < 7; ++i) {
            csm.addVertex(makeVertex(i), 0, inf);
        }

        return csm;
    }

    static CalCoreSubmesh makeSubmeshWithMultipleInfluences(unsigned int influenceCount) {
        CalCoreSubmesh csm(3, false, 1);
        CalCoreSubmesh::Vertex v = makeVertex(0);
        CalCoreSubmesh::InfluenceVector influenceVector;
        float weight = 1.0f / influenceCount;
        for (size_t i = 0 ; i < influenceCount; ++i) {
            bool isLast = (i == influenceCount - 1);
            influenceVector.push_back(CalCoreSubmesh::Influence(i, weight, isLast));
        }
        csm.addVertex(v, 0, influenceVector);
        csm.addVertex(v, 0, influenceVector);
        csm.addVertex(v, 0, influenceVector);
        csm.addFace(CalCoreSubmesh::Face(0, 1, 2));
        return csm;
    }

    static std::vector<CalIndex> getIndices(const CalCoreSubmesh& csm) {
        std::vector<CalIndex> indices;
        const auto& faces = csm.getFaces();
        for (size_t i = 0; i < faces.size(); ++i) {
            for (size_t j = 0; j < 3; ++j) {
                indices.push_back(faces[i].vertexId[j]);
            }
        }
        return indices;
    }

    static size_t getFaceIndexWithVertexIndex(const CalCoreSubmesh& csm, CalIndex vertexIndex) {
        auto indices = getIndices(csm);
        for (size_t i = 0; i < indices.size(); ++i) {
            if (indices[i] == vertexIndex) {
                return (i / 3);
            }
        }
        assert(false);
        return 0;
    }

    static size_t getIndexDistance(size_t index1, size_t index2) {
        // MSVC 2010 considers std::abs() ambiguous without a bunch of casts.
        return static_cast<unsigned int>(std::abs(static_cast<int>(index1) - static_cast<int>(index2)));
    }

    static CalCoreSubmesh::TextureCoordinate makeTextureCoordinate(int i) {
        return CalCoreSubmesh::TextureCoordinate(i, i);
    }
};

TEST_F(SubmeshFixture, duplicate_triangles) {
    CalCoreSubmesh csm(3, 0, 1);
    CalCoreSubmesh::Vertex vertex;
    vertex.position = CalVector4(1, 1, 1, 0);
    vertex.normal = CalVector4(-1, -1 -1, 0);
    CalCoreSubmesh::InfluenceVector iv(1);
    csm.addVertex(vertex, 0, iv);
    csm.addVertex(vertex, 0, iv);
    csm.addVertex(vertex, 0, iv);

    csm.addFace(CalCoreSubmesh::Face(0, 1, 2));

    csm.duplicateTriangles();
    CHECK_EQUAL(2u, csm.getFaceCount());
    CHECK_EQUAL(CalCoreSubmesh::Face(0, 2, 1), csm.getFaces()[1]);
}

TEST_F(SubmeshFixture, saving_and_loading_submesh_with_morph_stores_differences_in_memory_but_absolute_in_file) {
    CalCoreSubmeshPtr csm(new CalCoreSubmesh(1, 0, 0));
    CalCoreSubmesh::Vertex vertex;
    vertex.position = CalVector4(1, 1, 1, 0);
    vertex.normal = CalVector4(-1, -1 -1, 0);
    csm->addVertex(vertex, CalColor32(), CalCoreSubmesh::InfluenceVector());

    CalCoreMorphTarget::VertexOffsetArray vertexOffsets;
    VertexOffset mv;
    mv.vertexId = 0;
    mv.position = CalVector4(2, 2, 2, 0);
    mv.normal = CalVector4(-2, -2, -2, 0);
    vertexOffsets.push_back(mv);
    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("m", 1, vertexOffsets));
    csm->addMorphTarget(morphTarget);

    CalCoreMesh cm;
    cm.submeshes.push_back(csm);

    { // test binary format

        std::ostringstream os;
        CHECK(CalSaver::saveCoreMesh(os, &cm));

        std::string buffer = os.str();
        CalBufferSource source(buffer.c_str(), buffer.size());
        CalCoreMeshPtr loaded = CalLoader::loadCoreMesh(source);
        CHECK(loaded);

        CHECK_EQUAL(1u, loaded->submeshes.size());
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets().size());
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets.size());
        CHECK_EQUAL(CalVector4(2, 2, 2, 0),    loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets[0].position);
        CHECK_EQUAL(CalVector4(-2, -2, -2, 0), loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets[0].normal);
    }

    { // test XML format

        std::ostringstream os;
        CHECK(CalSaver::saveXmlCoreMesh(os, &cm));

        std::string buffer = os.str();
        CalBufferSource source(buffer.c_str(), buffer.size());
        CalCoreMeshPtr loaded = CalLoader::loadCoreMesh(source);
        CHECK(loaded);

        CHECK_EQUAL(1u, loaded->submeshes.size());
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets().size());
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets.size());
        CHECK_EQUAL(CalVector4(2, 2, 2, 0),    loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets[0].position);
        CHECK_EQUAL(CalVector4(-2, -2, -2, 0), loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets[0].normal);
    }
}

TEST_F(SubmeshFixture, not_static_without_vertices) {
    CalCoreSubmesh csm(0, 0, 0);
    CHECK(!csm.isStatic());
}

TEST_F(SubmeshFixture, is_static_if_all_vertices_are_influenced_by_same_bone) {
    CalCoreSubmesh csm(1, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, BLACK, inf);

    BoneTransform bts[1];
    bts[0].rowx.x = 1.0f;
    bts[0].rowx.y = 0.0f;
    bts[0].rowx.z = 0.0f;
    bts[0].rowx.w = 0.0f;
    bts[0].rowy.x = 0.0f;
    bts[0].rowy.y = 1.0f;
    bts[0].rowy.z = 0.0f;
    bts[0].rowy.w = 0.0f;
    bts[0].rowz.x = 0.0f;
    bts[0].rowz.y = 0.0f;
    bts[0].rowz.z = 1.0f;
    bts[0].rowz.w = 0.0f;

    CHECK(csm.isStatic());
    CHECK_EQUAL(bts[0], csm.getStaticTransform(bts));
}

TEST_F(SubmeshFixture, is_static_if_two_influences) {
    CalCoreSubmesh csm(1, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(2);
    inf[0].boneId = 0;
    inf[0].weight = 0.5f;
    inf[1].boneId = 1;
    inf[1].weight = 0.5f;
    csm.addVertex(v, BLACK, inf);

    BoneTransform bts[2];
    bts[0].rowx.x = 1.0f;
    bts[0].rowx.y = 0.0f;
    bts[0].rowx.z = 0.0f;
    bts[0].rowx.w = 0.0f;
    bts[0].rowy.x = 0.0f;
    bts[0].rowy.y = 1.0f;
    bts[0].rowy.z = 0.0f;
    bts[0].rowy.w = 0.0f;
    bts[0].rowz.x = 0.0f;
    bts[0].rowz.y = 0.0f;
    bts[0].rowz.z = 1.0f;
    bts[0].rowz.w = 0.0f;

    bts[1] = bts[0];
    bts[1].rowx.w = 1.0f;
    bts[1].rowy.w = 1.0f;
    bts[1].rowz.w = 1.0f;

    CHECK(csm.isStatic());
    BoneTransform staticTransform = csm.getStaticTransform(bts);
    CHECK_EQUAL(0.5f, staticTransform.rowx.w);
    CHECK_EQUAL(0.5f, staticTransform.rowy.w);
    CHECK_EQUAL(0.5f, staticTransform.rowz.w);
}

TEST_F(SubmeshFixture, not_static_if_two_vertices_are_influenced_by_different_bones) {
    CalCoreSubmesh csm(2, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, BLACK, inf);
    inf[0].boneId = 1;
    inf[0].weight = 1.0f;
    csm.addVertex(v, BLACK, inf);

    CHECK(!csm.isStatic());
}

TEST_F(SubmeshFixture, is_not_static_if_first_and_third_vertices_have_same_influence) {
    CalCoreSubmesh csm(3, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, BLACK, inf);
    inf[0].boneId = 1;
    inf[0].weight = 1.0f;
    csm.addVertex(v, BLACK, inf);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, BLACK, inf);

    CHECK(!csm.isStatic());
}

TEST_F(SubmeshFixture, is_static_if_two_vertices_have_influences_in_different_order) {
    CalCoreSubmesh csm(2, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(2);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    inf[1].boneId = 1;
    inf[1].weight = 0.0f;
    csm.addVertex(v, BLACK, inf);
    inf[0].boneId = 1;
    inf[0].weight = 0.0f;
    inf[1].boneId = 0;
    inf[1].weight = 1.0f;
    csm.addVertex(v, BLACK, inf);

    CHECK(csm.isStatic());
}

TEST_F(SubmeshFixture, is_not_static_if_has_morph_targets) {
    CalCoreSubmesh csm(2, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(2);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    inf[1].boneId = 1;
    inf[1].weight = 0.0f;
    csm.addVertex(v, BLACK, inf);
    inf[0].boneId = 1;
    inf[0].weight = 0.0f;
    inf[1].boneId = 0;
    inf[1].weight = 1.0f;
    csm.addVertex(v, BLACK, inf);

    CalCoreMorphTarget::VertexOffsetArray vertexOffsets;
    VertexOffset mv;
    mv.vertexId = 0;
    mv.position = CalVector4(0, 1, 2, 0);
    mv.normal = CalVector4(0, -1, -2, 0);
    vertexOffsets.push_back(mv);

    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("", 1, vertexOffsets));
    csm.addMorphTarget(morphTarget);

    CHECK(!csm.isStatic());
}

TEST_F(SubmeshFixture, CalRenderer_getTextureCoordinates_when_there_are_no_texture_coordinates) {
    CalCoreSubmeshPtr coreSubmesh(new CalCoreSubmesh(0, 1, 0));

    const std::vector<CalCoreSubmesh::TextureCoordinate>& texCoords = coreSubmesh->getTextureCoordinates();
    CHECK_EQUAL(0u, texCoords.size());
}

TEST_F(SubmeshFixture, CalRenderer_getNormals_when_there_are_no_normals) {
    CalCoreSubmeshPtr coreSubmesh(new CalCoreSubmesh(0, 0, 0));

    CalCoreMeshPtr coreMesh(new CalCoreMesh);
    coreMesh->submeshes.push_back(coreSubmesh);

    CalCoreSkeletonPtr coreSkeleton(new CalCoreSkeleton);

    CalSkeleton skeleton(coreSkeleton);
    CalSubmesh submesh(coreSubmesh);

    CalPhysique::calculateVerticesAndNormals(
        skeleton.boneTransforms.data(),
        &submesh,
        0);
}

TEST_F(SubmeshFixture, aabox_empty_without_vertices) {
    CalCoreSubmesh csm(0, 0, 0);
    CHECK_EQUAL(CalAABox(), csm.getBoundingVolume());
}

TEST_F(SubmeshFixture, aabox_is_single_point_with_one_vertex) {
    CalCoreSubmesh csm(1, 0, 0);
    CalVector pos(1.0f, 2.0f, 3.0f);

    CalCoreSubmesh::Vertex v;
    v.position = CalPoint4(pos);
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    CHECK_EQUAL(CalAABox(pos, pos), csm.getBoundingVolume());
}

TEST_F(SubmeshFixture, aabox_is_min_max_of_all_vertices) {
    CalCoreSubmesh csm(3, 0, 0);

    CalCoreSubmesh::Vertex v;
    v.position = CalPoint4(CalVector(1.0f, 2.0f, 3.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    v.position = CalPoint4(CalVector(2.0f, 3.0f, 1.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    v.position = CalPoint4(CalVector(3.0f, 1.0f, 2.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    CHECK_EQUAL(
        CalAABox(CalVector(1.0f, 1.0f, 1.0f), CalVector(3.0f, 3.0f, 3.0f)),
        csm.getBoundingVolume());
}

TEST_F(SubmeshFixture, face_constructors) {
    CalCoreSubmesh::Face f2(20, 21, 22);
    CHECK_EQUAL(f2.vertexId[0],20);
    CHECK_EQUAL(f2.vertexId[1],21);
    CHECK_EQUAL(f2.vertexId[2],22);
}

TEST_F(SubmeshFixture, make_cube) {
    CalCoreSubmeshPtr submeshPtr = MakeCube();
    const CalCoreSubmesh::VectorVertex &vertices =  submeshPtr->getVectorVertex();
    CHECK_EQUAL(submeshPtr->getVertexCount(), static_cast<size_t>(24));
    CHECK_EQUAL(CalVector(0,0,1), (vertices[0].position.asCalVector()));
    CHECK_EQUAL(CalVector(1,0,1), (vertices[23].position.asCalVector()));
    CHECK_EQUAL(CalVector(0,0,1), (vertices[0].normal.asCalVector()));
    CHECK_EQUAL(CalVector(0,-1,0), (vertices[23].normal.asCalVector()));
    CHECK_EQUAL(submeshPtr->getFaces()[0], CalCoreSubmesh::Face(0,1,2));
    CHECK_EQUAL(submeshPtr->getFaces()[11], CalCoreSubmesh::Face(20, 23, 21));
}

TEST_F(SubmeshFixture, make_cube_validate_submesh) {
    CalCoreSubmeshPtr submeshPtr = MakeCube();
    CHECK_EQUAL(submeshPtr->validateSubmesh(), true);
}

TEST_F(SubmeshFixture, validate_submesh_is_failed) {
    CalCoreSubmeshPtr cube(new CalCoreSubmesh(4, true, 2));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;

    int curVertexId=0;
    CalCoreSubmesh::TextureCoordinate texCoord;
    CalCoreSubmesh::Vertex vertex;
    //triangle face f0, f1 vertices
    //v0
    curVertexId = 0;
    vertex.position = CalPoint4(0, 0, 1);
    vertex.normal = CalVector4(0, 0, 1);
    cube->addVertex(vertex, BLACK, inf);
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId, texCoord);
    //v1
    ++curVertexId;
    vertex.position = CalPoint4(1,1,1);
    cube->addVertex(vertex, BLACK, inf);
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v2
    ++curVertexId;
    vertex.position = CalPoint4(0,1,1);
    cube->addVertex(vertex, BLACK, inf);
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v3
    ++curVertexId;
    vertex.position = CalPoint4(1,0,1);
    cube->addVertex(vertex, BLACK, inf);
    texCoord.u = 1.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);

    cube->addFace(CalCoreSubmesh::Face(0, 1, 5));
    cube->addFace(CalCoreSubmesh::Face(0, 3, 1));
    CHECK_EQUAL(cube->validateSubmesh(), false);
}

TEST_F(SubmeshFixture, minimumVertexBufferSize_starts_at_0) {
    CalCoreSubmesh csm(1, false, 1);
    CHECK_EQUAL(0u, csm.getMinimumVertexBufferSize());
}

TEST_F(SubmeshFixture, minimumVertexBufferSize_emcompasses_face_range) {
    CalCoreSubmesh csm(1, false, 1);
    csm.addFace(CalCoreSubmesh::Face(1, 3, 2));
    CHECK_EQUAL(4u, csm.getMinimumVertexBufferSize());
}

TEST_F(SubmeshFixture, can_optimize_vertex_cache) {
    CalCoreSubmesh csm = makeMeshWithUnoptimizedVertexCache();

    size_t v0FaceIndexBefore = getFaceIndexWithVertexIndex(csm, 0);
    size_t v6FaceIndexBefore = getFaceIndexWithVertexIndex(csm, 6);
    CHECK_EQUAL(2U, getIndexDistance(v0FaceIndexBefore, v6FaceIndexBefore));

    csm.optimizeVertexCache();

    // We expect vertex cache optimization to move the two faces that share
    // vertices together.
    size_t v0FaceIndexAfter = getFaceIndexWithVertexIndex(csm, 0);
    size_t v6FaceIndexAfter = getFaceIndexWithVertexIndex(csm, 6);
    CHECK_EQUAL(1U, getIndexDistance(v0FaceIndexAfter, v6FaceIndexAfter));
}

TEST_F(SubmeshFixture, can_optimize_vertex_cache_subset) {
    CalCoreSubmesh csm = makeMeshWithUnoptimizedVertexCache();

    size_t v0FaceIndexBefore = getFaceIndexWithVertexIndex(csm, 0);
    size_t v6FaceIndexBefore = getFaceIndexWithVertexIndex(csm, 6);
    CHECK_EQUAL(2U, getIndexDistance(v0FaceIndexBefore, v6FaceIndexBefore));

    csm.optimizeVertexCacheSubset(0, 3);

    // We expect vertex cache optimization to move the two faces that share
    // vertices together.
    size_t v0FaceIndexAfter = getFaceIndexWithVertexIndex(csm, 0);
    size_t v6FaceIndexAfter = getFaceIndexWithVertexIndex(csm, 6);
    CHECK_EQUAL(1U,  getIndexDistance(v0FaceIndexAfter, v6FaceIndexAfter));
}

TEST_F(SubmeshFixture, optimizing_vertex_cache_subset_does_not_affect_verts_outside_of_subset) {
    CalCoreSubmesh csm1 = makeMeshWithUnoptimizedVertexCache();
    CHECK_EQUAL(2U, getFaceIndexWithVertexIndex(csm1, 6));
    csm1.optimizeVertexCacheSubset(0, 2);
    CHECK_EQUAL(2U, getFaceIndexWithVertexIndex(csm1, 6));

    CalCoreSubmesh csm2 = makeMeshWithUnoptimizedVertexCache();
    CHECK_EQUAL(0U, getFaceIndexWithVertexIndex(csm2, 0));
    csm2.optimizeVertexCacheSubset(1, 2);
    CHECK_EQUAL(0U, getFaceIndexWithVertexIndex(csm2, 0));
}

TEST_F(SubmeshFixture, renumber_renumberIndices_without_texcoords) {
    CalCoreSubmesh csm(4, false, 2);
    csm.addFace(CalCoreSubmesh::Face(3, 2, 1));
    csm.addFace(CalCoreSubmesh::Face(2, 0, 1));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].weight = 1.0f;

    inf[0].boneId = 0;
    csm.addVertex(makeVertex(0), 0, inf);
    inf[0].boneId = 1;
    csm.addVertex(makeVertex(1), 1, inf);
    inf[0].boneId = 2;
    csm.addVertex(makeVertex(2), 2, inf);
    inf[0].boneId = 3;
    csm.addVertex(makeVertex(3), 3, inf);

    csm.renumberIndices();

    CHECK_EQUAL(CalCoreSubmesh::Face(0, 1, 2), csm.getFaces()[0]);
    CHECK_EQUAL(CalCoreSubmesh::Face(1, 3, 2), csm.getFaces()[1]);

    CHECK_EQUAL(makeVertex(3), csm.getVectorVertex()[0]);
    CHECK_EQUAL(makeVertex(2), csm.getVectorVertex()[1]);
    CHECK_EQUAL(makeVertex(1), csm.getVectorVertex()[2]);
    CHECK_EQUAL(makeVertex(0), csm.getVectorVertex()[3]);

    CHECK_EQUAL(CalColor32(3), csm.getVertexColors()[0]);
    CHECK_EQUAL(CalColor32(2), csm.getVertexColors()[1]);
    CHECK_EQUAL(CalColor32(1), csm.getVertexColors()[2]);
    CHECK_EQUAL(CalColor32(0), csm.getVertexColors()[3]);

    CHECK_EQUAL(3u, csm.getInfluences()[0].boneId);
    CHECK_EQUAL(2u, csm.getInfluences()[1].boneId);
    CHECK_EQUAL(1u, csm.getInfluences()[2].boneId);
    CHECK_EQUAL(0u, csm.getInfluences()[3].boneId);
}

TEST_F(SubmeshFixture, renumber_vertices_with_texcoords) {
    CalCoreSubmesh csm(4, true, 2);
    csm.addFace(CalCoreSubmesh::Face(3, 2, 1));
    csm.addFace(CalCoreSubmesh::Face(2, 0, 1));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].weight = 1.0f;
    inf[0].boneId = 0;

    csm.addVertex(makeVertex(0), 0, inf);
    csm.setTextureCoordinate(0, makeTextureCoordinate(0));
    csm.addVertex(makeVertex(1), 0, inf);
    csm.setTextureCoordinate(1, makeTextureCoordinate(1));
    csm.addVertex(makeVertex(2), 0, inf);
    csm.setTextureCoordinate(2, makeTextureCoordinate(2));
    csm.addVertex(makeVertex(3), 0, inf);
    csm.setTextureCoordinate(3, makeTextureCoordinate(3));

    csm.renumberIndices();

    CHECK_EQUAL(makeTextureCoordinate(3), csm.getTextureCoordinates()[0]);
    CHECK_EQUAL(makeTextureCoordinate(2), csm.getTextureCoordinates()[1]);
    CHECK_EQUAL(makeTextureCoordinate(1), csm.getTextureCoordinates()[2]);
    CHECK_EQUAL(makeTextureCoordinate(0), csm.getTextureCoordinates()[3]);
}

TEST_F(SubmeshFixture, renumber_morphs) {
    CalCoreSubmesh csm(3, false, 1);
    csm.addFace(CalCoreSubmesh::Face(2, 0, 1));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].weight = 1.0f;

    inf[0].boneId = 0;
    csm.addVertex(makeVertex(0), 0, inf);
    inf[0].boneId = 1;
    csm.addVertex(makeVertex(1), 1, inf);
    inf[0].boneId = 2;
    csm.addVertex(makeVertex(2), 2, inf);

    CalCoreMorphTarget::VertexOffsetArray offsets;
    offsets.push_back(VertexOffset(2, CalPoint4(), CalVector4()));
    csm.addMorphTarget(CalCoreMorphTargetPtr(new CalCoreMorphTarget("morph", 3, offsets)));

    csm.renumberIndices();

    CHECK_EQUAL(CalCoreSubmesh::Face(0, 1, 2), csm.getFaces()[0]);
    CHECK_EQUAL(0u, csm.getMorphTargets()[0]->vertexOffsets[0].vertexId);
}

TEST_F(SubmeshFixture, drop_morph_offsets_for_unused_vertices) {
    CalCoreSubmesh csm(4, false, 1);
    csm.addFace(CalCoreSubmesh::Face(2, 0, 1));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;

    csm.addVertex(makeVertex(0), 0, inf);
    csm.addVertex(makeVertex(1), 1, inf);
    csm.addVertex(makeVertex(2), 2, inf);
    csm.addVertex(makeVertex(3), 3, inf);

    CalCoreMorphTarget::VertexOffsetArray offsets;
    offsets.push_back(VertexOffset(3, CalPoint4(), CalVector4()));
    csm.addMorphTarget(CalCoreMorphTargetPtr(new CalCoreMorphTarget("morph", csm.getVertexCount(), offsets)));

    csm.renumberIndices();

    CHECK_EQUAL(CalCoreSubmesh::Face(0, 1, 2), csm.getFaces()[0]);
    CHECK_EQUAL(3u, csm.getVectorVertex().size());
    CHECK_EQUAL(0u, csm.getMorphTargets()[0]->vertexOffsets.size());
}

TEST_F(SubmeshFixture, minimumVertexBufferSize_is_changed_if_vertex_list_is_shrunk) {
    CalCoreSubmesh csm(4, false, 1);
    csm.addFace(CalCoreSubmesh::Face(1, 2, 3));
    CHECK_EQUAL(4u, csm.getMinimumVertexBufferSize());

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;

    csm.addVertex(makeVertex(0), 0, inf);
    csm.addVertex(makeVertex(1), 1, inf);
    csm.addVertex(makeVertex(2), 2, inf);
    csm.addVertex(makeVertex(3), 3, inf);

    csm.renumberIndices();

    CHECK_EQUAL(CalCoreSubmesh::Face(0, 1, 2), csm.getFaces()[0]);
    CHECK_EQUAL(3u, csm.getVectorVertex().size());
    CHECK_EQUAL(3u, csm.getMinimumVertexBufferSize());
}

FIXTURE(SubmeshNormalFixture) {
    static void checkNormalizedNormals(
        const CalVector4& exp,
        const CalVector4& in
    ) {
        CalCoreSubmesh csm(3, 0, 1);
        CalCoreSubmesh::Vertex vertex;
        vertex.position = CalPoint4(1, 1, 1, 0);
        vertex.normal = in;
        CalCoreSubmesh::InfluenceVector iv(1);
        csm.addVertex(vertex, 0, iv);
        csm.addVertex(vertex, 0, iv);
        csm.addVertex(vertex, 0, iv);

        csm.normalizeNormals();
        const CalCoreSubmesh::VectorVertex& vertices =  csm.getVectorVertex();
        CHECK_EQUAL(3u, vertices.size());
        const CalVector4& normal = vertices[0].normal;
        CHECK_CLOSE(exp.x, normal.x, 0.001);
        CHECK_CLOSE(exp.y, normal.y, 0.001);
        CHECK_CLOSE(exp.z, normal.z, 0.001);
    }
};

TEST_F(SubmeshNormalFixture, can_normalize_bad_normals) {
    checkNormalizedNormals(
        CalVector4(0.707106781187f, -0.707106781187f, 0.0f, 0.0f),
        CalVector4(1000.0f, -1000.0f, 0.0f, 0.0f)
    );
}

TEST_F(SubmeshNormalFixture, returns_a_default_when_asked_to_normalize_zero_normals) {
    checkNormalizedNormals(
        CalVector4(0.0f, 1.0f, 0.0f, 0.0f),
        CalVector4(0.0f, 0.0f, 0.0f, 0.0f)
    );
}

TEST_F(SubmeshNormalFixture, returns_a_default_when_asked_to_normalize_inf_normals) {
    float inf = std::numeric_limits<float>::infinity();
    checkNormalizedNormals(
        CalVector4(0.0f, 1.0f, 0.0f, 0.0f),
        CalVector4(inf, 1.0f, 0.0f, 0.0f)
    );
}

TEST_F(SubmeshNormalFixture, returns_a_default_when_asked_to_normalize_negative_inf_normals) {
    float inf = std::numeric_limits<float>::infinity();
    checkNormalizedNormals(
        CalVector4(0.0f, 1.0f, 0.0f, 0.0f),
        CalVector4(-inf, 1.0f, 0.0f, 0.0f)
    );
}

TEST_F(SubmeshNormalFixture, returns_a_default_when_asked_to_normalize_nan_normals) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    checkNormalizedNormals(
        CalVector4(0.0f, 1.0f, 0.0f, 0.0f),
        CalVector4(nan, 1.0f, 0.0f, 0.0f)
    );
}

TEST_F(SubmeshFixture, can_export_influences) {
    CalCoreSubmesh csm(4, 0, 1);
    CalCoreSubmesh::Vertex vertex;

    CalCoreSubmesh::InfluenceVector iv0;
    iv0.push_back(CalCoreSubmesh::Influence(0, 1.0f, true));

    CalCoreSubmesh::InfluenceVector iv1;
    iv1.push_back(CalCoreSubmesh::Influence(0, 0.5f, false));
    iv1.push_back(CalCoreSubmesh::Influence(1, 0.5f, true));

    CalCoreSubmesh::InfluenceVector iv2;
    iv2.push_back(CalCoreSubmesh::Influence(2, 1.0f, true));

    CalCoreSubmesh::InfluenceVector iv3;
    iv3.push_back(CalCoreSubmesh::Influence(3, 1.0f, true));

    csm.addVertex(makeVertex(0), 0, iv0);
    csm.addVertex(makeVertex(0), 0, iv1);
    csm.addVertex(makeVertex(0), 0, iv2);
    csm.addVertex(makeVertex(0), 0, iv3);

    csm.addFace(CalCoreSubmesh::Face(0, 1, 2));
    csm.addFace(CalCoreSubmesh::Face(2, 1, 3));

    auto influences = csm.exportInfluences(4);
    CHECK_EQUAL(2U, influences.maximumInfluenceCount);

    CHECK_EQUAL(4U, influences.weightsBoneIdsPairs.size());

    float expWeights0Arr[] = {1.0f, 0.0f, 0.0f, 0.0f};
    CHECK_VECTOR_CLOSE(arrayToVector(expWeights0Arr), influences.weightsBoneIdsPairs[0].weights, 0.001f);

    unsigned int expBoneIds0Arr[] = {0, 0, 0, 0};
    CHECK_EQUAL(arrayToVector(expBoneIds0Arr), influences.weightsBoneIdsPairs[0].boneIds);

    float expWeights1Arr[] = {0.5f, 0.5f, 0.0f, 0.0f};
    CHECK_VECTOR_CLOSE(arrayToVector(expWeights1Arr), influences.weightsBoneIdsPairs[1].weights, 0.001f);

    unsigned int expBoneIds1Arr[] = {0, 1, 0, 0};
    CHECK_EQUAL(arrayToVector(expBoneIds1Arr), influences.weightsBoneIdsPairs[1].boneIds);

    unsigned int expBoneIdsArr[] = {0, 1, 2, 3};
    CHECK_EQUAL(arrayToVector(expBoneIdsArr), influences.usedBoneIds);
}

TEST_F(SubmeshFixture, exportInfluences_raises_exception_on_incomplete_mesh) {
    CalCoreSubmesh csm(4, 0, 1);
    CalCoreSubmesh::Vertex vertex;
    CalCoreSubmesh::InfluenceVector influenceVector;
    // We declared two verts in the constructor, but we only add one, so the
    // vertex and influence counts don't match:
    csm.addVertex(vertex, 0, influenceVector);
    CHECK_THROW(csm.exportInfluences(4), std::runtime_error);
}

TEST_F(SubmeshFixture, exportInfluences_handles_empty_mesh) {
    CalCoreSubmesh csm(0, 0, 0);
    auto influences = csm.exportInfluences(4);
    CHECK_EQUAL(0U, influences.maximumInfluenceCount);
    CHECK(influences.weightsBoneIdsPairs.empty());
    CHECK(influences.usedBoneIds.empty());
}

TEST_F(SubmeshFixture, exportInfluences_limits_influences_and_renormalizes) {
    auto csm = makeSubmeshWithMultipleInfluences(8);

    auto influences2 = csm.exportInfluences(2);

    float expWeights2Arr[] = {0.5f, 0.5f};
    CHECK_VECTOR_CLOSE(arrayToVector(expWeights2Arr), influences2.weightsBoneIdsPairs[0].weights, 0.001f);

    unsigned int expBoneIds2Arr[] = {0, 1};
    CHECK_EQUAL(arrayToVector(expBoneIds2Arr), influences2.weightsBoneIdsPairs[0].boneIds);

    unsigned int expUsedBoneIds2Arr[] = {0, 1};
    CHECK_EQUAL(arrayToVector(expUsedBoneIds2Arr), influences2.usedBoneIds);

    auto influences4 = csm.exportInfluences(4);

    float expWeights4Arr[] = {0.25f, 0.25f, 0.25f, 0.25f};
    CHECK_VECTOR_CLOSE(arrayToVector(expWeights4Arr), influences4.weightsBoneIdsPairs[0].weights, 0.001f);

    unsigned int expBoneIds4Arr[] = {0, 1, 2, 3};
    CHECK_EQUAL(arrayToVector(expBoneIds4Arr), influences4.weightsBoneIdsPairs[0].boneIds);

    unsigned int expUsedBoneIds4Arr[] = {0, 1, 2, 3};
    CHECK_EQUAL(arrayToVector(expUsedBoneIds4Arr), influences4.usedBoneIds);
}
