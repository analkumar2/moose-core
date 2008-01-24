/*******************************************************************
 * File:            pymoose/BinomialRng.cpp
 * Description:      SWIG wrapper class for BinomialRng. This class
 *                      deviates from the general pattern of pymoose
 *                      classes. It has been manually modified from
 *                      the generated class definition.
 * Author:          Subhasis Ray
 * E-mail:          ray.subhasis@gmail.com
 * Created:         2007-11-30 20:20:09
 ********************************************************************/
/**********************************************************************
** This program is part of 'MOOSE', the
** Messaging Object Oriented Simulation Environment,
** also known as GENESIS 3 base code.
**           copyright (C) 2003-2005 Upinder S. Bhalla. and NCBS
** It is made available under the terms of the
** GNU General Public License version 2
** See the file COPYING.LIB for the full notice.
**********************************************************************/
#ifndef _pymoose_BinomialRng_cpp
#define _pymoose_BinomialRng_cpp
#include "BinomialRng.h"
using namespace pymoose;
const std::string BinomialRng::className = "BinomialRng";
BinomialRng::BinomialRng(Id id):RandGenerator(id){}
BinomialRng::BinomialRng(std::string path):RandGenerator(className, path){}
BinomialRng::BinomialRng(std::string name, Id parentId):RandGenerator(className, name, parentId){}
BinomialRng::BinomialRng(std::string name, PyMooseBase& parent):RandGenerator(className, name, parent){}
BinomialRng::~BinomialRng(){}
const std::string& BinomialRng::getType(){ return className; }
int BinomialRng::__get_n() const
{
    int n;
    get < int > (id_(), "n",n);
    return n;
}
void BinomialRng::__set_n( int n )
{
    set < int > (id_(), "n", n);
}
double BinomialRng::__get_p() const
{
    double p;
    get < double > (id_(), "p",p);
    return p;
}
void BinomialRng::__set_p( double p )
{
    set < double > (id_(), "p", p);
}
#endif
