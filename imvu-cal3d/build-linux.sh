#!/bin/sh
cd "`dirname \"$0\"`"

SOURCES="
    src/cal3d/animation.cpp
    src/cal3d/bone.cpp
    src/cal3d/buffersource.cpp
    src/cal3d/calxmlbindings.cpp
    src/cal3d/coreanimatedmorph.cpp
    src/cal3d/coreanimation.cpp
    src/cal3d/corebone.cpp
    src/cal3d/coremesh.cpp
    src/cal3d/coremodel.cpp
    src/cal3d/coremorphkeyframe.cpp
    src/cal3d/coremorphtrack.cpp
    src/cal3d/coreskeleton.cpp
    src/cal3d/coresubmesh.cpp
    src/cal3d/coresubmorphtarget.cpp
    src/cal3d/coretrack.cpp
    src/cal3d/error.cpp
    src/cal3d/global.cpp
    src/cal3d/loader.cpp
    src/cal3d/matrix.cpp
    src/cal3d/mesh.cpp
    src/cal3d/mixer.cpp
    src/cal3d/model.cpp
    src/cal3d/physique.cpp
    src/cal3d/platform.cpp
    src/cal3d/quaternion.cpp
    src/cal3d/renderer.cpp
    src/cal3d/saver.cpp
    src/cal3d/skeleton.cpp
    src/cal3d/streamsource.cpp
    src/cal3d/submesh.cpp
    src/cal3d/tinybind.cpp
    src/cal3d/tinystr.cpp
    src/cal3d/tinyxml.cpp
    src/cal3d/tinyxmlerror.cpp
    src/cal3d/tinyxmlparser.cpp
    src/cal3d/trisort.cpp
    src/cal3d/vector.cpp
    src/cal3d/xmlformat.cpp"

g++ -Wl,--demangle -Wall -shared -Isrc -I/usr/include/boost-1_33_1 -fpermissive -Wno-non-virtual-dtor -Wno-sign-compare -Wno-reorder -o cal3d.so $SOURCES
