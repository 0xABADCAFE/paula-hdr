#ifndef __PAULA_HDR_TABLES__
#define __PAULA_HDR_TABLES__


/**
 * Per sample amplification factors as a function of AUDxVOL-1. This gives the number by which a set of
 * 14-bit sample values are to be multiplied by for normalisation for a given attenuation. For example,
 * a frame of 14-bit samples that hit the maximum range will have amplification factor of 1.0 and be
 * ultimately converted to a full-range 8-bit sample set played at volume 64. A frame of 14-bit samples
 * that only hits half the 14-bit range will be converted into a full range 8-bit sample set played at
 * volume 32.
 */
const float amp_factors[64] = {
  64.00000000, 32.00000000, 21.33333333, 16.00000000, 12.80000000, 10.66666667, 9.14285714, 8.00000000,
   7.11111111,  6.40000000,  5.81818182,  5.33333333,  4.92307692,  4.57142857, 4.26666667, 4.00000000,
   3.76470588,  3.55555556,  3.36842105,  3.20000000,  3.04761905,  2.90909091, 2.78260870, 2.66666667,
   2.56000000,  2.46153846,  2.37037037,  2.28571429,  2.20689655,  2.13333333, 2.06451613, 2.00000000,
   1.93939394,  1.88235294,  1.82857143,  1.77777778,  1.72972973,  1.68421053, 1.64102564, 1.60000000,
   1.56097561,  1.52380952,  1.48837209,  1.45454545,  1.42222222,  1.39130435, 1.36170213, 1.33333333,
   1.30612245,  1.28000000,  1.25490196,  1.23076923,  1.20754717,  1.18518519, 1.16363636, 1.14285714,
   1.12280702,  1.10344828,  1.08474576,  1.06666667,  1.04918033,  1.03225806, 1.01587302, 1.00000000,
};

#endif

