//****************************************************************************//
// coresubmorphtarget.h                                                       //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/coresubmesh.h"

class CalQuaternion;

struct VertexOffset {
    size_t vertexId;
    CalPoint4 position;
    CalVector4 normal;

    VertexOffset()
        : vertexId(0) 
    {}

    VertexOffset(const size_t vertexId, const CalPoint4& p, const CalVector4& n)
        : vertexId(vertexId)
        , position(p)
        , normal(n)
    {}
};

class CAL3D_API CalCoreMorphTarget {
public:
    typedef cal3d::SSEArray<VertexOffset> VertexOffsetArray;

    const std::string name;
    const CalMorphTargetType morphTargetType;
    const VertexOffsetArray vertexOffsets;

    CalCoreMorphTarget(const std::string& name, const size_t vertexCount, const VertexOffsetArray& vertexOffsets);

    size_t size() const;

    void scale(float factor);
    void addVertexOffset(const size_t vertexId, const CalCoreSubmesh::Vertex& v);
};
CAL3D_PTR(CalCoreMorphTarget);
