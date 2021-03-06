#ifndef RS_H
#define RS_H

/* This program is an encoder/decoder for Reed-Solomon codes. Encoding is in
   systematic form, decoding via the Berlekamp iterative algorithm.
   In the present form , the constants mm, nn, tt, and kk=nn-2tt must be
   specified  (the double letters are used simply to avoid clashes with
   other n,k,t used in other programs into which this was incorporated!)
   Also, the irreducible polynomial used to generate GF(2**mm) must also be
   entered -- these can be found in Lin and Costello, and also Clark and Cain.

   The representation of the elements of GF(2**m) is either in index form,
   where the number is the power of the primitive element alpha, which is
   convenient for multiplication (add the powers modulo 2**m-1) or in
   polynomial form, where the bits represent the coefficients of the
   polynomial representation of the number, which is the most convenient form
   for addition.  The two forms are swapped between via lookup tables.
   This leads to fairly messy looking expressions, but unfortunately, there
   is no easy alternative when working with Galois arithmetic.

   The code is not written in the most elegant way, but to the best
   of my knowledge, (no absolute guarantees!), it works.
   However, when including it into a simulation program, you may want to do
   some conversion of global variables (used here because I am lazy!) to
   local variables where appropriate, and passing parameters (eg array
   addresses) to the functions  may be a sensible move to reduce the number
   of global variables and thus decrease the chance of a bug being introduced.

   This program does not handle erasures at present, but should not be hard
   to adapt to do this, as it is just an adjustment to the Berlekamp-Massey
   algorithm. It also does not attempt to decode past the BCH bound -- see
   Blahut "Theory and practice of error control codes" for how to do this.

              Simon Rockliff, University of Adelaide   21/9/89

   26/6/91 Slight modifications to remove a compiler dependent bug which hadn't
           previously surfaced. A few extra comments added for clarity.
           Appears to all work fine, ready for posting to net!

                  Notice
                 --------
   This program may be freely modified and/or given to whoever wants it.
   A condition of such distribution is that the author's contribution be
   acknowledged by his name being left in the comments heading the program,
   however no responsibility is accepted for any financial or other loss which
   may result from some unforeseen errors or malfunctioning of the program
   during use.
                                 Simon Rockliff, 26th June 1991
*/

/*
 * NOTE: encode_rs() and decode_rs() have been split into two header files
 * because the target devices need separate compilation.
 */

#define mm  8       /* RS code over GF(2**8) - change to suit */
#define nn  255     /* nn=2**mm -1   length of codeword */
#define tt  16      /* number of errors that can be corrected */
#define kk  223     /* kk = nn-2*tt  */

/*
 * The following variables will not be used simultaneously and can therefore be
 * defined globally in order to save execution time by not having to allocate
 * and deallocate frequently (mainly applies to recd).
 *
 * This is a (albeit highly simplified) method of static memory allocation.
 */
short pp[mm+1] = { 1, 0, 1, 1, 1, 0, 0, 0, 1 }; /* primitive polynomial p(x) = 1+x^2+x^3+x^4+x^8 */
short alpha_to[nn+1], index_of[nn+1], gg[nn-kk+1], recd[nn];

/* generate GF(2**mm) from the irreducible polynomial p(X) in pp[0]..pp[mm]
   lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
                   polynomial form -> index form  index_of[j=alpha**i] = i
   alpha=2 is the primitive element of GF(2**mm)
*/
void generate_gf()
{
    register int i, mask ;

    mask = 1 ;
    alpha_to[mm] = 0 ;
    for (i=0; i<mm; i++)
    { alpha_to[i] = mask ;
        index_of[alpha_to[i]] = i ;
        if (pp[i]!=0)
            alpha_to[mm] ^= mask ;
        mask <<= 1 ;
    }
    index_of[alpha_to[mm]] = mm ;
    mask >>= 1 ;
    for (i=mm+1; i<nn; i++)
    {
        if (alpha_to[i-1] >= mask)
            alpha_to[i] = alpha_to[mm] ^ ((alpha_to[i-1]^mask)<<1) ;
        else alpha_to[i] = alpha_to[i-1]<<1 ;
        index_of[alpha_to[i]] = i ;
    }
    index_of[0] = -1 ;


}

/* Obtain the generator polynomial of the tt-error correcting, length
  nn=(2**mm -1) Reed Solomon code  from the product of (X+alpha**i), i=1..2*tt
*/
void gen_poly()
{
    register int i,j ;

    gg[0] = 2 ;    /* primitive element alpha = 2  for GF(2**mm)  */
    gg[1] = 1 ;    /* g(x) = (X+alpha) initially */
    for (i=2; i<=nn-kk; i++)
    { gg[i] = 1 ;
        for (j=i-1; j>0; j--)
            if (gg[j] != 0)  gg[j] = gg[j-1]^ alpha_to[(index_of[gg[j]]+i)%nn] ;
            else gg[j] = gg[j-1] ;
        gg[0] = alpha_to[(index_of[gg[0]]+i)%nn] ;     /* gg[0] can never be zero */
    }
    /* convert gg[] to index form for quicker encoding */
    for (i=0; i<=nn-kk; i++)  gg[i] = index_of[gg[i]] ;
}

#endif //RS_H
