//****************************************************************************//
// coreMorphTrack.h                                                                //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/coremorphkeyframe.h"

class CalCoreMorphKeyframe;

class CAL3D_API CalCoreMorphTrack {
public:
    typedef std::vector<CalCoreMorphKeyframe> MorphKeyframeList;

    std::string morphName;
    MorphKeyframeList keyframes;

    CalCoreMorphTrack();
    CalCoreMorphTrack(const std::string& morphName, const MorphKeyframeList& keyframes);

    size_t size() const;

    float getState(float time);

    void addCoreMorphKeyframe(CalCoreMorphKeyframe pCoreKeyframe);
    void scale(float factor);

private:
    std::vector<CalCoreMorphKeyframe>::iterator getUpperBound(float time);
};

inline bool operator==(const CalCoreMorphTrack& lhs, const CalCoreMorphTrack& rhs) {
    return lhs.morphName == rhs.morphName &&
           lhs.keyframes == rhs.keyframes;
}
