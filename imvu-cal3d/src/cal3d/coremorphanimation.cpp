//****************************************************************************//
// coreanimatedMorph.cpp                                                          //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include "cal3d/coremorphanimation.h"
#include "cal3d/coremorphtrack.h"
#include "cal3d/memory.h"

CAL3D_DEFINE_SIZE(CalCoreMorphTrack*);

size_t sizeInBytes(const CalCoreMorphTrack& t) {
    return t.size();
}

size_t CalCoreMorphAnimation::sizeInBytes() const {
    size_t r = sizeof(*this);
    r += ::sizeInBytes(tracks);
    return r;
}

void CalCoreMorphAnimation::removeZeroScaleTracks() {
    std::vector<CalCoreMorphTrack> & p = tracks;
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<CalCoreMorphTrack>::iterator iteratorCoreTrack;
        for (iteratorCoreTrack = p.begin(); iteratorCoreTrack != p.end(); ++iteratorCoreTrack) {
            // get the core bone
            CalCoreMorphTrack* pCoreTrack;
            pCoreTrack = &(*iteratorCoreTrack);
            std::vector<CalCoreMorphKeyframe> & morphNameList = pCoreTrack->keyframes;

            bool nonZeroScaleTrack = false;
            for (size_t keyframeId = 0; keyframeId < morphNameList.size(); keyframeId++) {
                float weight = morphNameList[keyframeId].weight;
                if (weight != 0.0f) {
                    nonZeroScaleTrack = true;
                    break;
                }
            }
            if (!nonZeroScaleTrack) {
                p.erase(iteratorCoreTrack);
                changed = true;
                break;
            }
        }
    }
}

void CalCoreMorphAnimation::scale(float factor) {
    std::for_each(tracks.begin(), tracks.end(), std::bind2nd(std::mem_fun_ref(&CalCoreMorphTrack::scale), factor));
}

CalCoreMorphTrack* CalCoreMorphAnimation::getCoreTrack(std::string const& name) {
    // loop through all core track
    std::vector<CalCoreMorphTrack>::iterator iteratorCoreTrack;
    for (iteratorCoreTrack = tracks.begin(); iteratorCoreTrack != tracks.end(); ++iteratorCoreTrack) {
        // check if we found the matching core bone
        if (iteratorCoreTrack->morphName == name) {
            return &(*iteratorCoreTrack);
        }
    }

    // no match found
    return 0;
}
