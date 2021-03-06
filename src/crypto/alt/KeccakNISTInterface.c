/*
The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
Michaël Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/

Implementation by the designers,
hereby denoted as "the implementer".

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#include <string.h>
#include "KeccakNISTInterface.h"
#include "KeccakF-1600-interface.h"

HashReturn Init(hashState *state, int hashbitlen)
{
    switch(hashbitlen) {
        case 0: // Default parameters, arbitrary length output
            InitSponge((spongeState*)state, 1024, 576);
            break;
        case 224:
            InitSponge((spongeState*)state, 1152, 448);
            break;
        case 256:
            InitSponge((spongeState*)state, 1088, 512);
            break;
        case 384:
            InitSponge((spongeState*)state, 832, 768);
            break;
        case 512:
            InitSponge((spongeState*)state, 576, 1024);
            break;
        default:
            return BAD_HASHLEN;
    }
    state->fixedOutputLength = hashbitlen;
    return SUCCESS;
}

HashReturn Update(hashState *state, const BitSequence *data, DataLength databitlen, const UINT64* pscratchpd, UINT64 pscratchpd_sz)
{
    if ((databitlen % 8) == 0)
        return (HashReturn)Absorb((spongeState*)state, data, databitlen, pscratchpd, pscratchpd_sz);
    else {
        HashReturn ret = (HashReturn)Absorb((spongeState*)state, data, databitlen - (databitlen % 8), pscratchpd, pscratchpd_sz);
        if (ret == SUCCESS) {
            unsigned char lastByte; 
            // Align the last partial byte to the least significant bits
            lastByte = data[databitlen/8] >> (8 - (databitlen % 8));
            return (HashReturn)Absorb((spongeState*)state, &lastByte, databitlen % 8, pscratchpd, pscratchpd_sz);
        }
        else
            return ret;
    }
}

HashReturn Final(hashState *state, BitSequence *hashval, const UINT64* pscratchpd, UINT64 pscratchpd_sz)
{
    return (HashReturn)Squeeze(state, hashval, state->fixedOutputLength, pscratchpd, pscratchpd_sz);
}

HashReturn Hash(int hashbitlen, const BitSequence *data, DataLength databitlen, BitSequence *hashval, const UINT64* pscratchpd, UINT64 pscratchpd_sz)
{
    hashState state;
    HashReturn result;

    if ((hashbitlen != 224) && (hashbitlen != 256) && (hashbitlen != 384) && (hashbitlen != 512))
        return BAD_HASHLEN; // Only the four fixed output lengths available through this API
    result = Init(&state, hashbitlen);
    if (result != SUCCESS)
        return result;
    result = Update(&state, data, databitlen, pscratchpd, pscratchpd_sz);
    if (result != SUCCESS)
        return result;
    result = Final(&state, hashval, pscratchpd, pscratchpd_sz);
    return result;
}

