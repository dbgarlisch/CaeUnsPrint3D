/****************************************************************************
 *
 * class CaeUnsPrint3D
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2011 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#include <stdarg.h>
#include <math.h>
#include <string.h>

#include "apiCAEP.h"
#include "apiCAEPUtils.h"
#include "apiGridModel.h"
#include "apiPWP.h"
#include "runtimeWrite.h"
#include "pwpPlatform.h"

#include "CaePlugin.h"
#include "CaeUnsGridModel.h"
#include "CaeUnsPrint3D.h"

const char  SolidName[]         = "Pointwise_Print3D";
const char  AttrEdgeDiameter[]  = "EdgeDiameter";
const char  AttrMultiSolid[]    = "MultiSolid";
const char  AttrNumPoints[]     = "NumPoints";


static bool
valZero(double val)
{
    const double ZeroTol = 1.0E-10;
    return ::fabs(val) < ZeroTol;
}


static double
roundZero(double val)
{
    return valZero(val) ? 0 : val;
}


static bool
bcIs(const PWGM_CONDDATA &cond, const char bcType[])
{
    return 0 == strcmp(cond.type, bcType);
}


static bool
isHidden(const PWGM_CONDDATA &cond)
{
    return bcIs(cond, "hidden");
}


static bool
isSolid(const PWGM_CONDDATA &cond)
{
    return bcIs(cond, "solid");
}


static bool
isHidden(const CaeUnsPatch &patch)
{
    PWGM_CONDDATA cond;
    return patch.condition(cond) && isHidden(cond);
}


static bool
isSolid(const CaeUnsPatch &patch)
{
    PWGM_CONDDATA cond;
    return patch.condition(cond) && isSolid(cond);
}


static bool
isHidden(const CaeUnsBlock &block)
{
    PWGM_CONDDATA cond;
    return block.condition(cond) && isHidden(cond);
}


static bool
isSolid(const CaeUnsBlock &block)
{
    PWGM_CONDDATA cond;
    return block.condition(cond) && isSolid(cond);
}



//***************************************************************************
//***************************************************************************
//***************************************************************************

CaeUnsPrint3D::CaeUnsPrint3D(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL
        model, const CAEP_WRITEINFO *pWriteInfo) :
    CaeUnsPlugin(pRti, model, pWriteInfo),
    edges_(),
    numTris_(0),
    numSolids_(0),
    multiSolid_(PWP_TRUE),
    radius_(DefCylDiam / 2.0),
    zOffset_(DefCylDiam / 3.0),
    numBasePts_(DefNumBasePts)
{
}

CaeUnsPrint3D::~CaeUnsPrint3D()
{
}

bool
CaeUnsPrint3D::beginExport()
{
    if (isBinaryEncoding()) {
        // binary does not support multi-solid
        multiSolid_ = false;
    }
    else {
        model_.getAttribute(AttrMultiSolid, multiSolid_, true);
    }
    
    PWP_REAL cylDiam;
    model_.getAttribute(AttrEdgeDiameter, cylDiam, DefCylDiam);
    radius_ = cylDiam / 2.0;
    zOffset_ = cylDiam / 3.0;

    model_.getAttribute(AttrNumPoints, numBasePts_, DefNumBasePts);

    // Initialize masterCylBase_ data
    // polygon is centered at the origin in the z=0 plane
    // polygon's right-handed normal is +z
    const double PI = 3.141592653589793;
    double angle = 0; // radians
    double deltaR = (2 * PI) / numBasePts_;
    for (PWP_UINT ii = 0; ii < numBasePts_; ++ii, angle += deltaR) {
        // load cyl base pts (xyz)
        masterCylBase_[ii].set(cos(angle) * radius_, sin(angle) * radius_, 0);
    }

    setProgressMajorSteps(2);

    return true;
}

PWP_BOOL
CaeUnsPrint3D::write()
{
    writeHeader();
    writePatches();
    writeBlocks();
    writeFooter();
    return true;
}

bool
CaeUnsPrint3D::endExport()
{
    return true;
}


void
CaeUnsPrint3D::writeStr(const char *format, ...)
{
    if (isAsciiEncoding()) {
        va_list args;
        va_start(args, format);
        vfprintf(fp(), format, args);
        va_end(args);
    }
}


void
CaeUnsPrint3D::writeXyz(const char prefix[], const vector3 &xyz)
{
    const int AsciiFloatPrec = 8;
    if (isAsciiEncoding()) {
        fprintf(fp(), "%s %.*g %.*g %.*g\n", prefix,
            AsciiFloatPrec, roundZero(xyz[0]),
            AsciiFloatPrec, roundZero(xyz[1]),
            AsciiFloatPrec, roundZero(xyz[2]));
    }
    else if (isBinaryEncoding()) {
        float val;
        val = (float)roundZero(xyz[0]);
        pwpFileWrite(&val, sizeof(val), 1, fp());
        val = (float)roundZero(xyz[1]);
        pwpFileWrite(&val, sizeof(val), 1, fp());
        val = (float)roundZero(xyz[2]);
        pwpFileWrite(&val, sizeof(val), 1, fp());
    }
}


void
CaeUnsPrint3D::writeAttrByteCount(PWP_UINT16 byteCnt)
{
    if (isBinaryEncoding()) {
        pwpFileWrite(&byteCnt, sizeof(byteCnt), 1, fp());
    }
}


void
CaeUnsPrint3D::writeTriFacet(const vector3 &p0, const vector3 &p1,
    const vector3 &p2)
{
    // from: http://en.wikipedia.org/wiki/STL_(file_format)
    //
    // In both ASCII and binary versions of STL, the facet normal should be a
    // unit vector pointing OUTWARDS from the solid object. In most software
    // this may be set to (0,0,0) and the software will automatically calculate
    // a normal based on the order of the triangle vertices using the 'right
    // hand rule'. Some STL loaders (eg the STL plugin for Art of Illusion)
    // check that the normal in the file agrees with the normal they calculate
    // using the right hand rule and warn you when it does not. Other software
    // may ignore the facet normal entirely and use only the right hand rule.
    // So in order to be entirely portable one should provide both the facet
    // normal and order the vertices appropriately – even though it is
    // seemingly redundant to do so. Some other software (e.g. SolidWorks) use
    // the normal for shading effects, so the "normals" listed in the file are
    // not the true facets' normals.
    //
    // ******* ASCII export:
    //  facet normal  0.000000e+000  0.000000e+000  1.000000e+000
    //    outer loop
    //      vertex    0.000000e+000  0.000000e+000  0.000000e+000
    //      vertex    5.000000e-001  0.000000e+000  0.000000e+000
    //      vertex    5.000000e-001  5.000000e-001  0.000000e+000
    //    endloop
    //  endfacet
    //
    // ******* BINARY export:
    // foreach triangle
    //   REAL32[3]       –    Normal vector
    //   REAL32[3]       –    Vertex 1
    //   REAL32[3]       –    Vertex 2
    //   REAL32[3]       –    Vertex 3
    //   UINT16          –    Attribute byte count
    // end
    writeXyz("facet normal", cml::cross((p1 - p0), (p2 - p1)).normalize());
    writeStr(" outer loop\n");
    writeXyz("  vertex", p0);
    writeXyz("  vertex", p1);
    writeXyz("  vertex", p2);
    writeAttrByteCount();
    writeStr(" endloop\n");
    writeStr("endfacet\n");
    ++numTris_;
}


void
CaeUnsPrint3D::writeQuadFacet(const vector3 &p0, const vector3 &p1,
    const vector3 &p2, const vector3 &p3)
{
    writeTriFacet(p0, p1, p2);
    writeTriFacet(p0, p2, p3);
}


void
CaeUnsPrint3D::beginMultiSolid()
{
    // ******* ASCII export:
    // "solid [name]\n"
    //
    if (multiSolid_ && isAsciiEncoding()) {
        sprintf(curSolidName_, "%s_%06lu", SolidName,
            (unsigned long)++numSolids_);
        writeStr("solid %s\n", curSolidName_);
    }
}


void
CaeUnsPrint3D::endMultiSolid()
{
    if (multiSolid_ && isAsciiEncoding()) {
        // ******* ASCII export:
        // "endsolid [name]\n"
        //
        writeStr("endsolid %s\n", curSolidName_);
    }
}


void
CaeUnsPrint3D::makeCylinder(const matrix33 &rot, const vector3 &tran0,
    const vector3 &tran1, Cylinder &cyl)
{
    vector3 pt;
    for (PWP_UINT ii = 0; ii < numBasePts_; ++ii) {
        pt = masterCylBase_[ii] * rot;
        cyl[0][ii] = pt + tran0;
        cyl[1][ii] = pt + tran1;
    }
}


void
CaeUnsPrint3D::writeCylBase(const CylBase &base, bool reverse)
{
    if (!reverse) {
        for (PWP_UINT ii = 1; ii < numBasePts_ - 1; ++ii) {
            writeTriFacet(base[0], base[ii + 1], base[ii]);
        }
    }
    else {
        for (PWP_UINT ii = 1; ii < numBasePts_ - 1; ++ii) {
            writeTriFacet(base[0], base[ii], base[ii + 1]);
        }
    }
}


void
CaeUnsPrint3D::writeCylSides(const CylBase &cb0, const CylBase &cb1)
{
    PWP_UINT ii;
    for (ii = 0; ii < numBasePts_ - 1; ++ii) {
        writeQuadFacet(cb1[ii], cb0[ii], cb0[ii + 1], cb1[ii + 1]);
    }
    // here, ii == numBasePts_ - 1
    // write last side quad that wraps back to first side quad
    writeQuadFacet(cb1[ii], cb0[ii], cb0[0], cb1[0]);
}


void
CaeUnsPrint3D::writeCylinder(const vector3 &p0, const vector3 &p1)
{
    vector3 zaxis(0, 0, 1);
    vector3 cylAxis = (p1 - p0);
    vector3 dLen = zOffset_ * normalize(cylAxis);
    matrix33 v2v;
    cml::matrix_rotation_vec_to_vec(v2v, cylAxis, zaxis);
    Cylinder cyl;
    makeCylinder(v2v, p0 - dLen, p1 + dLen, cyl);
    beginMultiSolid();
    writeCylBase(cyl[0], false);
    writeCylBase(cyl[1], true);
    writeCylSides(cyl[0], cyl[1]);
    endMultiSolid();
}


void
CaeUnsPrint3D::writeCylinder(const PWGM_VERTDATA &vd0,
    const PWGM_VERTDATA &vd1)
{
    vector3 p0;
    vector3 p1;
    // try to keep vec from p0->p1 in +z direction
    if (vd0.z <= vd1.z) {
        p0.set(vd0.x, vd0.y, vd0.z);
        p1.set(vd1.x, vd1.y, vd1.z);
    }
    else {
        p0.set(vd1.x, vd1.y, vd1.z);
        p1.set(vd0.x, vd0.y, vd0.z);
    }
    writeCylinder(p0, p1);
}


bool
CaeUnsPrint3D::isNewEdge(const Edge &e)
{
    return edges_.insert(e).second;
}


void
CaeUnsPrint3D::writeEdge(const CaeUnsVertex &v0, const CaeUnsVertex &v1)
{
    if (v0.index() != v1.index()) {
        PWGM_VERTDATA vd0;
        PWGM_VERTDATA vd1;
        if (v0.dataMod(vd0) && v1.dataMod(vd1)) {
            Edge e(vd0.i, vd1.i);
            if (isNewEdge(e)) {
                writeCylinder(vd0, vd1);
            }
        }
    }
}


void
CaeUnsPrint3D::writePolygon(const CaeUnsVertex &v0, const CaeUnsVertex &v1,
    const CaeUnsVertex &v2, const CaeUnsVertex &v3)
{
    writeEdge(v0, v1);
    writeEdge(v1, v2);
    writeEdge(v2, v3);
    writeEdge(v3, v0);
}


void
CaeUnsPrint3D::writePolygon(const CaeUnsVertex &v0, const CaeUnsVertex &v1,
    const CaeUnsVertex &v2)
{
    writeEdge(v0, v1);
    writeEdge(v1, v2);
    writeEdge(v2, v0);
}


void
CaeUnsPrint3D::writeThickenedPolygon(const vector3 &tp0, const vector3 &tp1,
    const vector3 &tp2)
{
    double halfThickness = radius_;
    vector3 norm = cml::cross((tp1 - tp0), (tp2 - tp1)).normalize();
    vector3 offset = norm * halfThickness;
    // "thicken" by halfThickness to either side of tri pts
    vector3 p0 = tp0 - offset;
    vector3 p1 = tp1 - offset;
    vector3 p2 = tp2 - offset;
    vector3 p3 = tp0 + offset;
    vector3 p4 = tp1 + offset;
    vector3 p5 = tp2 + offset;
    // In STL, the facet normal should be a unit vector pointing OUTWARDS from
    // the solid object.
    //
    // we now have the 6 prism points - write out the solid
    beginMultiSolid();
    writeTriFacet(p0, p2, p1);
    writeTriFacet(p3, p4, p5);
    writeQuadFacet(p0, p1, p4, p3);
    writeQuadFacet(p1, p2, p5, p4);
    writeQuadFacet(p2, p0, p3, p5);
    endMultiSolid();
}


void
CaeUnsPrint3D::writeThickenedPolygon(const CaeUnsVertex &v0, const CaeUnsVertex &v1,
    const CaeUnsVertex &v2)
{
    PWGM_VERTDATA vd0;
    PWGM_VERTDATA vd1;
    PWGM_VERTDATA vd2;
    if (PwVertDataMod(v0, &vd0) && PwVertDataMod(v1, &vd1) &&
            PwVertDataMod(v2, &vd2)) {
        vector3 p0(vd0.x, vd0.y, vd0.z);
        vector3 p1(vd1.x, vd1.y, vd1.z);
        vector3 p2(vd2.x, vd2.y, vd2.z);
        writeThickenedPolygon(p0, p1, p2);
    }
}


void
CaeUnsPrint3D::writeThickenedPolygon(const vector3 &qp0, const vector3 &qp1,
    const vector3 &qp2, const vector3 &qp3)
{
    vector3 norm0 = cml::cross((qp1 - qp0), (qp2 - qp0)).normalize();
    vector3 norm1 = cml::cross((qp2 - qp0), (qp3 - qp0)).normalize();
    vector3 normSeam = (norm0 + norm1).normalize(); // diag seam normal
    // thicken this amount to either side of tri
    double halfThickness = radius_;
    // Account for non-planar quad to make thickness uniform
    // dot(norm0, normSeam) == dot(norm1, normSeam) == cos(angle). Where,
    // angle (in radians) is between norm0/norm1 and normSeam.
    // For a right-triangle: cos(angle) = opp / hyp
    // Hence: hyp = opp / cos(angle)
    // For this geometry, seamThickness == hyp
    double seamThickness = halfThickness / dot(norm0, normSeam);
    vector3 offset0 = norm0 * halfThickness;
    vector3 offset1 = norm1 * halfThickness;
    vector3 offsetSeam = normSeam * seamThickness;
    // hex base pts
    vector3 p0 = qp0 - offsetSeam;
    vector3 p1 = qp1 - offset0;
    vector3 p2 = qp2 - offsetSeam;
    vector3 p3 = qp3 - offset1;
    // hex top pts
    vector3 p4 = qp0 + offsetSeam;
    vector3 p5 = qp1 + offset0;
    vector3 p6 = qp2 + offsetSeam;
    vector3 p7 = qp3 + offset1;
    // In STL, the facet normal should be a unit vector pointing OUTWARDS from
    // the solid object.
    // we now have the 8 hex points - write out the solid
    beginMultiSolid();
    writeQuadFacet(p0, p3, p2, p1);
    writeQuadFacet(p4, p5, p6, p7);
    writeQuadFacet(p0, p1, p5, p4);
    writeQuadFacet(p1, p2, p6, p5);
    writeQuadFacet(p2, p3, p7, p6);
    writeQuadFacet(p3, p0, p4, p7);
    endMultiSolid();
}


void
CaeUnsPrint3D::writeThickenedPolygon(const CaeUnsVertex &v0, const CaeUnsVertex &v1,
    const CaeUnsVertex &v2, const CaeUnsVertex &v3)
{
    PWGM_VERTDATA vd0;
    PWGM_VERTDATA vd1;
    PWGM_VERTDATA vd2;
    PWGM_VERTDATA vd3;
    if (PwVertDataMod(v0, &vd0) && PwVertDataMod(v1, &vd1) &&
            PwVertDataMod(v2, &vd2) && PwVertDataMod(v3, &vd3)) {
        vector3 p0(vd0.x, vd0.y, vd0.z);
        vector3 p1(vd1.x, vd1.y, vd1.z);
        vector3 p2(vd2.x, vd2.y, vd2.z);
        vector3 p3(vd3.x, vd3.y, vd3.z);
        writeThickenedPolygon(p0, p1, p2, p3);
    }
}


void
CaeUnsPrint3D::writeElemData(PWGM_ELEMDATA &ed, bool solid)
{
    switch (ed.type) {
        case PWGM_ELEMTYPE_POINT:
            break;
        case PWGM_ELEMTYPE_BAR:
            writeEdge(ed.vert[0], ed.vert[1]);
            break;
        case PWGM_ELEMTYPE_QUAD:
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[2], ed.vert[3]);
            if (solid) {
                writeThickenedPolygon(ed.vert[0], ed.vert[1], ed.vert[2],
                    ed.vert[3]);
            }
            break;
        case PWGM_ELEMTYPE_TRI:
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[2]);
            if (solid) {
                writeThickenedPolygon(ed.vert[0], ed.vert[1],
                    ed.vert[2]);
            }
            break;
        case PWGM_ELEMTYPE_HEX:
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[2], ed.vert[3]);
            writePolygon(ed.vert[4], ed.vert[5], ed.vert[6], ed.vert[7]);
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[5], ed.vert[4]);
            writePolygon(ed.vert[1], ed.vert[2], ed.vert[6], ed.vert[5]);
            writePolygon(ed.vert[2], ed.vert[3], ed.vert[7], ed.vert[6]);
            writePolygon(ed.vert[3], ed.vert[0], ed.vert[4], ed.vert[7]);
            break;
        case PWGM_ELEMTYPE_TET:
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[2]);
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[3]);
            writePolygon(ed.vert[1], ed.vert[2], ed.vert[3]);
            writePolygon(ed.vert[2], ed.vert[0], ed.vert[3]);
            break;
        case PWGM_ELEMTYPE_WEDGE:
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[2]);
            writePolygon(ed.vert[3], ed.vert[4], ed.vert[5]);
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[4], ed.vert[3]);
            writePolygon(ed.vert[1], ed.vert[2], ed.vert[5], ed.vert[4]);
            writePolygon(ed.vert[2], ed.vert[0], ed.vert[3], ed.vert[5]);
            break;
        case PWGM_ELEMTYPE_PYRAMID:
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[2], ed.vert[3]);
            writePolygon(ed.vert[0], ed.vert[1], ed.vert[4]);
            writePolygon(ed.vert[1], ed.vert[2], ed.vert[4]);
            writePolygon(ed.vert[2], ed.vert[3], ed.vert[4]);
            writePolygon(ed.vert[3], ed.vert[0], ed.vert[4]);
            break;
    }
}


void
CaeUnsPrint3D::writeHeader()
{
    if (isBinaryEncoding()) {
        // fill with zeros
        memset(curSolidName_, 0, NameBufSize);
        strcpy(curSolidName_, SolidName);
        pwpFileWrite(curSolidName_, 1, 80, fp());
        rtFile_.getPos(numTrisPos_);
        // write placeholder - writeFooter() will replace with final value
        numTris_ = 0;
        rtFile_.write(numTris_);
    }
    else if (!multiSolid_) {
        // ASCII
        strcpy(curSolidName_, SolidName);
        writeStr("solid %s\n", curSolidName_);
    }
}


void
CaeUnsPrint3D::writeFooter()
{
    if (isBinaryEncoding()) {
        // update placeholder with actual tri count
        //if (rtFile_.setPos(numTrisPos_)) {
        //    rtFile_.write(numTris_);
        //}
        // BUG in setPos()!
        if (0 == pwpFileSetpos(fp(), &numTrisPos_)) {
            pwpFileWrite(&numTris_, sizeof(numTris_), 1, fp());
        }
    }
    else if (!multiSolid_) {
        // ASCII
        writeStr("endsolid %s\n", curSolidName_);
    }
}


bool
CaeUnsPrint3D::writePatch(const CaeUnsPatch &patch)
{
    bool ret = false;
    if (!patch.isValid()) {
        // bad
        ret = false;
    }
    else if (isHidden(patch)) {
        // silently skip
        ret = true;
    }
    else {
        PWGM_ELEMDATA eData;
        CaeUnsElement element(patch);
        while (element.data(eData) && progressIncrement()) {
            writeElemData(eData, isSolid(patch));
            ++element;
        }
        ret = !aborted();
    }
    return ret;
}


void
CaeUnsPrint3D::writePatches()
{
    if (aborted()) {
        return;
    }
    PWP_UINT32 steps = 0;
    CaeUnsPatch patch(model_);
    while (patch.isValid()) {
        if (!isHidden(patch)) {
            steps += patch.elementCount();
        }
        ++patch;
    }
    if (progressBeginStep(steps)) {
        patch.moveFirst(model_);
        while (writePatch(patch)) {
            ++patch;
        }
        progressEndStep();
    }
}


bool
CaeUnsPrint3D::writeBlock(const CaeUnsBlock &block)
{
    bool ret = false;
    if (!block.isValid()) {
        // bad
        ret = false;
    }
    else if (isHidden(block)) {
        // silently skip
        ret = true;
    }
    else {
        PWGM_ELEMDATA eData;
        CaeUnsElement element(block);
        while (element.data(eData) && progressIncrement()) {
            writeElemData(eData, isSolid(block));
            ++element;
        }
        ret = !aborted();
    }
    return ret;
}


void
CaeUnsPrint3D::writeBlocks()
{
    if (aborted()) {
        return;
    }
    PWP_UINT32 steps = 0;
    CaeUnsBlock block(model_);
    while (block.isValid()) {
        if (!isHidden(block)) {
            steps += block.elementCount();
        }
        ++block;
    }
    if (progressBeginStep(steps)) {
        block.moveFirst(model_);
        while (writeBlock(block)) {
            ++block;
        }
        progressEndStep();
    }
}



//===========================================================================
// face streaming handlers
//===========================================================================

PWP_UINT32
CaeUnsPrint3D::streamBegin(const PWGM_BEGINSTREAM_DATA &data)
{
    char msg[128];
    sprintf(msg, "STREAM BEGIN: %lu", (unsigned long)data.totalNumFaces);
    sendInfoMsg(msg);
    return 1;
}

PWP_UINT32
CaeUnsPrint3D::streamFace(const PWGM_FACESTREAM_DATA &data)
{
    char msg[128];
    sprintf(msg, "  STREAM FACE: %lu %lu", (unsigned long)data.owner.cellIndex,
        (unsigned long)data.face);
    sendInfoMsg(msg);
    return 1;
}

PWP_UINT32
CaeUnsPrint3D::streamEnd(const PWGM_ENDSTREAM_DATA &data)
{
    char msg[128];
    sprintf(msg, "STREAM END: %s", (data.ok ? "true" : "false"));
    sendInfoMsg(msg);
    return 1;
}


//===========================================================================
// called ONCE when plugin first loaded into memeory
//===========================================================================

bool
CaeUnsPrint3D::create(CAEP_RTITEM &rti)
{
    return
        publishRealValueDef(rti, AttrEdgeDiameter, DefCylDiam,
            "Edge inflation diameter") &&
        publishBoolValueDef(rti, AttrMultiSolid, true,
            "Export inflated edges as individual solid bodies (ASCII only)") &&
        publishUIntValueDef(rti, AttrNumPoints, DefNumBasePts,
            "Number of inflated edge points", MinNumBasePts, MaxNumBasePts);
}


//===========================================================================
// called ONCE just before plugin unloaded from memeory
//===========================================================================

void
CaeUnsPrint3D::destroy(CAEP_RTITEM &rti)
{
    (void)rti.BCCnt; // silence unused arg warning
}
