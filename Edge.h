/****************************************************************************
 *
 * CAEP Plugin example - PwCaeGridWrite implementation
 *
 * Proprietary software product of Pointwise, Inc.
 * Copyright (c) 1995-2014 Pointwise, Inc.
 * All rights reserved.
 *
 ***************************************************************************/

#ifndef _EDGE_H_
#define _EDGE_H_

#include "apiPWP.h"

#include <set>


class Edge {
public:
	Edge(PWP_UINT32 i0, PWP_UINT32 i1);

	~Edge();

	bool operator< (const Edge & rhs) const;

private:
	PWP_UINT32 i0_;
	PWP_UINT32 i1_;
};

typedef std::set<Edge> Edges;

#endif
