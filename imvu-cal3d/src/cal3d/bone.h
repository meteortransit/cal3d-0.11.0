//****************************************************************************//
// bone.h                                                                     //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/transform.h"

class CalCoreBone;
class CalSkeleton;
class CalModel;

namespace cal3d {
    class CAL3D_API TransformAccumulator {
    public:
        TransformAccumulator();

        void reset(const RotateTranslate& defaultPose);

        void addTransform(float weight, const RotateTranslate& transform);
        const RotateTranslate& getWeightedMean() const {
            return currentTransform;
        }

    private:
        float totalWeight;
        RotateTranslate currentTransform;
    };
}

class CAL3D_API CalBone {
public:
    const int parentId;
    cal3d::Transform absoluteTransform;
    cal3d::Scale scale;

    CalBone(const CalCoreBone& coreBone);

    const cal3d::RotateTranslate& getRelativeTransform() const {
        return transformAccumulator.getWeightedMean();
    }

    const CalVector& getOriginalTranslation() const {
        return coreRelativeTransform.translation;
    }

    void resetPose();
    void blendPose(
        float weight,
        const cal3d::RotateTranslate& transform,
        float subsequentAttenuation);

    void calculateAbsolutePose(const CalBone* bones);

private:
    // from core bone. stored locally for better cache locality
    const cal3d::RotateTranslate coreRelativeTransform;

    // animated bone state
    cal3d::Scale absoluteScale;
    float currentAttenuation;
    cal3d::TransformAccumulator transformAccumulator;
};
