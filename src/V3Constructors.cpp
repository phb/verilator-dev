// -*- mode: C++; c-file-style: "cc-mode" -*-
//*************************************************************************
// DESCRIPTION: Verilator: Emit C++ for tree
//
// Code available from: http://www.veripool.org/verilator
//
//*************************************************************************
//
// Copyright 2003-2016 by Wilson Snyder.  This program is free software; you can
// redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.
//
// Verilator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//*************************************************************************
// Create helper functions for constructing a module.

#include "config_build.h"
#include "verilatedos.h"
#include <cstdio>
#include <cstdarg>
#include <unistd.h>
#include <cmath>
#include <map>
#include <vector>
#include <algorithm>

#include "V3Global.h"
#include "V3Constructors.h"
/* For each module, create a _ctor_var_reset method to reset
 * the state of all variables. This was previously done in V3EmitC directly,
 * but most compiles dislike very large functions and due to heavy inlining,
 * the list of variables to reset can be massive.
 */

class V3Resets {
private:
    AstNodeModule* m_modp;
    AstCFunc*      m_tlFuncp;
    AstCFunc*      m_funcp;
    int		   m_numStmts;
    int 	   m_funcNum;

    void initializeVar(AstVar* nodep) {
	if (v3Global.opt.outputSplitCFuncs()
	    && v3Global.opt.outputSplitCFuncs() < m_numStmts) {
	    // Put every statement into a unique function to ease profiling or reduce function size
	    m_funcp = NULL;
	}
	if (!m_funcp) {
	    m_funcp = new AstCFunc(m_modp->fileline(), "_ctor_var_reset_" + cvtToStr(++m_funcNum), NULL, "void");
	    m_funcp->isStatic(false);
	    m_funcp->declPrivate(true);
	    m_funcp->slow(true);
	    m_modp->addStmtp(m_funcp);

	    // Add a top call to it
	    AstCCall *callp = new AstCCall(m_modp->fileline(), m_funcp);

	    m_tlFuncp->addStmtsp(callp);
	    m_numStmts = 0;
	}
	m_funcp->addStmtsp(new AstCReset(nodep->fileline(), new AstVarRef(nodep->fileline(), nodep, true)));
	m_numStmts += 10;
    }

	public:
    V3Resets(AstNodeModule *nodep) {
    m_modp = nodep;
	m_numStmts = 0;
	m_funcNum = 0;
	m_funcp = NULL;
	m_tlFuncp = new AstCFunc(nodep->fileline(), "_ctor_var_reset", NULL, "void");
	m_tlFuncp->declPrivate(true);
	m_tlFuncp->isStatic(false);
	m_tlFuncp->slow(true);
	m_modp->addStmtp(m_tlFuncp);
	for (AstNode *n = m_modp->stmtsp(); n != NULL; n = n->nextp()) {
		AstVar *varp = n->castVar();
		if (varp) initializeVar(varp); 
    }
}
};

//######################################################################
// EmitC class functions

void V3Constructors::construct() {
    UINFO(2,__FUNCTION__<<": "<<endl);
    for (AstNodeModule* nodep = v3Global.rootp()->modulesp(); nodep; nodep=nodep->nextp()->castNodeModule()) {
    	// Process each module in turn
    	V3Resets v3(nodep);
    }
}