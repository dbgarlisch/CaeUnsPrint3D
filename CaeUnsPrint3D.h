/****************************************************************************
 *
 * class CaeUnsPrint3D
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2011 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _CAEUNSPRINT3D_H_
#define _CAEUNSPRINT3D_H_

#include "apiGridModel.h"
#include "apiPWP.h"

#include "CaePlugin.h"
#include "CaeUnsGridModel.h"
#include "Edge.h"

#include "cml/cml.h"


#define DefCylDiam      0.9
#define DefNumBasePts   7
#define MinNumBasePts   3
#define MaxNumBasePts   10
#define NameBufSize     81


//////////////////////////////////////////////////////////////////////////
// typedef a 3D CML vector with element type double                     //
//////////////////////////////////////////////////////////////////////////
typedef cml::vector3d vector3;

//////////////////////////////////////////////////////////////////////////
// typedef a 4x4 column-major CML matrix with element type double,      //
// configured for use with column vectors:                              //
//////////////////////////////////////////////////////////////////////////
typedef cml::matrix33d_c matrix33;

typedef vector3 CylBase[MaxNumBasePts];
typedef CylBase Cylinder[2];


//***************************************************************************
//***************************************************************************
//***************************************************************************

class CaeUnsPrint3D :
    public CaeUnsPlugin,
    public CaeFaceStreamHandler {

public:
    CaeUnsPrint3D(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL model,
        const CAEP_WRITEINFO *pWriteInfo);
    ~CaeUnsPrint3D();
    static bool create(CAEP_RTITEM &rti);
    static void destroy(CAEP_RTITEM &rti);

private:

    void    writeStr(const char *format, ...);
    void    writeXyz(const char prefix[], const vector3 &xyz);
    void    writeAttrByteCount(PWP_UINT16 byteCnt = 0);
    void    writeTriFacet(const vector3 &p0, const vector3 &p1,
                const vector3 &p2);
    void    writeQuadFacet(const vector3 &p0, const vector3 &p1,
                const vector3 &p2, const vector3 &p3);
    void    beginMultiSolid();
    void    endMultiSolid();
    void    makeCylinder(const matrix33 &rot, const vector3 &tran0,
                const vector3 &tran1, Cylinder &cyl);
    void    writeCylBase(const CylBase &base, bool reverse);
    void    writeCylSides(const CylBase &cb0, const CylBase &cb1);
    void    writeCylinder(const vector3 &p0, const vector3 &p1);
    void    writeCylinder(const PWGM_VERTDATA &vd0, const PWGM_VERTDATA &vd1);
    bool    isNewEdge(const Edge &e);
    void    writeEdge(const CaeUnsVertex &v0, const CaeUnsVertex &v1);
    void    writePolygon(const CaeUnsVertex &v0, const CaeUnsVertex &v1,
                const CaeUnsVertex &v2, const CaeUnsVertex &v3);
    void    writePolygon(const CaeUnsVertex &v0, const CaeUnsVertex &v1,
                const CaeUnsVertex &v2);
    void    writeThickenedPolygon(const vector3 &tp0, const vector3 &tp1,
                const vector3 &tp2);
    void    writeThickenedPolygon(const CaeUnsVertex &v0,
                const CaeUnsVertex &v1, const CaeUnsVertex &v2);
    void    writeThickenedPolygon(const vector3 &qp0, const vector3 &qp1,
                const vector3 &qp2, const vector3 &qp3);
    void    writeThickenedPolygon(const CaeUnsVertex &v0,
                const CaeUnsVertex &v1, const CaeUnsVertex &v2,
                const CaeUnsVertex &v3);
    void    writeElemData(PWGM_ELEMDATA &ed, bool solid = false);
    void    writeHeader();
    void    writeFooter();
    bool    writePatch(const CaeUnsPatch &patch);
    void    writePatches();
    bool    writeBlock(const CaeUnsBlock &block);
    void    writeBlocks();

    virtual bool        beginExport();
    virtual PWP_BOOL    write();
    virtual bool        endExport();

    // face streaming handlers
    virtual PWP_UINT32 streamBegin(const PWGM_BEGINSTREAM_DATA &data);
    virtual PWP_UINT32 streamFace(const PWGM_FACESTREAM_DATA &data);
    virtual PWP_UINT32 streamEnd(const PWGM_ENDSTREAM_DATA &data);

private:
    Edges           edges_;
    sysFILEPOS      numTrisPos_;
    PWP_UINT32      numTris_;
    PWP_UINT32      numSolids_;
    char            curSolidName_[NameBufSize];
    bool            multiSolid_;
    CylBase         masterCylBase_;
    double          radius_;
    double          zOffset_;
    PWP_UINT        numBasePts_;
};

#endif // _CAEUNSPRINT3D_H_
