/****************************************************************************
 *
 * c++ implementation of runtimeWrite(), runtimeCreate(), and runtimeDestroy()
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2011 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

/* DO NOT EDIT THIS FILE!

 * To make plugin specific changes, you should edit CaeUnsPrint3D.cxx

 * DO NOT EDIT THIS FILE!
 */

#include "apiCAEP.h"
#include "apiCAEPUtils.h"
#include "apiGridModel.h"
#include "apiPWP.h"
#include "runtimeWrite.h"

#include "CaeUnsPrint3D.h"


/**************************************/
PWP_BOOL
runtimeWrite(CAEP_RTITEM *pRti, PWGM_HGRIDMODEL model,
    const CAEP_WRITEINFO *pWriteInfo)
{
    CaeUnsPrint3D plugin(pRti, model, pWriteInfo);
    return plugin.run();
}

PWP_BOOL
runtimeCreate(CAEP_RTITEM *pRti)
{
    return CaeUnsPrint3D::create(*pRti);
}

PWP_VOID
runtimeDestroy(CAEP_RTITEM *pRti)
{
    return CaeUnsPrint3D::destroy(*pRti);
}