
/** operators : Basic arithmetic operation using INFTY numbers
 * 
 * David Coeurjolly (david.coeurjolly@liris.cnrs.fr) - Sept. 2004
 *
**/

#include "operators.h"



/////////Basic functions to handle operations with INFTY

/** 
 **************************************************
 * @b sum
 * @param a Long number with INFTY
 * @param b Long number with INFTY
 * @return The sum of a and b handling INFTY
 **************************************************/
long sum(long a, long b) 
{
  if ((a==INFTY) || (b==INFTY))     
    return INFTY;    
  else 
    return a+b;
}

/** 
 **************************************************
 * @b prod
 * @param a Long number with INFTY
 * @param b Long number with INFTY
 * @return The product of a and b handling INFTY
 **************************************************/
long prod(long a, long b) 
{
  if ((a==INFTY) || (b==INFTY)) 
    return INFTY;  
  else 
    return a*b;
}
/** 
 **************************************************
 * @b opp
 * @param a Long number with INFTY
 * @return The opposite of a  handling INFTY
 **************************************************/
long opp (long a) {
  if (a == INFTY) {
    return INFTY;
  }
  else {
    return -a;
  }
}

/** 
 **************************************************
 * @b intdivint
 * @param divid Long number with INFTY
 * @param divis Long number with INFTY
 * @return The division (integer) of divid out of divis handling INFTY
 **************************************************/
long intdivint (long divid, long divis) {
  if (divis == 0) 
    return  INFTY;
  if (divid == INFTY) 
    return  INFTY;
  else 
    return  divid / divis;
}
//////////

/**
 =================================================
 * @file   operators.cc
 * @author David COEURJOLLY <David Coeurjolly <dcoeurjo@liris.cnrs.fr>>
 * @date   Thu Sep 30 09:22:19 2004
 * 
 * @brief  Basic implementation of arithmetical operations using INFTY numbers.
 * 
 * 
 =================================================*/
