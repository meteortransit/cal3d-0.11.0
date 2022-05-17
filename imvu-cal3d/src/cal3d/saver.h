//****************************************************************************//
// saver.h                                                                    //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once
#include <boost/shared_ptr.hpp>

#include "cal3d/global.h"

CAL3D_PTR(CalCoreSkeleton);
class CalCoreBone;
CAL3D_PTR(CalCoreAnimation);
CAL3D_PTR(CalCoreMorphAnimation);
class CalCoreTrack;
class CalCoreKeyframe;
class CalCoreMorphTrack;
class CalCoreMorphKeyframe;
CAL3D_PTR(CalCoreMesh);
class CalCoreSubmesh;
CAL3D_PTR(CalCoreMaterial);
class CalVector;
class CalQuaternion;

class CAL3D_API CalSaver {
public:
    static std::string saveCoreAnimationToBuffer(CalCoreAnimationPtr pCoreAnimation);
    static std::string saveCoreMorphAnimationToBuffer(CalCoreMorphAnimationPtr pCoreMorphAnimation);
    static std::string saveCoreMaterialToBuffer(CalCoreMaterialPtr pCoreMaterial);
    static std::string saveCoreMeshToBuffer(CalCoreMeshPtr pCoreMesh);
    static std::string saveCoreSkeletonToBuffer(CalCoreSkeletonPtr pCoreSkeleton);

    static std::string saveCoreAnimationXmlToBuffer(CalCoreAnimationPtr pCoreAnimation);

    static bool saveCoreAnimation(const std::string& strFilename, CalCoreAnimation* pCoreAnimation);
    static bool saveCoreAnimation(std::ostream& stream, CalCoreAnimation* coreAnimation);

    static bool saveCoreMorphAnimation(const std::string& strFilename, CalCoreMorphAnimation* pCoreMorphAnimation);
    static bool saveCoreMorphAnimation(std::ostream& file, CalCoreMorphAnimation* pCoreMorphAnimation);

    static std::string saveCoreMorphAnimationXmlToBuffer(CalCoreMorphAnimationPtr pCoreMorphAnimation);

    static bool saveCoreMaterial(const std::string& strFilename, CalCoreMaterial* pCoreMaterial);
    static bool saveCoreMaterial(std::ostream& file, CalCoreMaterial* pCoreMaterial);

    static bool saveCoreMesh(const std::string& strFilename, CalCoreMesh* pCoreMesh);
    static bool saveCoreMesh(std::ostream& os, CalCoreMesh* pCoreMesh);

    static bool saveCoreSkeleton(const std::string& strFilename, CalCoreSkeleton* pCoreSkeleton);
    static bool saveCoreSkeleton(std::ostream& os, CalCoreSkeleton* pCoreSkeleton);

    static bool saveXmlCoreSkeleton(const std::string& strFilename, CalCoreSkeleton* pCoreSkeleton);
    static bool saveXmlCoreAnimation(const std::string& strFilename, CalCoreAnimation* pCoreAnimation);
    static bool saveXmlCoreAnimation(std::ostream& os, CalCoreAnimation* pCoreAnimation);
    static bool saveXmlCoreMorphAnimation(const std::string& strFilename, CalCoreMorphAnimation* pCoreMorphAnimation);
    static bool saveXmlCoreMorphAnimation(std::ostream& os, CalCoreMorphAnimation* pCoreMorphAnimation);
    static bool saveXmlCoreMesh(const std::string& strFilename, CalCoreMesh* pCoreMesh);
    static bool saveXmlCoreMesh(std::ostream& os, CalCoreMesh* pCoreMesh);
    static bool saveXmlCoreMaterial(const std::string& strFilename, CalCoreMaterial* pCoreMaterial);

private:
    static bool saveCoreBone(std::ostream& file, const CalCoreSkeleton* coreSkeleton, const CalCoreBone* pCoreBone);
    static bool saveCoreKeyframe(std::ostream& file, const std::string& strFilename, const CalCoreKeyframe* pCoreKeyframe);
    static bool saveCoreSubmesh(std::ostream& file, CalCoreSubmesh* pCoreSubmesh);
    static bool saveCoreTrack(std::ostream& file, const std::string& strFilename, CalCoreTrack* pCoreTrack);
    static bool saveCoreMorphKeyframe(std::ostream& file, const std::string& strFilename, CalCoreMorphKeyframe* pCoreMorphKeyframe);
    static bool saveCoreMorphTrack(std::ostream& file, const std::string& strFilename, CalCoreMorphTrack* pCoreMorphTrack);

private:
    CalSaver();
    ~CalSaver();
};
