/****************************************************************************
 *
 * Pointwise Plugin utility functions
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2012 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _RTCAEPSUPPORTDATA_H_
#define _RTCAEPSUPPORTDATA_H_

/*------------------------------------*/
/* CaeUnsPrint3D format item setup data */
/*------------------------------------*/
CAEP_BCINFO CaeUnsPrint3DBCInfo[] = {
    { "mesh",   100 },
    { "hidden", 101 },
    { "solid",  102 },
};
/*------------------------------------*/
CAEP_VCINFO CaeUnsPrint3DVCInfo[] = {
    { "mesh",   200 },
    { "hidden", 201 },
    { "solid",  202 },
};
/*------------------------------------*/
const char *CaeUnsPrint3DFileExt[] = {
    "stl"
};

#endif /* _RTCAEPSUPPORTDATA_H_ */
