/** operators : Basic arithmetic operation using INFTY numbers
 * 
 * David Coeurjolly (david.coeurjolly@liris.cnrs.fr) - Sept. 2004
 *
**/

#ifndef __OPERATORS_H
#define __OPERATORS_H


#define INFTY 100000001

// The sum of a and b handling INFTY
long sum(long a, long b);

//The product of a and b handling INFTY
long prod(long a, long b);

//The opposite of a  handling INFTY
long opp (long a);

// The division (integer) of divid out of divis handling INFTY
long intdivint (long divid, long divis);

#endif
