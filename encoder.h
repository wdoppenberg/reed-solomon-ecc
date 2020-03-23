#ifndef ENCODER_H
#define ENCODER_H

#include "transfer.h"

/* take the string of symbols in data[i], i=0..(k-1) and encode systematically
   to produce 2*tt parity symbols in bb[0]..bb[2*tt-1]
   data[] is input and bb[] is output in polynomial form.
   Encoding is done by using a feedback shift register with appropriate
   connections specified by the elements of gg[], which was generated above.
   Codeword is   c(X) = data(X)*X**(nn-kk)+ b(X)          */
void encode_rs(packet_t * packet)
{
    register int i,j ;
    int feedback ;

    for (i=0; i<nn-kk; i++)   packet->ECF[i] = 0 ;
        for (i=kk-1; i>=0; i--)
            {  feedback = index_of[packet->data[i]^packet->ECF[nn-kk-1]] ;
                if (feedback != -1)
                    { for (j=nn-kk-1; j>0; j--)
                        if (gg[j] != -1)
                            packet->ECF[j] = packet->ECF[j-1]^alpha_to[(gg[j]+feedback)%nn] ;
                        else
                            packet->ECF[j] = packet->ECF[j-1] ;
                        packet->ECF[0] = alpha_to[(gg[0]+feedback)%nn] ;
                    }
                else
                    { for (j=nn-kk-1; j>0; j--)
                            packet->ECF[j] = packet->ECF[j-1] ;
                        packet->ECF[0] = 0 ;
                    }
            }
}

#endif //ENCODER_H