/************************************************************************/
/*                                                                      */
/*               Copyright 2009-2010 by Ullrich Koethe                  */
/*                                                                      */
/*    This file is part of the VIGRA computer vision library.           */
/*    The VIGRA Website is                                              */
/*        http://hci.iwr.uni-heidelberg.de/vigra/                       */
/*    Please direct questions, bug reports, and contributions to        */
/*        ullrich.koethe@iwr.uni-heidelberg.de    or                    */
/*        vigra@informatik.uni-hamburg.de                               */
/*                                                                      */
/*    Permission is hereby granted, free of charge, to any person       */
/*    obtaining a copy of this software and associated documentation    */
/*    files (the "Software"), to deal in the Software without           */
/*    restriction, including without limitation the rights to use,      */
/*    copy, modify, merge, publish, distribute, sublicense, and/or      */
/*    sell copies of the Software, and to permit persons to whom the    */
/*    Software is furnished to do so, subject to the following          */
/*    conditions:                                                       */
/*                                                                      */
/*    The above copyright notice and this permission notice shall be    */
/*    included in all copies or substantial portions of the             */
/*    Software.                                                         */
/*                                                                      */
/*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND    */
/*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES   */
/*    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND          */
/*    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT       */
/*    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,      */
/*    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      */
/*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR     */
/*    OTHER DEALINGS IN THE SOFTWARE.                                   */
/*                                                                      */
/************************************************************************/

#ifndef VIGRA_MULTI_FFT_HXX
#define VIGRA_MULTI_FFT_HXX

#include "fftw3.hxx"
#include "multi_array.hxx"
#include "navigator.hxx"
#include "copyimage.hxx"

namespace vigra {

/********************************************************/
/*                                                      */
/*                    Fourier Transform                 */
/*                                                      */
/********************************************************/

/** \addtogroup FourierTransform 
*/
//@{

namespace detail {

template <unsigned int N, class T, class C>
void moveDCToCenterImpl(MultiArrayView<N, T, C> a, unsigned int startDimension)
{
    typedef typename MultiArrayView<N, T, C>::traverser Traverser;
    typedef MultiArrayNavigator<Traverser, N> Navigator;
    typedef typename Navigator::iterator Iterator;
    
    for(unsigned int d = startDimension; d < N; ++d)
    {
        Navigator nav(a.traverser_begin(), a.shape(), d);

        for( ; nav.hasMore(); nav++ )
        {
            Iterator i = nav.begin();
            int s  = nav.end() - i;
            int s2 = s/2;
                
            if(even(s))
            {
                for(int k=0; k<s2; ++k)
                {
                    std::swap(i[k], i[k+s2]);
                }
            }
            else            
            {
                T v = i[0];
                for(int k=0; k<s2; ++k)
                {
                    i[k] = i[k+s2+1];
                    i[k+s2+1] = i[k+1];
                }
                i[s2] = v;
            }
        }
    }
}

template <unsigned int N, class T, class C>
void moveDCToUpperLeftImpl(MultiArrayView<N, T, C> a, unsigned int startDimension)
{
    typedef typename MultiArrayView<N, T, C>::traverser Traverser;
    typedef MultiArrayNavigator<Traverser, N> Navigator;
    typedef typename Navigator::iterator Iterator;
    
    for(unsigned int d = startDimension; d < N; ++d)
    {
        Navigator nav(a.traverser_begin(), a.shape(), d);

        for( ; nav.hasMore(); nav++ )
        {
            Iterator i = nav.begin();
            int s  = nav.end() - i;
            int s2 = s/2;
            
            if(even(s))
            {
                for(int k=0; k<s2; ++k)
                {
                    std::swap(i[k], i[k+s2]);
                }
            }
            else            
            {
                T v = i[s2];
                for(int k=s2; k>0; --k)
                {
                    i[k] = i[k+s2];
                    i[k+s2] = i[k-1];
                }
                i[0] = v;
            }
        }
    }
}

} // namespace detail

/********************************************************/
/*                                                      */
/*                     moveDCToCenter                   */
/*                                                      */
/********************************************************/

template <unsigned int N, class T, class C>
inline void moveDCToCenter(MultiArrayView<N, T, C> a)
{
    detail::moveDCToCenterImpl(a, 0);
}

template <unsigned int N, class T, class C>
inline void moveDCToUpperLeft(MultiArrayView<N, T, C> a)
{
    detail::moveDCToUpperLeftImpl(a, 0);
}

template <unsigned int N, class T, class C>
inline void moveDCToHalfspaceCenter(MultiArrayView<N, T, C> a)
{
    detail::moveDCToCenterImpl(a, 1);
}

template <unsigned int N, class T, class C>
inline void moveDCToHalfspaceUpperLeft(MultiArrayView<N, T, C> a)
{
    detail::moveDCToUpperLeftImpl(a, 1);
}

namespace detail
{

inline fftw_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               FFTWComplex<double> * in,  int* instrides,  int instep,
               FFTWComplex<double> * out, int* outstrides, int outstep,
               int sign, unsigned int planner_flags)
{
    return fftw_plan_many_dft(N, shape, 1,
                              (fftw_complex *)in, instrides, instep, 0,
                              (fftw_complex *)out, outstrides, outstep, 0,
                              sign, planner_flags);
}

inline fftw_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               double * in,  int* instrides,  int instep,
               FFTWComplex<double> * out, int* outstrides, int outstep,
               int /*sign is ignored*/, unsigned int planner_flags)
{
    return fftw_plan_many_dft_r2c(N, shape, 1,
                                   in, instrides, instep, 0,
                                   (fftw_complex *)out, outstrides, outstep, 0,
                                   planner_flags);
}

inline fftw_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               FFTWComplex<double> * in,  int* instrides,  int instep,
               double * out, int* outstrides, int outstep,
               int /*sign is ignored*/, unsigned int planner_flags)
{
    return fftw_plan_many_dft_c2r(N, shape, 1,
                                  (fftw_complex *)in, instrides, instep, 0,
                                  out, outstrides, outstep, 0,
                                  planner_flags);
}

inline fftwf_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               FFTWComplex<float> * in,  int* instrides,  int instep,
               FFTWComplex<float> * out, int* outstrides, int outstep,
               int sign, unsigned int planner_flags)
{
    return fftwf_plan_many_dft(N, shape, 1,
                               (fftwf_complex *)in, instrides, instep, 0,
                               (fftwf_complex *)out, outstrides, outstep, 0,
                               sign, planner_flags);
}

inline fftwf_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               float * in,  int* instrides,  int instep,
               FFTWComplex<float> * out, int* outstrides, int outstep,
               int /*sign is ignored*/, unsigned int planner_flags)
{
    return fftwf_plan_many_dft_r2c(N, shape, 1,
                                    in, instrides, instep, 0,
                                    (fftwf_complex *)out, outstrides, outstep, 0,
                                    planner_flags);
}

inline fftwf_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               FFTWComplex<float> * in,  int* instrides,  int instep,
               float * out, int* outstrides, int outstep,
               int /*sign is ignored*/, unsigned int planner_flags)
{
    return fftwf_plan_many_dft_c2r(N, shape, 1,
                                   (fftwf_complex *)in, instrides, instep, 0,
                                   out, outstrides, outstep, 0,
                                   planner_flags);
}

inline fftwl_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               FFTWComplex<long double> * in,  int* instrides,  int instep,
               FFTWComplex<long double> * out, int* outstrides, int outstep,
               int sign, unsigned int planner_flags)
{
    return fftwl_plan_many_dft(N, shape, 1,
                               (fftwl_complex *)in, instrides, instep, 0,
                               (fftwl_complex *)out, outstrides, outstep, 0,
                               sign, planner_flags);
}

inline fftwl_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               long double * in,  int* instrides,  int instep,
               FFTWComplex<long double> * out, int* outstrides, int outstep,
               int /*sign is ignored*/, unsigned int planner_flags)
{
    return fftwl_plan_many_dft_r2c(N, shape, 1,
                                    in, instrides, instep, 0,
                                    (fftwl_complex *)out, outstrides, outstep, 0,
                                    planner_flags);
}

inline fftwl_plan 
fftwPlanCreate(unsigned int N, int* shape, 
               FFTWComplex<long double> * in,  int* instrides,  int instep,
               long double * out, int* outstrides, int outstep,
               int /*sign is ignored*/, unsigned int planner_flags)
{
    return fftwl_plan_many_dft_c2r(N, shape, 1,
                                   (fftwl_complex *)in, instrides, instep, 0,
                                   out, outstrides, outstep, 0,
                                   planner_flags);
}

inline void fftwPlanDestroy(fftw_plan plan)
{
    if(plan != 0)
        fftw_destroy_plan(plan);
}

inline void fftwPlanDestroy(fftwf_plan plan)
{
    if(plan != 0)
        fftwf_destroy_plan(plan);
}

inline void fftwPlanDestroy(fftwl_plan plan)
{
    if(plan != 0)
        fftwl_destroy_plan(plan);
}

inline void 
fftwPlanExecute(fftw_plan plan) 
{
    fftw_execute(plan);
}

inline void 
fftwPlanExecute(fftwf_plan plan) 
{
    fftwf_execute(plan);
}

inline void 
fftwPlanExecute(fftwl_plan plan) 
{
    fftwl_execute(plan);
}

inline void 
fftwPlanExecute(fftw_plan plan, FFTWComplex<double> * in,  FFTWComplex<double> * out) 
{
    fftw_execute_dft(plan, (fftw_complex *)in, (fftw_complex *)out);
}

inline void 
fftwPlanExecute(fftw_plan plan, double * in,  FFTWComplex<double> * out) 
{
    fftw_execute_dft_r2c(plan, in, (fftw_complex *)out);
}

inline void 
fftwPlanExecute(fftw_plan plan, FFTWComplex<double> * in,  double * out) 
{
    fftw_execute_dft_c2r(plan, (fftw_complex *)in, out);
}

inline void 
fftwPlanExecute(fftwf_plan plan, FFTWComplex<float> * in,  FFTWComplex<float> * out) 
{
    fftwf_execute_dft(plan, (fftwf_complex *)in, (fftwf_complex *)out);
}

inline void 
fftwPlanExecute(fftwf_plan plan, float * in,  FFTWComplex<float> * out) 
{
    fftwf_execute_dft_r2c(plan, in, (fftwf_complex *)out);
}

inline void 
fftwPlanExecute(fftwf_plan plan, FFTWComplex<float> * in,  float * out) 
{
    fftwf_execute_dft_c2r(plan, (fftwf_complex *)in, out);
}

inline void 
fftwPlanExecute(fftwl_plan plan, FFTWComplex<long double> * in,  FFTWComplex<long double> * out) 
{
    fftwl_execute_dft(plan, (fftwl_complex *)in, (fftwl_complex *)out);
}

inline void 
fftwPlanExecute(fftwl_plan plan, long double * in,  FFTWComplex<long double> * out) 
{
    fftwl_execute_dft_r2c(plan, in, (fftwl_complex *)out);
}

inline void 
fftwPlanExecute(fftwl_plan plan, FFTWComplex<long double> * in,  long double * out) 
{
    fftwl_execute_dft_c2r(plan, (fftwl_complex *)in, out);
}

template <int DUMMY>
struct FFTWPaddingSize
{
    static const int size = 1330;
    static const int evenSize = 1063;
    static int goodSizes[size];
    static int goodEvenSizes[evenSize];
    
    static inline int find(int s)
    {
        if(s <= 0 || s >= goodSizes[size-1])
            return s;
        // find the smallest padding size that is at least as large as 's'
        int * upperBound = std::upper_bound(goodSizes, goodSizes+size, s);
        if(upperBound > goodSizes && upperBound[-1] == s)
            return s;
        else
            return *upperBound;
    }

    static inline int findEven(int s)
    {
        if(s <= 0 || s >= goodEvenSizes[evenSize-1])
            return s;
        // find the smallest padding size that is at least as large as 's'
        int * upperBound = std::upper_bound(goodEvenSizes, goodEvenSizes+evenSize, s);
        if(upperBound > goodEvenSizes && upperBound[-1] == s)
            return s;
        else
            return *upperBound;
    }
};
    
    // Image sizes where FFTW is fast. The list contains all
    // numbers less than 100000 whose prime decomposition is of the form
    // 2^a*3^b*5^c*7^d*11^e*13^f, where e+f is either 0 or 1, and the 
    // other exponents are arbitrary
template <int DUMMY>
int FFTWPaddingSize<DUMMY>::goodSizes[size] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 
        18, 20, 21, 22, 24, 25, 26, 27, 28, 30, 32, 33, 35, 36, 39, 40, 42, 44, 45, 48, 
        49, 50, 52, 54, 55, 56, 60, 63, 64, 65, 66, 70, 72, 75, 77, 78, 80, 81, 
        84, 88, 90, 91, 96, 98, 99, 100, 104, 105, 108, 110, 112, 117, 120, 125, 
        126, 128, 130, 132, 135, 140, 144, 147, 150, 154, 156, 160, 162, 165, 
        168, 175, 176, 180, 182, 189, 192, 195, 196, 198, 200, 208, 210, 216, 
        220, 224, 225, 231, 234, 240, 243, 245, 250, 252, 256, 260, 264, 270, 
        273, 275, 280, 288, 294, 297, 300, 308, 312, 315, 320, 324, 325, 330, 
        336, 343, 350, 351, 352, 360, 364, 375, 378, 384, 385, 390, 392, 396, 
        400, 405, 416, 420, 432, 440, 441, 448, 450, 455, 462, 468, 480, 486, 
        490, 495, 500, 504, 512, 520, 525, 528, 539, 540, 546, 550, 560, 567, 
        576, 585, 588, 594, 600, 616, 624, 625, 630, 637, 640, 648, 650, 660, 
        672, 675, 686, 693, 700, 702, 704, 720, 728, 729, 735, 750, 756, 768, 
        770, 780, 784, 792, 800, 810, 819, 825, 832, 840, 864, 875, 880, 882, 
        891, 896, 900, 910, 924, 936, 945, 960, 972, 975, 980, 990, 1000, 1008, 
        1024, 1029, 1040, 1050, 1053, 1056, 1078, 1080, 1092, 1100, 1120, 1125, 
        1134, 1152, 1155, 1170, 1176, 1188, 1200, 1215, 1225, 1232, 1248, 1250, 
        1260, 1274, 1280, 1296, 1300, 1320, 1323, 1344, 1350, 1365, 1372, 1375, 
        1386, 1400, 1404, 1408, 1440, 1456, 1458, 1470, 1485, 1500, 1512, 1536, 
        1540, 1560, 1568, 1575, 1584, 1600, 1617, 1620, 1625, 1638, 1650, 1664, 
        1680, 1701, 1715, 1728, 1750, 1755, 1760, 1764, 1782, 1792, 1800, 1820, 
        1848, 1872, 1875, 1890, 1911, 1920, 1925, 1944, 1950, 1960, 1980, 2000, 
        2016, 2025, 2048, 2058, 2079, 2080, 2100, 2106, 2112, 2156, 2160, 2184, 
        2187, 2200, 2205, 2240, 2250, 2268, 2275, 2304, 2310, 2340, 2352, 2376, 
        2400, 2401, 2430, 2450, 2457, 2464, 2475, 2496, 2500, 2520, 2548, 2560, 
        2592, 2600, 2625, 2640, 2646, 2673, 2688, 2695, 2700, 2730, 2744, 2750, 
        2772, 2800, 2808, 2816, 2835, 2880, 2912, 2916, 2925, 2940, 2970, 3000, 
        3024, 3072, 3080, 3087, 3120, 3125, 3136, 3150, 3159, 3168, 3185, 3200, 
        3234, 3240, 3250, 3276, 3300, 3328, 3360, 3375, 3402, 3430, 3456, 3465, 
        3500, 3510, 3520, 3528, 3564, 3584, 3600, 3640, 3645, 3675, 3696, 3744, 
        3750, 3773, 3780, 3822, 3840, 3850, 3888, 3900, 3920, 3960, 3969, 4000, 
        4032, 4050, 4095, 4096, 4116, 4125, 4158, 4160, 4200, 4212, 4224, 4312, 
        4320, 4368, 4374, 4375, 4400, 4410, 4455, 4459, 4480, 4500, 4536, 4550, 
        4608, 4620, 4680, 4704, 4725, 4752, 4800, 4802, 4851, 4860, 4875, 4900, 
        4914, 4928, 4950, 4992, 5000, 5040, 5096, 5103, 5120, 5145, 5184, 5200, 
        5250, 5265, 5280, 5292, 5346, 5376, 5390, 5400, 5460, 5488, 5500, 5544, 
        5600, 5616, 5625, 5632, 5670, 5733, 5760, 5775, 5824, 5832, 5850, 5880, 
        5940, 6000, 6048, 6075, 6125, 6144, 6160, 6174, 6237, 6240, 6250, 6272, 
        6300, 6318, 6336, 6370, 6400, 6468, 6480, 6500, 6552, 6561, 6600, 6615, 
        6656, 6720, 6750, 6804, 6825, 6860, 6875, 6912, 6930, 7000, 7020, 7040, 
        7056, 7128, 7168, 7200, 7203, 7280, 7290, 7350, 7371, 7392, 7425, 7488, 
        7500, 7546, 7560, 7644, 7680, 7700, 7776, 7800, 7840, 7875, 7920, 7938, 
        8000, 8019, 8064, 8085, 8100, 8125, 8190, 8192, 8232, 8250, 8316, 8320, 
        8400, 8424, 8448, 8505, 8575, 8624, 8640, 8736, 8748, 8750, 8775, 8800, 
        8820, 8910, 8918, 8960, 9000, 9072, 9100, 9216, 9240, 9261, 9360, 9375, 
        9408, 9450, 9477, 9504, 9555, 9600, 9604, 9625, 9702, 9720, 9750, 9800, 
        9828, 9856, 9900, 9984, 10000, 10080, 10125, 10192, 10206, 10240, 10290, 
        10368, 10395, 10400, 10500, 10530, 10560, 10584, 10692, 10752, 10780, 
        10800, 10920, 10935, 10976, 11000, 11025, 11088, 11200, 11232, 11250, 
        11264, 11319, 11340, 11375, 11466, 11520, 11550, 11648, 11664, 11700, 
        11760, 11880, 11907, 12000, 12005, 12096, 12150, 12250, 12285, 12288, 
        12320, 12348, 12375, 12474, 12480, 12500, 12544, 12600, 12636, 12672, 
        12740, 12800, 12936, 12960, 13000, 13104, 13122, 13125, 13200, 13230, 
        13312, 13365, 13377, 13440, 13475, 13500, 13608, 13650, 13720, 13750, 
        13824, 13860, 14000, 14040, 14080, 14112, 14175, 14256, 14336, 14400, 
        14406, 14553, 14560, 14580, 14625, 14700, 14742, 14784, 14850, 14976, 
        15000, 15092, 15120, 15288, 15309, 15360, 15400, 15435, 15552, 15600, 
        15625, 15680, 15750, 15795, 15840, 15876, 15925, 16000, 16038, 16128, 
        16170, 16200, 16250, 16380, 16384, 16464, 16500, 16632, 16640, 16800, 
        16807, 16848, 16875, 16896, 17010, 17150, 17199, 17248, 17280, 17325, 
        17472, 17496, 17500, 17550, 17600, 17640, 17820, 17836, 17920, 18000, 
        18144, 18200, 18225, 18375, 18432, 18480, 18522, 18711, 18720, 18750, 
        18816, 18865, 18900, 18954, 19008, 19110, 19200, 19208, 19250, 19404, 
        19440, 19500, 19600, 19656, 19683, 19712, 19800, 19845, 19968, 20000, 
        20160, 20250, 20384, 20412, 20475, 20480, 20580, 20625, 20736, 20790, 
        20800, 21000, 21060, 21120, 21168, 21384, 21504, 21560, 21600, 21609, 
        21840, 21870, 21875, 21952, 22000, 22050, 22113, 22176, 22275, 22295, 
        22400, 22464, 22500, 22528, 22638, 22680, 22750, 22932, 23040, 23100, 
        23296, 23328, 23400, 23520, 23625, 23760, 23814, 24000, 24010, 24057, 
        24192, 24255, 24300, 24375, 24500, 24570, 24576, 24640, 24696, 24750, 
        24948, 24960, 25000, 25088, 25200, 25272, 25344, 25480, 25515, 25600, 
        25725, 25872, 25920, 26000, 26208, 26244, 26250, 26325, 26400, 26411, 
        26460, 26624, 26730, 26754, 26880, 26950, 27000, 27216, 27300, 27440, 
        27500, 27648, 27720, 27783, 28000, 28080, 28125, 28160, 28224, 28350, 
        28431, 28512, 28665, 28672, 28800, 28812, 28875, 29106, 29120, 29160, 
        29250, 29400, 29484, 29568, 29700, 29952, 30000, 30184, 30240, 30375, 
        30576, 30618, 30625, 30720, 30800, 30870, 31104, 31185, 31200, 31213, 
        31250, 31360, 31500, 31590, 31680, 31752, 31850, 32000, 32076, 32256, 
        32340, 32400, 32500, 32760, 32768, 32805, 32928, 33000, 33075, 33264, 
        33280, 33600, 33614, 33696, 33750, 33792, 33957, 34020, 34125, 34300, 
        34375, 34398, 34496, 34560, 34650, 34944, 34992, 35000, 35100, 35200, 
        35280, 35640, 35672, 35721, 35840, 36000, 36015, 36288, 36400, 36450, 
        36750, 36855, 36864, 36960, 37044, 37125, 37422, 37440, 37500, 37632, 
        37730, 37800, 37908, 38016, 38220, 38400, 38416, 38500, 38808, 38880, 
        39000, 39200, 39312, 39366, 39375, 39424, 39600, 39690, 39936, 40000, 
        40095, 40131, 40320, 40425, 40500, 40625, 40768, 40824, 40950, 40960, 
        41160, 41250, 41472, 41580, 41600, 42000, 42120, 42240, 42336, 42525, 
        42768, 42875, 43008, 43120, 43200, 43218, 43659, 43680, 43740, 43750, 
        43875, 43904, 44000, 44100, 44226, 44352, 44550, 44590, 44800, 44928, 
        45000, 45056, 45276, 45360, 45500, 45864, 45927, 46080, 46200, 46305, 
        46592, 46656, 46800, 46875, 47040, 47250, 47385, 47520, 47628, 47775, 
        48000, 48020, 48114, 48125, 48384, 48510, 48600, 48750, 49000, 49140, 
        49152, 49280, 49392, 49500, 49896, 49920, 50000, 50176, 50400, 50421, 
        50544, 50625, 50688, 50960, 51030, 51200, 51450, 51597, 51744, 51840, 
        51975, 52000, 52416, 52488, 52500, 52650, 52800, 52822, 52920, 53248, 
        53460, 53508, 53760, 53900, 54000, 54432, 54600, 54675, 54880, 55000, 
        55125, 55296, 55440, 55566, 56000, 56133, 56160, 56250, 56320, 56448, 
        56595, 56700, 56862, 56875, 57024, 57330, 57344, 57600, 57624, 57750, 
        58212, 58240, 58320, 58500, 58800, 58968, 59049, 59136, 59400, 59535, 
        59904, 60000, 60025, 60368, 60480, 60750, 61152, 61236, 61250, 61425, 
        61440, 61600, 61740, 61875, 62208, 62370, 62400, 62426, 62500, 62720, 
        63000, 63180, 63360, 63504, 63700, 64000, 64152, 64512, 64680, 64800, 
        64827, 65000, 65520, 65536, 65610, 65625, 65856, 66000, 66150, 66339, 
        66528, 66560, 66825, 66885, 67200, 67228, 67375, 67392, 67500, 67584, 
        67914, 68040, 68250, 68600, 68750, 68796, 68992, 69120, 69300, 69888, 
        69984, 70000, 70200, 70400, 70560, 70875, 71280, 71344, 71442, 71680, 
        72000, 72030, 72171, 72576, 72765, 72800, 72900, 73125, 73500, 73710, 
        73728, 73920, 74088, 74250, 74844, 74880, 75000, 75264, 75460, 75600, 
        75816, 76032, 76440, 76545, 76800, 76832, 77000, 77175, 77616, 77760, 
        78000, 78125, 78400, 78624, 78732, 78750, 78848, 78975, 79200, 79233, 
        79380, 79625, 79872, 80000, 80190, 80262, 80640, 80850, 81000, 81250, 
        81536, 81648, 81900, 81920, 82320, 82500, 82944, 83160, 83200, 83349, 
        84000, 84035, 84240, 84375, 84480, 84672, 85050, 85293, 85536, 85750, 
        85995, 86016, 86240, 86400, 86436, 86625, 87318, 87360, 87480, 87500, 
        87750, 87808, 88000, 88200, 88452, 88704, 89100, 89180, 89600, 89856, 
        90000, 90112, 90552, 90720, 91000, 91125, 91728, 91854, 91875, 92160, 
        92400, 92610, 93184, 93312, 93555, 93600, 93639, 93750, 94080, 94325, 
        94500, 94770, 95040, 95256, 95550, 96000, 96040, 96228, 96250, 96768, 
        97020, 97200, 97500, 98000, 98280, 98304, 98415, 98560, 98784, 99000, 
        99225, 99792, 99840 
}; 

template <int DUMMY>
int FFTWPaddingSize<DUMMY>::goodEvenSizes[evenSize] = { 
        2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 
        24, 26, 28, 30, 32, 36, 40, 42, 44, 48, 50, 52, 54, 56, 60, 64, 66, 70, 
        72, 78, 80, 84, 88, 90, 96, 98, 100, 104, 108, 110, 112, 120, 126, 128, 
        130, 132, 140, 144, 150, 154, 156, 160, 162, 168, 176, 180, 182, 192, 
        196, 198, 200, 208, 210, 216, 220, 224, 234, 240, 250, 252, 256, 260, 
        264, 270, 280, 288, 294, 300, 308, 312, 320, 324, 330, 336, 350, 352, 
        360, 364, 378, 384, 390, 392, 396, 400, 416, 420, 432, 440, 448, 450, 
        462, 468, 480, 486, 490, 500, 504, 512, 520, 528, 540, 546, 550, 560, 
        576, 588, 594, 600, 616, 624, 630, 640, 648, 650, 660, 672, 686, 700, 
        702, 704, 720, 728, 750, 756, 768, 770, 780, 784, 792, 800, 810, 832, 
        840, 864, 880, 882, 896, 900, 910, 924, 936, 960, 972, 980, 990, 1000, 
        1008, 1024, 1040, 1050, 1056, 1078, 1080, 1092, 1100, 1120, 1134, 1152, 
        1170, 1176, 1188, 1200, 1232, 1248, 1250, 1260, 1274, 1280, 1296, 1300, 
        1320, 1344, 1350, 1372, 1386, 1400, 1404, 1408, 1440, 1456, 1458, 1470, 
        1500, 1512, 1536, 1540, 1560, 1568, 1584, 1600, 1620, 1638, 1650, 1664, 
        1680, 1728, 1750, 1760, 1764, 1782, 1792, 1800, 1820, 1848, 1872, 1890, 
        1920, 1944, 1950, 1960, 1980, 2000, 2016, 2048, 2058, 2080, 2100, 2106, 
        2112, 2156, 2160, 2184, 2200, 2240, 2250, 2268, 2304, 2310, 2340, 2352, 
        2376, 2400, 2430, 2450, 2464, 2496, 2500, 2520, 2548, 2560, 2592, 2600, 
        2640, 2646, 2688, 2700, 2730, 2744, 2750, 2772, 2800, 2808, 2816, 2880, 
        2912, 2916, 2940, 2970, 3000, 3024, 3072, 3080, 3120, 3136, 3150, 3168, 
        3200, 3234, 3240, 3250, 3276, 3300, 3328, 3360, 3402, 3430, 3456, 3500, 
        3510, 3520, 3528, 3564, 3584, 3600, 3640, 3696, 3744, 3750, 3780, 3822, 
        3840, 3850, 3888, 3900, 3920, 3960, 4000, 4032, 4050, 4096, 4116, 4158, 
        4160, 4200, 4212, 4224, 4312, 4320, 4368, 4374, 4400, 4410, 4480, 4500, 
        4536, 4550, 4608, 4620, 4680, 4704, 4752, 4800, 4802, 4860, 4900, 4914, 
        4928, 4950, 4992, 5000, 5040, 5096, 5120, 5184, 5200, 5250, 5280, 5292, 
        5346, 5376, 5390, 5400, 5460, 5488, 5500, 5544, 5600, 5616, 5632, 5670, 
        5760, 5824, 5832, 5850, 5880, 5940, 6000, 6048, 6144, 6160, 6174, 6240, 
        6250, 6272, 6300, 6318, 6336, 6370, 6400, 6468, 6480, 6500, 6552, 6600, 
        6656, 6720, 6750, 6804, 6860, 6912, 6930, 7000, 7020, 7040, 7056, 7128, 
        7168, 7200, 7280, 7290, 7350, 7392, 7488, 7500, 7546, 7560, 7644, 7680, 
        7700, 7776, 7800, 7840, 7920, 7938, 8000, 8064, 8100, 8190, 8192, 8232, 
        8250, 8316, 8320, 8400, 8424, 8448, 8624, 8640, 8736, 8748, 8750, 8800, 
        8820, 8910, 8918, 8960, 9000, 9072, 9100, 9216, 9240, 9360, 9408, 9450, 
        9504, 9600, 9604, 9702, 9720, 9750, 9800, 9828, 9856, 9900, 9984, 10000, 
        10080, 10192, 10206, 10240, 10290, 10368, 10400, 10500, 10530, 10560, 
        10584, 10692, 10752, 10780, 10800, 10920, 10976, 11000, 11088, 11200, 
        11232, 11250, 11264, 11340, 11466, 11520, 11550, 11648, 11664, 11700, 
        11760, 11880, 12000, 12096, 12150, 12250, 12288, 12320, 12348, 12474, 
        12480, 12500, 12544, 12600, 12636, 12672, 12740, 12800, 12936, 12960, 
        13000, 13104, 13122, 13200, 13230, 13312, 13440, 13500, 13608, 13650, 
        13720, 13750, 13824, 13860, 14000, 14040, 14080, 14112, 14256, 14336, 
        14400, 14406, 14560, 14580, 14700, 14742, 14784, 14850, 14976, 15000, 
        15092, 15120, 15288, 15360, 15400, 15552, 15600, 15680, 15750, 15840, 
        15876, 16000, 16038, 16128, 16170, 16200, 16250, 16380, 16384, 16464, 
        16500, 16632, 16640, 16800, 16848, 16896, 17010, 17150, 17248, 17280, 
        17472, 17496, 17500, 17550, 17600, 17640, 17820, 17836, 17920, 18000, 
        18144, 18200, 18432, 18480, 18522, 18720, 18750, 18816, 18900, 18954, 
        19008, 19110, 19200, 19208, 19250, 19404, 19440, 19500, 19600, 19656, 
        19712, 19800, 19968, 20000, 20160, 20250, 20384, 20412, 20480, 20580, 
        20736, 20790, 20800, 21000, 21060, 21120, 21168, 21384, 21504, 21560, 
        21600, 21840, 21870, 21952, 22000, 22050, 22176, 22400, 22464, 22500, 
        22528, 22638, 22680, 22750, 22932, 23040, 23100, 23296, 23328, 23400, 
        23520, 23760, 23814, 24000, 24010, 24192, 24300, 24500, 24570, 24576, 
        24640, 24696, 24750, 24948, 24960, 25000, 25088, 25200, 25272, 25344, 
        25480, 25600, 25872, 25920, 26000, 26208, 26244, 26250, 26400, 26460, 
        26624, 26730, 26754, 26880, 26950, 27000, 27216, 27300, 27440, 27500, 
        27648, 27720, 28000, 28080, 28160, 28224, 28350, 28512, 28672, 28800, 
        28812, 29106, 29120, 29160, 29250, 29400, 29484, 29568, 29700, 29952, 
        30000, 30184, 30240, 30576, 30618, 30720, 30800, 30870, 31104, 31200, 
        31250, 31360, 31500, 31590, 31680, 31752, 31850, 32000, 32076, 32256, 
        32340, 32400, 32500, 32760, 32768, 32928, 33000, 33264, 33280, 33600, 
        33614, 33696, 33750, 33792, 34020, 34300, 34398, 34496, 34560, 34650, 
        34944, 34992, 35000, 35100, 35200, 35280, 35640, 35672, 35840, 36000, 
        36288, 36400, 36450, 36750, 36864, 36960, 37044, 37422, 37440, 37500, 
        37632, 37730, 37800, 37908, 38016, 38220, 38400, 38416, 38500, 38808, 
        38880, 39000, 39200, 39312, 39366, 39424, 39600, 39690, 39936, 40000, 
        40320, 40500, 40768, 40824, 40950, 40960, 41160, 41250, 41472, 41580, 
        41600, 42000, 42120, 42240, 42336, 42768, 43008, 43120, 43200, 43218, 
        43680, 43740, 43750, 43904, 44000, 44100, 44226, 44352, 44550, 44590, 
        44800, 44928, 45000, 45056, 45276, 45360, 45500, 45864, 46080, 46200, 
        46592, 46656, 46800, 47040, 47250, 47520, 47628, 48000, 48020, 48114, 
        48384, 48510, 48600, 48750, 49000, 49140, 49152, 49280, 49392, 49500, 
        49896, 49920, 50000, 50176, 50400, 50544, 50688, 50960, 51030, 51200, 
        51450, 51744, 51840, 52000, 52416, 52488, 52500, 52650, 52800, 52822, 
        52920, 53248, 53460, 53508, 53760, 53900, 54000, 54432, 54600, 54880, 
        55000, 55296, 55440, 55566, 56000, 56160, 56250, 56320, 56448, 56700, 
        56862, 57024, 57330, 57344, 57600, 57624, 57750, 58212, 58240, 58320, 
        58500, 58800, 58968, 59136, 59400, 59904, 60000, 60368, 60480, 60750, 
        61152, 61236, 61250, 61440, 61600, 61740, 62208, 62370, 62400, 62426, 
        62500, 62720, 63000, 63180, 63360, 63504, 63700, 64000, 64152, 64512, 
        64680, 64800, 65000, 65520, 65536, 65610, 65856, 66000, 66150, 66528, 
        66560, 67200, 67228, 67392, 67500, 67584, 67914, 68040, 68250, 68600, 
        68750, 68796, 68992, 69120, 69300, 69888, 69984, 70000, 70200, 70400, 
        70560, 71280, 71344, 71442, 71680, 72000, 72030, 72576, 72800, 72900, 
        73500, 73710, 73728, 73920, 74088, 74250, 74844, 74880, 75000, 75264, 
        75460, 75600, 75816, 76032, 76440, 76800, 76832, 77000, 77616, 77760, 
        78000, 78400, 78624, 78732, 78750, 78848, 79200, 79380, 79872, 80000, 
        80190, 80262, 80640, 80850, 81000, 81250, 81536, 81648, 81900, 81920, 
        82320, 82500, 82944, 83160, 83200, 84000, 84240, 84480, 84672, 85050, 
        85536, 85750, 86016, 86240, 86400, 86436, 87318, 87360, 87480, 87500, 
        87750, 87808, 88000, 88200, 88452, 88704, 89100, 89180, 89600, 89856, 
        90000, 90112, 90552, 90720, 91000, 91728, 91854, 92160, 92400, 92610, 
        93184, 93312, 93600, 93750, 94080, 94500, 94770, 95040, 95256, 95550, 
        96000, 96040, 96228, 96250, 96768, 97020, 97200, 97500, 98000, 98280, 
        98304, 98560, 98784, 99000, 99792, 99840 
}; 

template <int M>
struct FFTEmbedKernel
{
    template <unsigned int N, class Real, class C, class Shape>
    static void 
    exec(MultiArrayView<N, Real, C> & out, Shape const & kernelShape, 
         Shape & srcPoint, Shape & destPoint, bool copyIt)
    {
        for(srcPoint[M]=0; srcPoint[M]<kernelShape[M]; ++srcPoint[M])
        {
            if(srcPoint[M] < (kernelShape[M] + 1) / 2)
            {
                destPoint[M] = srcPoint[M];
            }
            else
            {
                destPoint[M] = srcPoint[M] + out.shape(M) - kernelShape[M];
                copyIt = true;
            }
            FFTEmbedKernel<M-1>::exec(out, kernelShape, srcPoint, destPoint, copyIt);
        }
    }
};

template <>
struct FFTEmbedKernel<0>
{
    template <unsigned int N, class Real, class C, class Shape>
    static void 
    exec(MultiArrayView<N, Real, C> & out, Shape const & kernelShape, 
         Shape & srcPoint, Shape & destPoint, bool copyIt)
    {
        for(srcPoint[0]=0; srcPoint[0]<kernelShape[0]; ++srcPoint[0])
        {
            if(srcPoint[0] < (kernelShape[0] + 1) / 2)
            {
                destPoint[0] = srcPoint[0];
            }
            else
            {
                destPoint[0] = srcPoint[0] + out.shape(0) - kernelShape[0];
                copyIt = true;
            }
            if(copyIt)
            {
                out[destPoint] = out[srcPoint];
                out[srcPoint] = 0.0;
            }
        }
    }
};

template <unsigned int N, class Real, class C1, class C2>
void 
fftEmbedKernel(MultiArrayView<N, Real, C1> kernel,
               MultiArrayView<N, Real, C2> out,
               Real norm = 1.0)
{
    typedef typename MultiArrayShape<N>::type Shape;

    MultiArrayView<N, Real, C2> kout = out.subarray(Shape(), kernel.shape());
    
    out.init(0.0);
    kout = kernel;
    if (norm != 1.0)
        kout *= norm;
    moveDCToUpperLeft(kout);
    
    Shape srcPoint, destPoint;    
    FFTEmbedKernel<(int)N-1>::exec(out, kernel.shape(), srcPoint, destPoint, false);
}

template <unsigned int N, class Real, class C1, class C2>
void 
fftEmbedArray(MultiArrayView<N, Real, C1> in,
              MultiArrayView<N, Real, C2> out)
{
    typedef typename MultiArrayShape<N>::type Shape;
    
    Shape diff = out.shape() - in.shape(), 
          leftDiff = div(diff, MultiArrayIndex(2)),
          rightDiff = diff - leftDiff,
          right = in.shape() + leftDiff; 
    
    out.subarray(leftDiff, right) = in;
    
    typedef typename MultiArrayView<N, Real, C2>::traverser Traverser;
    typedef MultiArrayNavigator<Traverser, N> Navigator;
    typedef typename Navigator::iterator Iterator;
    
    for(unsigned int d = 0; d < N; ++d)
    {
        Navigator nav(out.traverser_begin(), out.shape(), d);

        for( ; nav.hasMore(); nav++ )
        {
            Iterator i = nav.begin();
            for(int k=1; k<=leftDiff[d]; ++k)
                i[leftDiff[d] - k] = i[leftDiff[d] + k];
            for(int k=0; k<rightDiff[d]; ++k)
                i[right[d] + k] = i[right[d] - k - 2];
        }
    }
}

} // namespace detail

template <class T, int N>
TinyVector<T, N>
fftwBestPaddedShape(TinyVector<T, N> shape)
{
    for(unsigned int k=0; k<N; ++k)
        shape[k] = detail::FFTWPaddingSize<0>::find(shape[k]);
    return shape;
}

template <class T, int N>
TinyVector<T, N>
fftwBestPaddedShapeR2C(TinyVector<T, N> shape)
{
    shape[0] = detail::FFTWPaddingSize<0>::findEven(shape[0]);
    for(unsigned int k=1; k<N; ++k)
        shape[k] = detail::FFTWPaddingSize<0>::find(shape[k]);
    return shape;
}

/** \brief Find frequency domain shape for a R2C Fourier transform.

    When a real valued array is transformed to the frequency domain, about half of the 
    Fourier coefficients are redundant. The transform can be optimized as a <a href="http://www.fftw.org/doc/Multi_002dDimensional-DFTs-of-Real-Data.html">R2C 
    transform</a> that doesn't compute and store the redundant coefficients. This function
    computes the appropriate frequency domain shape for a given shape in the spatial domain.
    It simply replaces <tt>shape[0]</tt> with <tt>shape[0] / 2 + 1</tt>.
    
    <b>\#include</b> \<vigra/multi_fft.hxx\><br/>
    Namespace: vigra
*/
template <class T, int N>
TinyVector<T, N>
fftwCorrespondingShapeR2C(TinyVector<T, N> shape)
{
    shape[0] = shape[0] / 2 + 1;
    return shape;
}

template <class T, int N>
TinyVector<T, N>
fftwCorrespondingShapeC2R(TinyVector<T, N> shape, bool oddDimension0 = false)
{
    shape[0] = oddDimension0
                  ? (shape[0] - 1) * 2 + 1
                  : (shape[0] - 1) * 2;
    return shape;
}

/********************************************************/
/*                                                      */
/*                       FFTWPlan                       */
/*                                                      */
/********************************************************/

/** C++ wrapper for FFTW plans.

    The class encapsulates the calls to <tt>fftw_plan_dft_2d</tt>, <tt>fftw_execute</tt>, and
    <tt>fftw_destroy_plan</tt> (and their <tt>float</tt> and <tt>long double</tt> counterparts)
    in an easy-to-use interface.

    Usually, you use this class only indirectly via \ref fourierTransform() 
    and \ref fourierTransformInverse(). You only need this class if you want to have more control
    about FFTW's planning process (by providing non-default planning flags) and/or want to re-use
    plans for several transformations.
    
    <b> Usage:</b>

    <b>\#include</b> \<vigra/multi_fft.hxx\><br>
    Namespace: vigra

    \code
    // compute complex Fourier transform of a real image
    MultiArray<2, double> src(Shape2(w, h));
    MultiArray<2, FFTWComplex<double> > fourier(Shape2(w, h));
    
    // create an optimized plan by measuring the speed of several algorithm variants
    FFTWPlan<2, double> plan(src, fourier, FFTW_MEASURE);
    
    plan.execute(src, fourier); 
    \endcode
*/
template <unsigned int N, class Real = double>
class FFTWPlan
{
    typedef ArrayVector<int> Shape;
    typedef typename FFTWReal2Complex<Real>::plan_type PlanType;
    typedef typename FFTWComplex<Real>::complex_type Complex;
    
    PlanType plan;
    Shape shape, instrides, outstrides;
    int sign;
    
  public:
        /** \brief Create an empty plan.
        
            The plan can be initialized later by one of the init() functions.
        */
    FFTWPlan()
    : plan(0)
    {}
    
        /** \brief Create a plan for a complex-to-complex transform.
        
            \arg SIGN must be <tt>FFTW_FORWARD</tt> or <tt>FFTW_BACKWARD</tt> according to the
            desired transformation direction.
            \arg planner_flags must be a combination of the <a href="http://www.fftw.org/doc/Planner-Flags.html">planner 
            flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
            optimal algorithm settings or read them from pre-loaded <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
        */
    template <class C1, class C2>
    FFTWPlan(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
             MultiArrayView<N, FFTWComplex<Real>, C2> out,
             int SIGN, unsigned int planner_flags = FFTW_ESTIMATE)
    : plan(0)
    {
        init(in, out, SIGN, planner_flags);
    }
    
        /** \brief Create a plan for a real-to-complex transform.
        
            This always refers to a forward transform. The shape of the output determines
            if a standard transform (when <tt>out.shape() == in.shape()</tt>) or an 
            <a href="http://www.fftw.org/doc/Multi_002dDimensional-DFTs-of-Real-Data.html">R2C 
            transform</a> (when <tt>out.shape() == fftwCorrespondingShapeR2C(in.shape())</tt>) will be executed. 
            
            \arg planner_flags must be a combination of the <a href="http://www.fftw.org/doc/Planner-Flags.html">planner 
            flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
            optimal algorithm settings or read them from pre-loaded <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
        */
    template <class C1, class C2>
    FFTWPlan(MultiArrayView<N, Real, C1> in, 
             MultiArrayView<N, FFTWComplex<Real>, C2> out,
             unsigned int planner_flags = FFTW_ESTIMATE)
    : plan(0)
    {
        init(in, out, planner_flags);
    }

        /** \brief Create a plan for a complex-to-real transform.
        
            This always refers to a inverse transform. The shape of the input determines
            if a standard transform (when <tt>in.shape() == out.shape()</tt>) or a 
            <a href="http://www.fftw.org/doc/Multi_002dDimensional-DFTs-of-Real-Data.html">C2R 
            transform</a> (when <tt>in.shape() == fftwCorrespondingShapeR2C(out.shape())</tt>) will be executed. 
            
            \arg planner_flags must be a combination of the <a href="http://www.fftw.org/doc/Planner-Flags.html">planner 
            flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
            optimal algorithm settings or read them from pre-loaded <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
        */
    template <class C1, class C2>
    FFTWPlan(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
             MultiArrayView<N, Real, C2> out,
             unsigned int planner_flags = FFTW_ESTIMATE)
    : plan(0)
    {
        init(in, out, planner_flags);
    }
    
        /** \brief Copy constructor.
        */
    FFTWPlan(FFTWPlan const & other)
    : plan(other.plan),
      sign(other.sign)
    {
        FFTWPlan & o = const_cast<FFTWPlan &>(other);
        shape.swap(o.shape);
        instrides.swap(o.instrides);
        outstrides.swap(o.outstrides);
        o.plan = 0; // act like std::auto_ptr
    }
    
        /** \brief Copy assigment.
        */
    FFTWPlan & operator=(FFTWPlan const & other)
    {
        if(this != &other)
        {
            FFTWPlan & o = const_cast<FFTWPlan &>(other);
            plan = o.plan;
            shape.swap(o.shape);
            instrides.swap(o.instrides);
            outstrides.swap(o.outstrides);
            sign = o.sign;
            o.plan = 0; // act like std::auto_ptr
        }
        return *this;
    }

        /** \brief Destructor.
        */
    ~FFTWPlan()
    {
        detail::fftwPlanDestroy(plan);
    }

        /** \brief Init a complex-to-complex transform.
        
            See the constructor with the same signature for details.
        */
    template <class C1, class C2>
    void init(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
              MultiArrayView<N, FFTWComplex<Real>, C2> out,
              int SIGN, unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.strideOrdering() == out.strideOrdering(),
            "FFTWPlan.init(): input and output must have the same stride ordering.");
            
        initImpl(in.permuteStridesDescending(), out.permuteStridesDescending(), 
                 SIGN, planner_flags);
    }
        
        /** \brief Init a real-to-complex transform.
        
            See the constructor with the same signature for details.
        */
    template <class C1, class C2>
    void init(MultiArrayView<N, Real, C1> in, 
              MultiArrayView<N, FFTWComplex<Real>, C2> out,
              unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.strideOrdering() == out.strideOrdering(),
            "FFTWPlan.init(): input and output must have the same stride ordering.");

        initImpl(in.permuteStridesDescending(), out.permuteStridesDescending(), 
                 FFTW_FORWARD, planner_flags);
    }
        
        /** \brief Init a complex-to-real transform.
        
            See the constructor with the same signature for details.
        */
    template <class C1, class C2>
    void init(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
              MultiArrayView<N, Real, C2> out,
              unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.strideOrdering() == out.strideOrdering(),
            "FFTWPlan.init(): input and output must have the same stride ordering.");

        initImpl(in.permuteStridesDescending(), out.permuteStridesDescending(), 
                 FFTW_BACKWARD, planner_flags);
    }
    
        /** \brief Execute a complex-to-complex transform.
        
            The array shapes must be the same as in the corresponding init function
            or constructor. However, execute() can be called several times on
            the same plan, even with different arrays, as long as they have the appropriate 
            shapes.
        */
    template <class C1, class C2>
    void execute(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                 MultiArrayView<N, FFTWComplex<Real>, C2> out) const
    {
        executeImpl(in.permuteStridesDescending(), out.permuteStridesDescending());
    }
    
        /** \brief Execute a real-to-complex transform.
        
            The array shapes must be the same as in the corresponding init function
            or constructor. However, execute() can be called several times on
            the same plan, even with different arrays, as long as they have the appropriate 
            shapes.
        */
    template <class C1, class C2>
    void execute(MultiArrayView<N, Real, C1> in, 
                 MultiArrayView<N, FFTWComplex<Real>, C2> out) const
    {
        executeImpl(in.permuteStridesDescending(), out.permuteStridesDescending());
    }
    
        /** \brief Execute a complex-to-real transform.
        
            The array shapes must be the same as in the corresponding init function
            or constructor. However, execute() can be called several times on
            the same plan, even with different arrays, as long as they have the appropriate 
            shapes.
        */
    template <class C1, class C2>
    void execute(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                 MultiArrayView<N, Real, C2> out) const
    {
        executeImpl(in.permuteStridesDescending(), out.permuteStridesDescending());
    }
    
  private:
    
    template <class MI, class MO>
    void initImpl(MI ins, MO outs, int SIGN, unsigned int planner_flags);
    
    template <class MI, class MO>
    void executeImpl(MI ins, MO outs) const;
    
    void checkShapes(MultiArrayView<N, FFTWComplex<Real>, StridedArrayTag> in, 
                     MultiArrayView<N, FFTWComplex<Real>, StridedArrayTag> out) const
    {
        vigra_precondition(in.shape() == out.shape(),
            "FFTWPlan.init(): input and output must have the same shape.");
    }
    
    void checkShapes(MultiArrayView<N, Real, StridedArrayTag> ins, 
                     MultiArrayView<N, FFTWComplex<Real>, StridedArrayTag> outs) const
    {
        for(int k=0; k<(int)N-1; ++k)
            vigra_precondition(ins.shape(k) == outs.shape(k),
                "FFTWPlan.init(): input and output must have matching shapes.");
        vigra_precondition(ins.shape(N-1) / 2 + 1 == outs.shape(N-1),
            "FFTWPlan.init(): input and output must have matching shapes.");
    }
    
    void checkShapes(MultiArrayView<N, FFTWComplex<Real>, StridedArrayTag> ins, 
                     MultiArrayView<N, Real, StridedArrayTag> outs) const
    {
        for(int k=0; k<(int)N-1; ++k)
            vigra_precondition(ins.shape(k) == outs.shape(k),
                "FFTWPlan.init(): input and output must have matching shapes.");
        vigra_precondition(outs.shape(N-1) / 2 + 1 == ins.shape(N-1),
            "FFTWPlan.init(): input and output must have matching shapes.");
    }
};

template <unsigned int N, class Real>
template <class MI, class MO>
void
FFTWPlan<N, Real>::initImpl(MI ins, MO outs, int SIGN, unsigned int planner_flags)
{
    checkShapes(ins, outs);
    
    typename MultiArrayShape<N>::type logicalShape(SIGN == FFTW_FORWARD
                                                ? ins.shape()
                                                : outs.shape());
                                           
    Shape newShape(logicalShape.begin(), logicalShape.end()),
          newIStrides(ins.stride().begin(), ins.stride().end()),
          newOStrides(outs.stride().begin(), outs.stride().end()),
          itotal(ins.shape().begin(), ins.shape().end()), 
          ototal(outs.shape().begin(), outs.shape().end());

    for(unsigned int j=1; j<N; ++j)
    {
        itotal[j] = ins.stride(j-1) / ins.stride(j);
        ototal[j] = outs.stride(j-1) / outs.stride(j);
    }
    
    PlanType newPlan = detail::fftwPlanCreate(N, newShape.begin(), 
                                  ins.data(), itotal.begin(), ins.stride(N-1),
                                  outs.data(), ototal.begin(), outs.stride(N-1),
                                  SIGN, planner_flags);
    detail::fftwPlanDestroy(plan);
    plan = newPlan;
    shape.swap(newShape);
    instrides.swap(newIStrides);
    outstrides.swap(newOStrides);
    sign = SIGN;
}

template <unsigned int N, class Real>
template <class MI, class MO>
void FFTWPlan<N, Real>::executeImpl(MI ins, MO outs) const
{
    vigra_precondition(plan != 0, "FFTWPlan::execute(): plan is NULL.");

    typename MultiArrayShape<N>::type lshape(sign == FFTW_FORWARD
                                                ? ins.shape()
                                                : outs.shape());
                                           
    vigra_precondition((lshape == TinyVectorView<int, N>(shape.data())),
        "FFTWPlan::execute(): shape mismatch between plan and data.");
    vigra_precondition((ins.stride() == TinyVectorView<int, N>(instrides.data())),
        "FFTWPlan::execute(): strides mismatch between plan and input data.");
    vigra_precondition((outs.stride() == TinyVectorView<int, N>(outstrides.data())),
        "FFTWPlan::execute(): strides mismatch between plan and output data.");

    detail::fftwPlanExecute(plan, ins.data(), outs.data());
    
    typedef typename MO::value_type V;
    if(sign == FFTW_BACKWARD)
        outs *= V(1.0) / Real(outs.size());
}

/********************************************************/
/*                                                      */
/*                  FFTWConvolvePlan                    */
/*                                                      */
/********************************************************/

/** C++ wrapper for a pair of FFTW plans used to perform FFT-based convolution.

    The class encapsulates the calls to <tt>fftw_plan_dft_2d</tt>, <tt>fftw_execute</tt>, and
    <tt>fftw_destroy_plan</tt> (and their <tt>float</tt> and <tt>long double</tt> counterparts)
    in an easy-to-use interface. It always creates a pair of plans, one for the forward and one
    for the inverse transform required for convolution.

    Usually, you use this class only indirectly via \ref convolveFFT() and its variants. 
    You only need this class if you want to have more control about FFTW's planning process 
    (by providing non-default planning flags) and/or want to re-use plans for several convolutions.
    
    <b> Usage:</b>

    <b>\#include</b> \<vigra/multi_fft.hxx\><br>
    Namespace: vigra

    \code
    // convolve a real array with a real kernel
    MultiArray<2, double> src(Shape2(w, h)), dest(Shape2(w, h));
    
    MultiArray<2, double> spatial_kernel(Shape2(9, 9));
    Gaussian<double> gauss(1.0);
    
    for(int y=0; y<9; ++y)
        for(int x=0; x<9; ++x)
            spatial_kernel(x, y) = gauss(x-4.0)*gauss(y-4.0);
            
    // create an optimized plan by measuring the speed of several algorithm variants
    FFTWConvolvePlan<2, double> plan(src, spatial_kernel, dest, FFTW_MEASURE);
    
    plan.execute(src, spatial_kernel, dest); 
    \endcode
*/
template <unsigned int N, class Real>
class FFTWConvolvePlan
{
    typedef FFTWComplex<Real> Complex;
    typedef MultiArrayView<N, Real, UnstridedArrayTag >     RArray;
    typedef MultiArray<N, Complex, FFTWAllocator<Complex> > CArray;
    
    FFTWPlan<N, Real> forward_plan, backward_plan;
    RArray realArray, realKernel;
    CArray fourierArray, fourierKernel;
    bool useFourierKernel;

  public:
  
    typedef typename MultiArrayShape<N>::type Shape;

        /** \brief Create an empty plan.
        
            The plan can be initialized later by one of the init() functions.
        */
    FFTWConvolvePlan()
    : useFourierKernel(false)
    {}
    
        /** \brief Create a plan to convolve a real array with a real kernel.
        
            The kernel must be defined in the spatial domain.
            See \ref convolveFFT() for detailed information on required shapes and internal padding.
        
            \arg planner_flags must be a combination of the 
            <a href="http://www.fftw.org/doc/Planner-Flags.html">planner 
            flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
            optimal algorithm settings or read them from pre-loaded 
            <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
        */
    template <class C1, class C2, class C3>
    FFTWConvolvePlan(MultiArrayView<N, Real, C1> in, 
                     MultiArrayView<N, Real, C2> kernel,
                     MultiArrayView<N, Real, C3> out,
                     unsigned int planner_flags = FFTW_ESTIMATE)
    : useFourierKernel(false)
    {
        init(in, kernel, out, planner_flags);
    }
    
        /** \brief Create a plan to convolve a real array with a complex kernel.
        
            The kernel must be defined in the Fourier domain, using the half-space format. 
            See \ref convolveFFT() for detailed information on required shapes and internal padding.
        
            \arg planner_flags must be a combination of the 
            <a href="http://www.fftw.org/doc/Planner-Flags.html">planner 
            flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
            optimal algorithm settings or read them from pre-loaded 
            <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
        */
    template <class C1, class C2, class C3>
    FFTWConvolvePlan(MultiArrayView<N, Real, C1> in, 
                     MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                     MultiArrayView<N, Real, C3> out,
                     unsigned int planner_flags = FFTW_ESTIMATE)
    : useFourierKernel(true)
    {
        init(in, kernel, out, planner_flags);
    }
   
        /** \brief Create a plan to convolve a complex array with a complex kernel.
        
            See \ref convolveFFT() for detailed information on required shapes and internal padding.
        
            \arg fourierDomainKernel determines if the kernel is defined in the spatial or
                  Fourier domain.
            \arg planner_flags must be a combination of the 
            <a href="http://www.fftw.org/doc/Planner-Flags.html">planner 
            flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
            optimal algorithm settings or read them from pre-loaded 
            <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
        */
    template <class C1, class C2, class C3>
    FFTWConvolvePlan(MultiArrayView<N, FFTWComplex<Real>, C1> in,
                     MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                     MultiArrayView<N, FFTWComplex<Real>, C3> out, 
                     bool fourierDomainKernel,
                     unsigned int planner_flags = FFTW_ESTIMATE)
    {
        init(in, kernel, out, fourierDomainKernel, planner_flags);
    }

 
        /** \brief Create a plan from just the shape information.
        
            See \ref convolveFFT() for detailed information on required shapes and internal padding.
        
            \arg fourierDomainKernel determines if the kernel is defined in the spatial or
                  Fourier domain.
            \arg planner_flags must be a combination of the 
            <a href="http://www.fftw.org/doc/Planner-Flags.html">planner 
            flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
            optimal algorithm settings or read them from pre-loaded 
            <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
        */
    template <class C1, class C2, class C3>
    FFTWConvolvePlan(Shape inOut, Shape kernel, 
                     bool useFourierKernel = false,
                     unsigned int planner_flags = FFTW_ESTIMATE)
    {
        if(useFourierKernel)
            init(inOut, kernel, planner_flags);
        else
            initFourierKernel(inOut, kernel, planner_flags);
    }
    
        /** \brief Init a plan to convolve a real array with a real kernel.
         
            See the constructor with the same signature for details.
        */
    template <class C1, class C2, class C3>
    void init(MultiArrayView<N, Real, C1> in, 
              MultiArrayView<N, Real, C2> kernel,
              MultiArrayView<N, Real, C3> out,
              unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.shape() == out.shape(),
            "FFTWConvolvePlan::init(): input and output must have the same shape.");
        init(in.shape(), kernel.shape(), planner_flags);
    }
    
        /** \brief Init a plan to convolve a real array with a complex kernel.
         
            See the constructor with the same signature for details.
        */
    template <class C1, class C2, class C3>
    void init(MultiArrayView<N, Real, C1> in, 
              MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
              MultiArrayView<N, Real, C3> out,
              unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.shape() == out.shape(),
            "FFTWConvolvePlan::init(): input and output must have the same shape.");
        initFourierKernel(in.shape(), kernel.shape(), planner_flags);
    }
    
        /** \brief Init a plan to convolve a complex array with a complex kernel.
         
            See the constructor with the same signature for details.
        */
    template <class C1, class C2, class C3>
    void init(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
              MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
              MultiArrayView<N, FFTWComplex<Real>, C3> out, 
              bool fourierDomainKernel,
              unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.shape() == out.shape(),
            "FFTWConvolvePlan::init(): input and output must have the same shape.");
        useFourierKernel = fourierDomainKernel;
        initComplex(in.shape(), kernel.shape(), planner_flags);
    }
    
        /** \brief Init a plan to convolve a real array with a sequence of kernels.
         
            The kernels can be either real or complex. The sequences \a kernels and \a outs
            must have the same length. See the corresponding constructors 
            for single kernels for details.
        */
    template <class C1, class KernelIterator, class OutIterator>
    void initMany(MultiArrayView<N, Real, C1> in, 
                  KernelIterator kernels, KernelIterator kernelsEnd,
                  OutIterator outs, unsigned int planner_flags = FFTW_ESTIMATE)
    {
        typedef typename std::iterator_traits<KernelIterator>::value_type KernelArray;
        typedef typename KernelArray::value_type KernelValue;
        typedef typename std::iterator_traits<OutIterator>::value_type OutArray;
        typedef typename OutArray::value_type OutValue;

        bool realKernel = IsSameType<KernelValue, Real>::value;
        bool fourierKernel = IsSameType<KernelValue, Complex>::value;

        vigra_precondition(realKernel || fourierKernel,
             "FFTWConvolvePlan::initMany(): kernels have unsuitable value_type.");
        vigra_precondition((IsSameType<OutValue, Real>::value),
             "FFTWConvolvePlan::initMany(): outputs have unsuitable value_type.");

        if(realKernel)
        {
            initMany(in.shape(), checkShapes(in.shape(), kernels, kernelsEnd, outs),
                     planner_flags);
        }
        else
        {
            initFourierKernelMany(in.shape(), 
                                  checkShapesFourier(in.shape(), kernels, kernelsEnd, outs),
                                  planner_flags);
        }
    }
     
        /** \brief Init a plan to convolve a complex array with a sequence of kernels.
         
            The kernels must be complex as well. The sequences \a kernels and \a outs
            must have the same length. See the corresponding constructors 
            for single kernels for details.
        */
    template <class C1, class KernelIterator, class OutIterator>
    void initMany(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                  KernelIterator kernels, KernelIterator kernelsEnd,
                  OutIterator outs,
                  bool fourierDomainKernels,
                  unsigned int planner_flags = FFTW_ESTIMATE)
    {
        typedef typename std::iterator_traits<KernelIterator>::value_type KernelArray;
        typedef typename KernelArray::value_type KernelValue;
        typedef typename std::iterator_traits<OutIterator>::value_type OutArray;
        typedef typename OutArray::value_type OutValue;

        vigra_precondition((IsSameType<KernelValue, Complex>::value),
             "FFTWConvolvePlan::initMany(): kernels have unsuitable value_type.");
        vigra_precondition((IsSameType<OutValue, Complex>::value),
             "FFTWConvolvePlan::initMany(): outputs have unsuitable value_type.");

        useFourierKernel = fourierDomainKernels;
        
        Shape paddedShape = checkShapesComplex(in.shape(), kernels, kernelsEnd, outs);
    
        CArray newFourierArray(paddedShape), newFourierKernel(paddedShape);
    
        FFTWPlan<N, Real> fplan(newFourierArray, newFourierArray, FFTW_FORWARD, planner_flags);
        FFTWPlan<N, Real> bplan(newFourierArray, newFourierArray, FFTW_BACKWARD, planner_flags);
    
        forward_plan = fplan;
        backward_plan = bplan;
        fourierArray.swap(newFourierArray);
        fourierKernel.swap(newFourierKernel);
    }
    
    void init(Shape inOut, Shape kernel,
              unsigned int planner_flags = FFTW_ESTIMATE);
    
    void initFourierKernel(Shape inOut, Shape kernel,
                           unsigned int planner_flags = FFTW_ESTIMATE);
    
    void initComplex(Shape inOut, Shape kernel,
                     unsigned int planner_flags = FFTW_ESTIMATE);
    
    void initMany(Shape inOut, Shape maxKernel,
                  unsigned int planner_flags = FFTW_ESTIMATE)
    {
        init(inOut, maxKernel, planner_flags);
    }
    
    void initFourierKernelMany(Shape inOut, Shape kernels,
                               unsigned int planner_flags = FFTW_ESTIMATE)
    {
        initFourierKernel(inOut, kernels, planner_flags);
    }
        
        /** \brief Execute a plan to convolve a real array with a real kernel.
         
            The array shapes must be the same as in the corresponding init function
            or constructor. However, execute() can be called several times on
            the same plan, even with different arrays, as long as they have the appropriate 
            shapes.
        */
    template <class C1, class C2, class C3>
    void execute(MultiArrayView<N, Real, C1> in, 
                 MultiArrayView<N, Real, C2> kernel,
                 MultiArrayView<N, Real, C3> out);
    
        /** \brief Execute a plan to convolve a real array with a complex kernel.
         
            The array shapes must be the same as in the corresponding init function
            or constructor. However, execute() can be called several times on
            the same plan, even with different arrays, as long as they have the appropriate 
            shapes.
        */
    template <class C1, class C2, class C3>
    void execute(MultiArrayView<N, Real, C1> in, 
                 MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                 MultiArrayView<N, Real, C3> out);

        /** \brief Execute a plan to convolve a complex array with a complex kernel.
         
            The array shapes must be the same as in the corresponding init function
            or constructor. However, execute() can be called several times on
            the same plan, even with different arrays, as long as they have the appropriate 
            shapes.
        */
    template <class C1, class C2, class C3>
    void execute(MultiArrayView<N, FFTWComplex<Real>, C1> in,
                 MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                 MultiArrayView<N, FFTWComplex<Real>, C3> out);


        /** \brief Execute a plan to convolve a complex array with a sequence of kernels.
         
            The array shapes must be the same as in the corresponding init function
            or constructor. However, executeMany() can be called several times on
            the same plan, even with different arrays, as long as they have the appropriate 
            shapes.
        */
    template <class C1, class KernelIterator, class OutIterator>
    void executeMany(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                     KernelIterator kernels, KernelIterator kernelsEnd,
                     OutIterator outs);
                     
        /** \brief Execute a plan to convolve a real array with a sequence of kernels.
         
            The array shapes must be the same as in the corresponding init function
            or constructor. However, executeMany() can be called several times on
            the same plan, even with different arrays, as long as they have the appropriate 
            shapes.
        */
    template <class C1, class KernelIterator, class OutIterator>
    void executeMany(MultiArrayView<N, Real, C1> in, 
                     KernelIterator kernels, KernelIterator kernelsEnd,
                     OutIterator outs)
    {
        typedef typename std::iterator_traits<KernelIterator>::value_type KernelArray;
        typedef typename KernelArray::value_type KernelValue;
        typedef typename IsSameType<KernelValue, Complex>::type UseFourierKernel;
        typedef typename std::iterator_traits<OutIterator>::value_type OutArray;
        typedef typename OutArray::value_type OutValue;

        bool realKernel = IsSameType<KernelValue, Real>::value;
        bool fourierKernel = IsSameType<KernelValue, Complex>::value;

        vigra_precondition(realKernel || fourierKernel,
             "FFTWConvolvePlan::executeMany(): kernels have unsuitable value_type.");
        vigra_precondition((IsSameType<OutValue, Real>::value),
             "FFTWConvolvePlan::executeMany(): outputs have unsuitable value_type.");

        executeManyImpl(in, kernels, kernelsEnd, outs, UseFourierKernel());
    }

  private:
  
    template <class KernelIterator, class OutIterator>
    Shape checkShapes(Shape in, 
                      KernelIterator kernels, KernelIterator kernelsEnd,
                      OutIterator outs);
     
    template <class KernelIterator, class OutIterator>
    Shape checkShapesFourier(Shape in, 
                             KernelIterator kernels, KernelIterator kernelsEnd,
                             OutIterator outs);
     
    template <class KernelIterator, class OutIterator>
    Shape checkShapesComplex(Shape in, 
                             KernelIterator kernels, KernelIterator kernelsEnd,
                             OutIterator outs);
    
    template <class C1, class KernelIterator, class OutIterator>
    void 
    executeManyImpl(MultiArrayView<N, Real, C1> in, 
                    KernelIterator kernels, KernelIterator kernelsEnd,
                    OutIterator outs, VigraFalseType /* useFourierKernel*/);
    
    template <class C1, class KernelIterator, class OutIterator>
    void 
    executeManyImpl(MultiArrayView<N, Real, C1> in, 
                    KernelIterator kernels, KernelIterator kernelsEnd,
                    OutIterator outs, VigraTrueType /* useFourierKernel*/);
    
};    
    
template <unsigned int N, class Real>
void 
FFTWConvolvePlan<N, Real>::init(Shape in, Shape kernel,
                                unsigned int planner_flags)
{
    Shape paddedShape = fftwBestPaddedShapeR2C(in + kernel - Shape(1)),
          complexShape = fftwCorrespondingShapeR2C(paddedShape);
     
    CArray newFourierArray(complexShape), newFourierKernel(complexShape);
    
    Shape realStrides = 2*newFourierArray.stride();
    realStrides[0] = 1;
    RArray newRealArray(paddedShape, realStrides, (Real*)newFourierArray.data());
    RArray newRealKernel(paddedShape, realStrides, (Real*)newFourierKernel.data());
    
    FFTWPlan<N, Real> fplan(newRealArray, newFourierArray, planner_flags);
    FFTWPlan<N, Real> bplan(newFourierArray, newRealArray, planner_flags);
    
    forward_plan = fplan;
    backward_plan = bplan;
    realArray = newRealArray;
    realKernel = newRealKernel;
    fourierArray.swap(newFourierArray);
    fourierKernel.swap(newFourierKernel);
    useFourierKernel = false;
}

template <unsigned int N, class Real>
void 
FFTWConvolvePlan<N, Real>::initFourierKernel(Shape in, Shape kernel,
                                             unsigned int planner_flags)
{
    Shape complexShape = kernel,
          paddedShape  = fftwCorrespondingShapeC2R(complexShape);
    
    for(unsigned int k=0; k<N; ++k)
        vigra_precondition(in[k] <= paddedShape[k],
             "FFTWConvolvePlan::init(): kernel too small for given input.");

    CArray newFourierArray(complexShape), newFourierKernel(complexShape);
    
    Shape realStrides = 2*newFourierArray.stride();
    realStrides[0] = 1;
    RArray newRealArray(paddedShape, realStrides, (Real*)newFourierArray.data());
    RArray newRealKernel(paddedShape, realStrides, (Real*)newFourierKernel.data());
    
    FFTWPlan<N, Real> fplan(newRealArray, newFourierArray, planner_flags);
    FFTWPlan<N, Real> bplan(newFourierArray, newRealArray, planner_flags);
    
    forward_plan = fplan;
    backward_plan = bplan;
    realArray = newRealArray;
    realKernel = newRealKernel;
    fourierArray.swap(newFourierArray);
    fourierKernel.swap(newFourierKernel);
    useFourierKernel = true;
}

template <unsigned int N, class Real>
void 
FFTWConvolvePlan<N, Real>::initComplex(Shape in, Shape kernel,
                                        unsigned int planner_flags)
{
    Shape paddedShape;
    
    if(useFourierKernel)
    {
        for(unsigned int k=0; k<N; ++k)
            vigra_precondition(in[k] <= kernel[k],
                 "FFTWConvolvePlan::init(): kernel too small for given input.");

        paddedShape = kernel;
    }
    else
    {
        paddedShape  = fftwBestPaddedShape(in + kernel - Shape(1));
    }
    
    CArray newFourierArray(paddedShape), newFourierKernel(paddedShape);
    
    FFTWPlan<N, Real> fplan(newFourierArray, newFourierArray, FFTW_FORWARD, planner_flags);
    FFTWPlan<N, Real> bplan(newFourierArray, newFourierArray, FFTW_BACKWARD, planner_flags);
    
    forward_plan = fplan;
    backward_plan = bplan;
    fourierArray.swap(newFourierArray);
    fourierKernel.swap(newFourierKernel);
}

#ifndef DOXYGEN // doxygen documents these functions as free functions

template <unsigned int N, class Real>
template <class C1, class C2, class C3>
void 
FFTWConvolvePlan<N, Real>::execute(MultiArrayView<N, Real, C1> in, 
                                    MultiArrayView<N, Real, C2> kernel,
                                    MultiArrayView<N, Real, C3> out)
{
    vigra_precondition(!useFourierKernel,
       "FFTWConvolvePlan::execute(): plan was generated for Fourier kernel, got spatial kernel.");

    vigra_precondition(in.shape() == out.shape(),
        "FFTWConvolvePlan::execute(): input and output must have the same shape.");
    
    Shape paddedShape = fftwBestPaddedShapeR2C(in.shape() + kernel.shape() - Shape(1)),
          diff = paddedShape - in.shape(), 
          left = div(diff, MultiArrayIndex(2)),
          right = in.shape() + left;
          
    vigra_precondition(paddedShape == realArray.shape(),
       "FFTWConvolvePlan::execute(): shape mismatch between input and plan.");

    detail::fftEmbedArray(in, realArray);
    forward_plan.execute(realArray, fourierArray);

    detail::fftEmbedKernel(kernel, realKernel);
    forward_plan.execute(realKernel, fourierKernel);
    
    fourierArray *= fourierKernel;
    
    backward_plan.execute(fourierArray, realArray);
    
    out = realArray.subarray(left, right);
}

template <unsigned int N, class Real>
template <class C1, class C2, class C3>
void 
FFTWConvolvePlan<N, Real>::execute(MultiArrayView<N, Real, C1> in, 
                                    MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                                    MultiArrayView<N, Real, C3> out)
{
    vigra_precondition(useFourierKernel,
       "FFTWConvolvePlan::execute(): plan was generated for spatial kernel, got Fourier kernel.");

    vigra_precondition(in.shape() == out.shape(),
        "FFTWConvolvePlan::execute(): input and output must have the same shape.");
    
    vigra_precondition(kernel.shape() == fourierArray.shape(),
       "FFTWConvolvePlan::execute(): shape mismatch between kernel and plan.");

    Shape paddedShape = fftwCorrespondingShapeC2R(kernel.shape(), odd(in.shape(0))),
          diff = paddedShape - in.shape(), 
          left = div(diff, MultiArrayIndex(2)),
          right = in.shape() + left;
          
    vigra_precondition(paddedShape == realArray.shape(),
       "FFTWConvolvePlan::execute(): shape mismatch between input and plan.");

    detail::fftEmbedArray(in, realArray);
    forward_plan.execute(realArray, fourierArray);

    fourierKernel = kernel;
    moveDCToHalfspaceUpperLeft(fourierKernel);

    fourierArray *= fourierKernel;
    
    backward_plan.execute(fourierArray, realArray);
    
    out = realArray.subarray(left, right);
}

template <unsigned int N, class Real>
template <class C1, class C2, class C3>
void 
FFTWConvolvePlan<N, Real>::execute(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                                    MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                                    MultiArrayView<N, FFTWComplex<Real>, C3> out)
{
    vigra_precondition(in.shape() == out.shape(),
        "FFTWConvolvePlan::execute(): input and output must have the same shape.");
    
    Shape paddedShape = fourierArray.shape(),
          diff = paddedShape - in.shape(), 
          left = div(diff, MultiArrayIndex(2)),
          right = in.shape() + left;
          
    if(useFourierKernel)
    {
        vigra_precondition(kernel.shape() == fourierArray.shape(),
           "FFTWConvolvePlan::execute(): shape mismatch between kernel and plan.");
           
        fourierKernel = kernel;
        moveDCToUpperLeft(fourierKernel);
    }
    else
    {
        detail::fftEmbedKernel(kernel, fourierKernel);
        forward_plan.execute(fourierKernel, fourierKernel);
    }

    detail::fftEmbedArray(in, fourierArray);
    forward_plan.execute(fourierArray, fourierArray);

    fourierArray *= fourierKernel;
    
    backward_plan.execute(fourierArray, fourierArray);
    
    out = fourierArray.subarray(left, right);
}

template <unsigned int N, class Real>
template <class C1, class KernelIterator, class OutIterator>
void 
FFTWConvolvePlan<N, Real>::executeManyImpl(MultiArrayView<N, Real, C1> in, 
                                           KernelIterator kernels, KernelIterator kernelsEnd,
                                           OutIterator outs, VigraFalseType /*useFourierKernel*/)
{
    vigra_precondition(!useFourierKernel,
       "FFTWConvolvePlan::execute(): plan was generated for Fourier kernel, got spatial kernel.");

    Shape kernelMax = checkShapes(in.shape(), kernels, kernelsEnd, outs),
          paddedShape = fftwBestPaddedShapeR2C(in.shape() + kernelMax - Shape(1)),
          diff = paddedShape - in.shape(), 
          left = div(diff, MultiArrayIndex(2)),
          right = in.shape() + left;
          
    vigra_precondition(paddedShape == realArray.shape(),
       "FFTWConvolvePlan::executeMany(): shape mismatch between input and plan.");

    detail::fftEmbedArray(in, realArray);
    forward_plan.execute(realArray, fourierArray);

    for(; kernels != kernelsEnd; ++kernels, ++outs)
    {
        detail::fftEmbedKernel(*kernels, realKernel);
        forward_plan.execute(realKernel, fourierKernel);
        
        fourierKernel *= fourierArray;
        
        backward_plan.execute(fourierKernel, realKernel);
        
        *outs = realKernel.subarray(left, right);
    }
}

template <unsigned int N, class Real>
template <class C1, class KernelIterator, class OutIterator>
void 
FFTWConvolvePlan<N, Real>::executeManyImpl(MultiArrayView<N, Real, C1> in, 
                                           KernelIterator kernels, KernelIterator kernelsEnd,
                                           OutIterator outs, VigraTrueType /*useFourierKernel*/)
{
    vigra_precondition(useFourierKernel,
       "FFTWConvolvePlan::execute(): plan was generated for spatial kernel, got Fourier kernel.");

    Shape complexShape = checkShapesFourier(in.shape(), kernels, kernelsEnd, outs),
          paddedShape = fftwCorrespondingShapeC2R(complexShape, odd(in.shape(0))),
          diff = paddedShape - in.shape(), 
          left = div(diff, MultiArrayIndex(2)),
          right = in.shape() + left;
          
    vigra_precondition(complexShape == fourierArray.shape(),
       "FFTWConvolvePlan::executeFourierKernelMany(): shape mismatch between kernels and plan.");

    vigra_precondition(paddedShape == realArray.shape(),
       "FFTWConvolvePlan::executeFourierKernelMany(): shape mismatch between input and plan.");

    detail::fftEmbedArray(in, realArray);
    forward_plan.execute(realArray, fourierArray);

    for(; kernels != kernelsEnd; ++kernels, ++outs)
    {
        fourierKernel = *kernels;
        moveDCToHalfspaceUpperLeft(fourierKernel);
        fourierKernel *= fourierArray;
        
        backward_plan.execute(fourierKernel, realKernel);
        
        *outs = realKernel.subarray(left, right);
    }
}

template <unsigned int N, class Real>
template <class C1, class KernelIterator, class OutIterator>
void 
FFTWConvolvePlan<N, Real>::executeMany(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                                       KernelIterator kernels, KernelIterator kernelsEnd,
                                       OutIterator outs)
{
    typedef typename std::iterator_traits<KernelIterator>::value_type KernelArray;
    typedef typename KernelArray::value_type KernelValue;
    typedef typename std::iterator_traits<OutIterator>::value_type OutArray;
    typedef typename OutArray::value_type OutValue;

    vigra_precondition((IsSameType<KernelValue, Complex>::value),
         "FFTWConvolvePlan::executeMany(): kernels have unsuitable value_type.");
    vigra_precondition((IsSameType<OutValue, Complex>::value),
         "FFTWConvolvePlan::executeMany(): outputs have unsuitable value_type.");

    Shape paddedShape = checkShapesComplex(in.shape(), kernels, kernelsEnd, outs),
          diff = paddedShape - in.shape(), 
          left = div(diff, MultiArrayIndex(2)),
          right = in.shape() + left;
          
    detail::fftEmbedArray(in, fourierArray);
    forward_plan.execute(fourierArray, fourierArray);

    for(; kernels != kernelsEnd; ++kernels, ++outs)
    {
        if(useFourierKernel)
        {
            fourierKernel = *kernels;
            moveDCToUpperLeft(fourierKernel);
        }
        else
        {
            detail::fftEmbedKernel(*kernels, fourierKernel);
            forward_plan.execute(fourierKernel, fourierKernel);
        }

        fourierKernel *= fourierArray;
        
        backward_plan.execute(fourierKernel, fourierKernel);
        
        *outs = fourierKernel.subarray(left, right);
    }
}

#endif // DOXYGEN

template <unsigned int N, class Real>
template <class KernelIterator, class OutIterator>
typename FFTWConvolvePlan<N, Real>::Shape 
FFTWConvolvePlan<N, Real>::checkShapes(Shape in, 
                                       KernelIterator kernels, KernelIterator kernelsEnd,
                                       OutIterator outs)
{
    vigra_precondition(kernels != kernelsEnd,
        "FFTWConvolvePlan::checkShapes(): empty kernel sequence.");

    Shape kernelMax;            
    for(; kernels != kernelsEnd; ++kernels, ++outs)
    {
        vigra_precondition(in == outs->shape(),
            "FFTWConvolvePlan::checkShapes(): shape mismatch between input and (one) output.");
        kernelMax = max(kernelMax, kernels->shape());
    }
    vigra_precondition(prod(kernelMax) > 0,
        "FFTWConvolvePlan::checkShapes(): all kernels have size 0.");
    return kernelMax;
}
 
template <unsigned int N, class Real>
template <class KernelIterator, class OutIterator>
typename FFTWConvolvePlan<N, Real>::Shape 
FFTWConvolvePlan<N, Real>::checkShapesFourier(Shape in, 
                                               KernelIterator kernels, KernelIterator kernelsEnd,
                                               OutIterator outs)
{
    vigra_precondition(kernels != kernelsEnd,
        "FFTWConvolvePlan::checkShapesFourier(): empty kernel sequence.");

    Shape complexShape = kernels->shape(),
          paddedShape  = fftwCorrespondingShapeC2R(complexShape);

    for(unsigned int k=0; k<N; ++k)
        vigra_precondition(in[k] <= paddedShape[k],
             "FFTWConvolvePlan::checkShapesFourier(): kernels too small for given input.");

    for(; kernels != kernelsEnd; ++kernels, ++outs)
    {
        vigra_precondition(in == outs->shape(),
            "FFTWConvolvePlan::checkShapesFourier(): shape mismatch between input and (one) output.");
        vigra_precondition(complexShape == kernels->shape(),
            "FFTWConvolvePlan::checkShapesFourier(): all kernels must have the same size.");
    }
    return complexShape;
}

template <unsigned int N, class Real>
template <class KernelIterator, class OutIterator>
typename FFTWConvolvePlan<N, Real>::Shape 
FFTWConvolvePlan<N, Real>::checkShapesComplex(Shape in, 
                                               KernelIterator kernels, KernelIterator kernelsEnd,
                                               OutIterator outs)
{
    vigra_precondition(kernels != kernelsEnd,
        "FFTWConvolvePlan::checkShapesComplex(): empty kernel sequence.");

    Shape kernelShape = kernels->shape();            
    for(; kernels != kernelsEnd; ++kernels, ++outs)
    {
        vigra_precondition(in == outs->shape(),
            "FFTWConvolvePlan::checkShapesComplex(): shape mismatch between input and (one) output.");
        if(useFourierKernel)
        {
            vigra_precondition(kernelShape == kernels->shape(),
                "FFTWConvolvePlan::checkShapesComplex(): Fourier domain kernels must have identical size.");
        }
        else
        {
            kernelShape = max(kernelShape, kernels->shape());
        }
    }
    vigra_precondition(prod(kernelShape) > 0,
        "FFTWConvolvePlan::checkShapesComplex(): all kernels have size 0.");
        
    if(useFourierKernel)
    {
        for(unsigned int k=0; k<N; ++k)
            vigra_precondition(in[k] <= kernelShape[k],
                 "FFTWConvolvePlan::checkShapesComplex(): kernels too small for given input.");
        return kernelShape;
    }
    else
    {
        return fftwBestPaddedShape(in + kernelShape - Shape(1));
    }
}
 
/********************************************************/
/*                                                      */
/*                  FFTWCorrelatePlan                    */
/*                                                      */
/********************************************************/

/** C++ wrapper for a pair of FFTW plans used to perform FFT-based convolution.
 
 The class encapsulates the calls to <tt>fftw_plan_dft_2d</tt>, <tt>fftw_execute</tt>, and
 <tt>fftw_destroy_plan</tt> (and their <tt>float</tt> and <tt>long double</tt> counterparts)
 in an easy-to-use interface. It always creates a pair of plans, one for the forward and one
 for the inverse transform required for convolution.
 
 Usually, you use this class only indirectly via \ref convolveFFT() and its variants.
 You only need this class if you want to have more control about FFTW's planning process
 (by providing non-default planning flags) and/or want to re-use plans for several convolutions.
 
 <b> Usage:</b>
 
 <b>\#include</b> \<vigra/multi_fft.hxx\><br>
 Namespace: vigra
 
 \code
 // convolve a real array with a real kernel
 MultiArray<2, double> src(Shape2(w, h)), dest(Shape2(w, h));
 
 MultiArray<2, double> spatial_kernel(Shape2(9, 9));
 Gaussian<double> gauss(1.0);
 
 for(int y=0; y<9; ++y)
 for(int x=0; x<9; ++x)
 spatial_kernel(x, y) = gauss(x-4.0)*gauss(y-4.0);
 
 // create an optimized plan by measuring the speed of several algorithm variants
 FFTWCorrelatePlan<2, double> plan(src, spatial_kernel, dest, FFTW_MEASURE);
 
 plan.execute(src, spatial_kernel, dest);
 \endcode
 */
template <unsigned int N, class Real>
class FFTWCorrelatePlan
{
    typedef FFTWComplex<Real> Complex;
    typedef MultiArrayView<N, Real, UnstridedArrayTag >     RArray;
    typedef MultiArray<N, Complex, FFTWAllocator<Complex> > CArray;
    
    FFTWPlan<N, Real> forward_plan, backward_plan;
    RArray realArray, realKernel;
    CArray fourierArray, fourierKernel;
    bool useFourierKernel;
    
public:
    
    typedef typename MultiArrayShape<N>::type Shape;
    
    /** \brief Create an empty plan.
     
     The plan can be initialized later by one of the init() functions.
     */
    FFTWCorrelatePlan()
    : useFourierKernel(false)
    {}
    
    /** \brief Create a plan to correlate a real array with a real kernel.
     
     The kernel must be defined in the spatial domain.
     See \ref convolveFFT() for detailed information on required shapes and internal padding.
     
     \arg planner_flags must be a combination of the
     <a href="http://www.fftw.org/doc/Planner-Flags.html">planner
     flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
     optimal algorithm settings or read them from pre-loaded
     <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
     */
    template <class C1, class C2, class C3>
    FFTWCorrelatePlan(MultiArrayView<N, Real, C1> in,
                     MultiArrayView<N, Real, C2> kernel,
                     MultiArrayView<N, Real, C3> out,
                     unsigned int planner_flags = FFTW_ESTIMATE)
    : useFourierKernel(false)
    {
        init(in, kernel, out, planner_flags);
    }
    
    /** \brief Create a plan to correlate a real array with a complex kernel.
     
     The kernel must be defined in the Fourier domain, using the half-space format.
     See \ref convolveFFT() for detailed information on required shapes and internal padding.
     
     \arg planner_flags must be a combination of the
     <a href="http://www.fftw.org/doc/Planner-Flags.html">planner
     flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
     optimal algorithm settings or read them from pre-loaded
     <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
     */
    template <class C1, class C2, class C3>
    FFTWCorrelatePlan(MultiArrayView<N, Real, C1> in,
                     MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                     MultiArrayView<N, Real, C3> out,
                     unsigned int planner_flags = FFTW_ESTIMATE)
    : useFourierKernel(true)
    {
        init(in, kernel, out, planner_flags);
    }
    
    /** \brief Create a plan to corelate a complex array with a complex kernel.
     
     See \ref convolveFFT() for detailed information on required shapes and internal padding.
     
     \arg fourierDomainKernel determines if the kernel is defined in the spatial or
     Fourier domain.
     \arg planner_flags must be a combination of the
     <a href="http://www.fftw.org/doc/Planner-Flags.html">planner
     flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
     optimal algorithm settings or read them from pre-loaded
     <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
     */
    template <class C1, class C2, class C3>
    FFTWCorrelatePlan(MultiArrayView<N, FFTWComplex<Real>, C1> in,
                     MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                     MultiArrayView<N, FFTWComplex<Real>, C3> out,
                     bool fourierDomainKernel,
                     unsigned int planner_flags = FFTW_ESTIMATE)
    {
        init(in, kernel, out, fourierDomainKernel, planner_flags);
    }
    
    
    /** \brief Create a plan from just the shape information.
     
     See \ref convolveFFT() for detailed information on required shapes and internal padding.
     
     \arg fourierDomainKernel determines if the kernel is defined in the spatial or
     Fourier domain.
     \arg planner_flags must be a combination of the
     <a href="http://www.fftw.org/doc/Planner-Flags.html">planner
     flags</a> defined by the FFTW library. The default <tt>FFTW_ESTIMATE</tt> will guess
     optimal algorithm settings or read them from pre-loaded
     <a href="http://www.fftw.org/doc/Wisdom.html">"wisdom"</a>.
     */
    template <class C1, class C2, class C3>
    FFTWCorrelatePlan(Shape inOut, Shape kernel,
                     bool useFourierKernel = false,
                     unsigned int planner_flags = FFTW_ESTIMATE)
    {
        if(useFourierKernel)
            init(inOut, kernel, planner_flags);
        else
            initFourierKernel(inOut, kernel, planner_flags);
    }
    
    /** \brief Init a plan to convolve a real array with a real kernel.
     
     See the constructor with the same signature for details.
     */
    template <class C1, class C2, class C3>
    void init(MultiArrayView<N, Real, C1> in,
              MultiArrayView<N, Real, C2> kernel,
              MultiArrayView<N, Real, C3> out,
              unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.shape() == out.shape(),
                           "FFTWCorrelatePlan::init(): input and output must have the same shape.");
        init(in.shape(), kernel.shape(), planner_flags);
    }
    
    /** \brief Init a plan to convolve a real array with a complex kernel.
     
     See the constructor with the same signature for details.
     */
    template <class C1, class C2, class C3>
    void init(MultiArrayView<N, Real, C1> in,
              MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
              MultiArrayView<N, Real, C3> out,
              unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.shape() == out.shape(),
                           "FFTWCorrelatePlan::init(): input and output must have the same shape.");
        initFourierKernel(in.shape(), kernel.shape(), planner_flags);
    }
    
    /** \brief Init a plan to convolve a complex array with a complex kernel.
     
     See the constructor with the same signature for details.
     */
    template <class C1, class C2, class C3>
    void init(MultiArrayView<N, FFTWComplex<Real>, C1> in,
              MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
              MultiArrayView<N, FFTWComplex<Real>, C3> out,
              bool fourierDomainKernel,
              unsigned int planner_flags = FFTW_ESTIMATE)
    {
        vigra_precondition(in.shape() == out.shape(),
                           "FFTWCorrelatePlan::init(): input and output must have the same shape.");
        useFourierKernel = fourierDomainKernel;
        initComplex(in.shape(), kernel.shape(), planner_flags);
    }
    
    void init(Shape inOut, Shape kernel,
              unsigned int planner_flags = FFTW_ESTIMATE);
    
    void initFourierKernel(Shape inOut, Shape kernel,
                           unsigned int planner_flags = FFTW_ESTIMATE);
    
    void initComplex(Shape inOut, Shape kernel,
                     unsigned int planner_flags = FFTW_ESTIMATE);
    
    /** \brief Execute a plan to convolve a real array with a real kernel.
     
     The array shapes must be the same as in the corresponding init function
     or constructor. However, execute() can be called several times on
     the same plan, even with different arrays, as long as they have the appropriate
     shapes.
     */
    template <class C1, class C2, class C3>
    void execute(MultiArrayView<N, Real, C1> in,
                 MultiArrayView<N, Real, C2> kernel,
                 MultiArrayView<N, Real, C3> out);
    
    /** \brief Execute a plan to convolve a real array with a complex kernel.
     
     The array shapes must be the same as in the corresponding init function
     or constructor. However, execute() can be called several times on
     the same plan, even with different arrays, as long as they have the appropriate
     shapes.
     */
    template <class C1, class C2, class C3>
    void execute(MultiArrayView<N, Real, C1> in,
                 MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                 MultiArrayView<N, Real, C3> out);
    
    /** \brief Execute a plan to convolve a complex array with a complex kernel.
     
     The array shapes must be the same as in the corresponding init function
     or constructor. However, execute() can be called several times on
     the same plan, even with different arrays, as long as they have the appropriate
     shapes.
     */
    template <class C1, class C2, class C3>
    void execute(MultiArrayView<N, FFTWComplex<Real>, C1> in,
                 MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                 MultiArrayView<N, FFTWComplex<Real>, C3> out);
};

template <unsigned int N, class Real>
void
FFTWCorrelatePlan<N, Real>::init(Shape in, Shape kernel,
                                unsigned int planner_flags)
{
    Shape paddedShape = fftwBestPaddedShapeR2C(in + kernel - Shape(1)),
    complexShape = fftwCorrespondingShapeR2C(paddedShape);
    
    CArray newFourierArray(complexShape), newFourierKernel(complexShape);
    
    Shape realStrides = 2*newFourierArray.stride();
    realStrides[0] = 1;
    RArray newRealArray(paddedShape, realStrides, (Real*)newFourierArray.data());
    RArray newRealKernel(paddedShape, realStrides, (Real*)newFourierKernel.data());
    
    FFTWPlan<N, Real> fplan(newRealArray, newFourierArray, planner_flags);
    FFTWPlan<N, Real> bplan(newFourierArray, newRealArray, planner_flags);
    
    forward_plan = fplan;
    backward_plan = bplan;
    realArray = newRealArray;
    realKernel = newRealKernel;
    fourierArray.swap(newFourierArray);
    fourierKernel.swap(newFourierKernel);
    useFourierKernel = false;
}

template <unsigned int N, class Real>
void
FFTWCorrelatePlan<N, Real>::initFourierKernel(Shape in, Shape kernel,
                                             unsigned int planner_flags)
{
    Shape complexShape = kernel,
    paddedShape  = fftwCorrespondingShapeC2R(complexShape);
    
    for(unsigned int k=0; k<N; ++k)
        vigra_precondition(in[k] <= paddedShape[k],
                           "FFTWCorrelatePlan::init(): kernel too small for given input.");
    
    CArray newFourierArray(complexShape), newFourierKernel(complexShape);
    
    Shape realStrides = 2*newFourierArray.stride();
    realStrides[0] = 1;
    RArray newRealArray(paddedShape, realStrides, (Real*)newFourierArray.data());
    RArray newRealKernel(paddedShape, realStrides, (Real*)newFourierKernel.data());
    
    FFTWPlan<N, Real> fplan(newRealArray, newFourierArray, planner_flags);
    FFTWPlan<N, Real> bplan(newFourierArray, newRealArray, planner_flags);
    
    forward_plan = fplan;
    backward_plan = bplan;
    realArray = newRealArray;
    realKernel = newRealKernel;
    fourierArray.swap(newFourierArray);
    fourierKernel.swap(newFourierKernel);
    useFourierKernel = true;
}

template <unsigned int N, class Real>
void
FFTWCorrelatePlan<N, Real>::initComplex(Shape in, Shape kernel,
                                       unsigned int planner_flags)
{
    Shape paddedShape;
    
    if(useFourierKernel)
    {
        for(unsigned int k=0; k<N; ++k)
            vigra_precondition(in[k] <= kernel[k],
                               "FFTWCorrelatePlan::init(): kernel too small for given input.");
        
        paddedShape = kernel;
    }
    else
    {
        paddedShape  = fftwBestPaddedShape(in + kernel - Shape(1));
    }
    
    CArray newFourierArray(paddedShape), newFourierKernel(paddedShape);
    
    FFTWPlan<N, Real> fplan(newFourierArray, newFourierArray, FFTW_FORWARD, planner_flags);
    FFTWPlan<N, Real> bplan(newFourierArray, newFourierArray, FFTW_BACKWARD, planner_flags);
    
    forward_plan = fplan;
    backward_plan = bplan;
    fourierArray.swap(newFourierArray);
    fourierKernel.swap(newFourierKernel);
}

#ifndef DOXYGEN // doxygen documents these functions as free functions

template <unsigned int N, class Real>
template <class C1, class C2, class C3>
void
FFTWCorrelatePlan<N, Real>::execute(MultiArrayView<N, Real, C1> in,
                                   MultiArrayView<N, Real, C2> kernel,
                                   MultiArrayView<N, Real, C3> out)
{
    vigra_precondition(!useFourierKernel,
                       "FFTWCorrelatePlan::execute(): plan was generated for Fourier kernel, got spatial kernel.");
    
    vigra_precondition(in.shape() == out.shape(),
                       "FFTWCorrelatePlan::execute(): input and output must have the same shape.");
    
    Shape paddedShape = fftwBestPaddedShapeR2C(in.shape() + kernel.shape() - Shape(1)),
    diff = paddedShape - in.shape(),
    left = div(diff, MultiArrayIndex(2)),
    right = in.shape() + left;
    
    vigra_precondition(paddedShape == realArray.shape(),
                       "FFTWCorrelatePlan::execute(): shape mismatch between input and plan.");
    
    detail::fftEmbedArray(in, realArray);
    forward_plan.execute(realArray, fourierArray);
    
    detail::fftEmbedKernel(kernel, realKernel);
    forward_plan.execute(realKernel, fourierKernel);
    
    using namespace vigra::multi_math;
    fourierArray *= conj(fourierKernel);
    
    backward_plan.execute(fourierArray, realArray);
    
    out = realArray.subarray(left, right);
}

template <unsigned int N, class Real>
template <class C1, class C2, class C3>
void
FFTWCorrelatePlan<N, Real>::execute(MultiArrayView<N, Real, C1> in,
                                   MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                                   MultiArrayView<N, Real, C3> out)
{
    vigra_precondition(useFourierKernel,
                       "FFTWCorrelatePlan::execute(): plan was generated for spatial kernel, got Fourier kernel.");
    
    vigra_precondition(in.shape() == out.shape(),
                       "FFTWCorrelatePlan::execute(): input and output must have the same shape.");
    
    vigra_precondition(kernel.shape() == fourierArray.shape(),
                       "FFTWCorrelatePlan::execute(): shape mismatch between kernel and plan.");
    
    Shape paddedShape = fftwCorrespondingShapeC2R(kernel.shape(), odd(in.shape(0))),
    diff = paddedShape - in.shape(),
    left = div(diff, MultiArrayIndex(2)),
    right = in.shape() + left;
    
    vigra_precondition(paddedShape == realArray.shape(),
                       "FFTWCorrelatePlan::execute(): shape mismatch between input and plan.");
    
    detail::fftEmbedArray(in, realArray);
    forward_plan.execute(realArray, fourierArray);
    
    fourierKernel = kernel;
    moveDCToHalfspaceUpperLeft(fourierKernel);
    
    using namespace vigra::multi_math;
    fourierArray *= conj(fourierKernel);
    
    backward_plan.execute(fourierArray, realArray);
    
    out = realArray.subarray(left, right);
}

template <unsigned int N, class Real>
template <class C1, class C2, class C3>
void
FFTWCorrelatePlan<N, Real>::execute(MultiArrayView<N, FFTWComplex<Real>, C1> in,
                                   MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                                   MultiArrayView<N, FFTWComplex<Real>, C3> out)
{
    vigra_precondition(in.shape() == out.shape(),
                       "FFTWCorrelatePlan::execute(): input and output must have the same shape.");
    
    Shape paddedShape = fourierArray.shape(),
    diff = paddedShape - in.shape(),
    left = div(diff, MultiArrayIndex(2)),
    right = in.shape() + left;
    
    if(useFourierKernel)
    {
        vigra_precondition(kernel.shape() == fourierArray.shape(),
                           "FFTWCorrelatePlan::execute(): shape mismatch between kernel and plan.");
        
        fourierKernel = kernel;
        moveDCToUpperLeft(fourierKernel);
    }
    else
    {
        detail::fftEmbedKernel(kernel, fourierKernel);
        forward_plan.execute(fourierKernel, fourierKernel);
    }
    
    detail::fftEmbedArray(in, fourierArray);
    forward_plan.execute(fourierArray, fourierArray);
    
    using namespace vigra::multi_math;
    fourierArray *= conj(fourierKernel);
    
    backward_plan.execute(fourierArray, fourierArray);
    
    out = fourierArray.subarray(left, right);
}

#endif // DOXYGEN

/********************************************************/
/*                                                      */
/*                   fourierTransform                   */
/*                                                      */
/********************************************************/

template <unsigned int N, class Real, class C1, class C2>
inline void 
fourierTransform(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                 MultiArrayView<N, FFTWComplex<Real>, C2> out)
{
    FFTWPlan<N, Real>(in, out, FFTW_FORWARD).execute(in, out);
}

template <unsigned int N, class Real, class C1, class C2>
inline void 
fourierTransformInverse(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                        MultiArrayView<N, FFTWComplex<Real>, C2> out)
{
    FFTWPlan<N, Real>(in, out, FFTW_BACKWARD).execute(in, out);
}

template <unsigned int N, class Real, class C1, class C2>
void 
fourierTransform(MultiArrayView<N, Real, C1> in, 
                 MultiArrayView<N, FFTWComplex<Real>, C2> out)
{
    if(in.shape() == out.shape())
    {
        // copy the input array into the output and then perform an in-place FFT
        out = in;
        FFTWPlan<N, Real>(out, out, FFTW_FORWARD).execute(out, out);
    }
    else if(out.shape() == fftwCorrespondingShapeR2C(in.shape()))
    {
        FFTWPlan<N, Real>(in, out).execute(in, out);
    }
    else
        vigra_precondition(false,
            "fourierTransform(): shape mismatch between input and output.");
}

template <unsigned int N, class Real, class C1, class C2>
void 
fourierTransformInverse(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                        MultiArrayView<N, Real, C2> out)
{
    vigra_precondition(in.shape() == fftwCorrespondingShapeR2C(out.shape()),
        "fourierTransformInverse(): shape mismatch between input and output.");
    FFTWPlan<N, Real>(in, out).execute(in, out);
}

//@}

/** \addtogroup MultiArrayConvolutionFilters
*/
//@{

/********************************************************/
/*                                                      */
/*                     convolveFFT                      */
/*                                                      */
/********************************************************/

/** \brief Convolve an array with a kernel by means of the Fourier transform.

    Thanks to the convolution theorem of Fourier theory, a convolution in the spatial domain
    is equivalent to a multiplication in the frequency domain. Thus, for certain kernels
    (especially large, non-separable ones), it is advantageous to perform the convolution by first
    transforming both array and kernel to the frequency domain, multiplying the frequency 
    representations, and transforming the result back into the spatial domain. 
    Some kernels have a much simpler definition in the frequency domain, so that they are readily 
    computed there directly, avoiding Fourier transformation of those kernels. 
    
    The following functions implement various variants of FFT-based convolution:
    
    <DL>
    <DT><b>convolveFFT</b><DD> Convolve a real-valued input array with a kernel such that the 
                        result is also real-valued. That is, the kernel is either provided
                        as a real-valued array in the spatial domain, or as a 
                        complex-valued array in the Fourier domain, using the half-space format 
                        of the R2C Fourier transform (see below).
    <DT><b>convolveFFTMany</b><DD> Like <tt>convolveFFT</tt>, but you may provide many kernels at once 
                        (using an iterator pair specifying the kernel sequence). 
                        This has the advantage that the forward transform of the input array needs 
                        to be executed only once.
    <DT><b>convolveFFTComplex</b><DD> Convolve a complex-valued input array with a complex-valued kernel, 
                        resulting in a complex-valued output array. An additional flag is used to 
                        specify whether the kernel is defined in the spatial or frequency domain.
    <DT><b>convolveFFTComplexMany</b><DD> Like <tt>convolveFFTComplex</tt>, but you may provide many 
                        kernels at once (using an iterator pair specifying the kernel sequence). 
                        This has the advantage that the forward transform of the input array needs 
                        to be executed only once.
    </DL>
    
    The output arrays must have the same shape as the input arrays. In the "Many" variants of the
    convolution functions, the kernels must all have the same shape.
    
    The origin of the kernel is always assumed to be in the center of the kernel array (precisely,
    at the point <tt>floor(kernel.shape() / 2.0)</tt>, except when the half-space format is used, see below). 
    The function \ref moveDCToUpperLeft() will be called internally to align the kernel with the transformed 
    input as appropriate.
    
    If a real input is combined with a real kernel, the kernel is automatically assumed to be defined
    in the spatial domain. If a real input is combined with a complex kernel, the kernel is assumed 
    to be defined in the Fourier domain in half-space format. If the input array is complex, a flag 
    <tt>fourierDomainKernel</tt> determines where the kernel is defined.
    
    When the kernel is defined in the spatial domain, the convolution functions will automatically pad
    (enlarge) the input array by at least the kernel radius in each direction. The newly added space is
    filled according to reflective boundary conditions in order to minimize border artifacts during 
    convolution. It is thus ensured that convolution in the Fourier domain yields the same results as 
    convolution in the spatial domain (e.g. when \ref separableConvolveMultiArray() is called with the 
    same kernel). A little further padding may be added to make sure that the padded array shape
    uses integers which have only small prime factors, because FFTW is then able to use the fastest
    possible algorithms. Any padding is automatically removed from the result arrays before the function
    returns.
    
    When the kernel is defined in the frequency domain, it must be complex-valued, and its shape determines
    the shape of the Fourier representation (i.e. the input is padded according to the shape of the kernel).
    If we are going to perform a complex-valued convolution, the kernel must be defined for the entire 
    frequency domain, and its shape directly determines the size of the FFT. 
    
    In contrast, a frequency domain kernel for a real-valued convolution must have symmetry properties
    that allow to drop half of the kernel coefficients, as in the 
    <a href="http://www.fftw.org/doc/Multi_002dDimensional-DFTs-of-Real-Data.html">R2C transform</a>. 
    That is, the kernel must have the <i>half-space format</i>, that is the shape returned by <tt>fftwCorrespondingShapeR2C(fourier_shape)</tt>, where <tt>fourier_shape</tt> is the desired 
    logical shape of the frequency representation (and thus the size of the padded input). The origin 
    of the kernel must be at the point 
    <tt>(0, floor(fourier_shape[0] / 2.0), ..., floor(fourier_shape[N-1] / 2.0))</tt> 
    (i.e. as in a regular kernel except for the first dimension).
    
    The <tt>Real</tt> type in the declarations can be <tt>double</tt>, <tt>float</tt>, and 
    <tt>long double</tt>. Your program must always link against <tt>libfftw3</tt>. If you use
    <tt>float</tt> or <tt>long double</tt> arrays, you must <i>additionally</i> link against 
    <tt>libfftw3f</tt> and <tt>libfftw3l</tt> respectively.
    
    The Fourier transform functions internally create <a href="http://www.fftw.org/doc/Using-Plans.html">FFTW plans</a>
    which control the algorithm details. The plans are created with the flag <tt>FFTW_ESTIMATE</tt>, i.e.
    optimal settings are guessed or read from saved "wisdom" files. If you need more control over planning,
    you can use the class \ref FFTWConvolvePlan.
    
    See also \ref applyFourierFilter() for corresponding functionality on the basis of the
    old image iterator interface.
    
    <b> Declarations:</b>

    Real-valued convolution with kernel in the spatial domain:
    \code
    namespace vigra {
        template <unsigned int N, class Real, class C1, class C2, class C3>
        void 
        convolveFFT(MultiArrayView<N, Real, C1> in, 
                    MultiArrayView<N, Real, C2> kernel,
                    MultiArrayView<N, Real, C3> out);
    }
    \endcode

    Real-valued convolution with kernel in the Fourier domain (half-space format):
    \code
    namespace vigra {
        template <unsigned int N, class Real, class C1, class C2, class C3>
        void 
        convolveFFT(MultiArrayView<N, Real, C1> in, 
                    MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                    MultiArrayView<N, Real, C3> out);
    }
    \endcode

    Series of real-valued convolutions with kernels in the spatial or Fourier domain 
    (the kernel and out sequences must have the same length):
    \code
    namespace vigra {
        template <unsigned int N, class Real, class C1, 
                  class KernelIterator, class OutIterator>
        void 
        convolveFFTMany(MultiArrayView<N, Real, C1> in, 
                        KernelIterator kernels, KernelIterator kernelsEnd,
                        OutIterator outs);
    }
    \endcode

    Complex-valued convolution (parameter <tt>fourierDomainKernel</tt> determines if
    the kernel is defined in the spatial or Fourier domain):
    \code
    namespace vigra {
        template <unsigned int N, class Real, class C1, class C2, class C3>
        void
        convolveFFTComplex(MultiArrayView<N, FFTWComplex<Real>, C1> in,
                           MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                           MultiArrayView<N, FFTWComplex<Real>, C3> out,
                           bool fourierDomainKernel);
    }
    \endcode

    Series of complex-valued convolutions (parameter <tt>fourierDomainKernel</tt> 
    determines if the kernels are defined in the spatial or Fourier domain, 
    the kernel and out sequences must have the same length):
    \code
    namespace vigra {
        template <unsigned int N, class Real, class C1, 
                  class KernelIterator, class OutIterator>
        void 
        convolveFFTComplexMany(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                               KernelIterator kernels, KernelIterator kernelsEnd,
                               OutIterator outs,
                               bool fourierDomainKernel);
    }
    \endcode

    <b> Usage:</b>

    <b>\#include</b> \<vigra/multi_fft.hxx\><br>
    Namespace: vigra

    \code
    // convolve real array with a Gaussian (sigma=1) defined in the spatial domain
    // (implicitly uses padding by at least 4 pixels)
    MultiArray<2, double> src(Shape2(w, h)), dest(Shape2(w,h));
    
    MultiArray<2, double> spatial_kernel(Shape2(9, 9));
    Gaussian<double> gauss(1.0);
    
    for(int y=0; y<9; ++y)
        for(int x=0; x<9; ++x)
            spatial_kernel(x, y) = gauss(x-4.0)*gauss(y-4.0);

    convolveFFT(src, spatial_kernel, dest);
    
    // convolve real array with a Gaussian (sigma=1) defined in the Fourier domain
    // (uses no padding, because the kernel size corresponds to the input size)
    MultiArray<2, FFTWComplex<double> > fourier_kernel(fftwCorrespondingShapeR2C(src.shape()));
    int y0 = h / 2;
        
    for(int y=0; y<fourier_kernel.shape(1); ++y)
        for(int x=0; x<fourier_kernel.shape(0); ++x)
            fourier_kernel(x, y) = exp(-0.5*sq(x / double(w))) * exp(-0.5*sq((y-y0)/double(h)));

    convolveFFT(src, fourier_kernel, dest);
    \endcode
*/
doxygen_overloaded_function(template <...> void convolveFFT)

template <unsigned int N, class Real, class C1, class C2, class C3>
void 
convolveFFT(MultiArrayView<N, Real, C1> in, 
            MultiArrayView<N, Real, C2> kernel,
            MultiArrayView<N, Real, C3> out)
{
    FFTWConvolvePlan<N, Real>(in, kernel, out).execute(in, kernel, out);
}

template <unsigned int N, class Real, class C1, class C2, class C3>
void 
convolveFFT(MultiArrayView<N, Real, C1> in, 
            MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
            MultiArrayView<N, Real, C3> out)
{
    FFTWConvolvePlan<N, Real>(in, kernel, out).execute(in, kernel, out);
}

/** \brief Convolve a complex-valued array by means of the Fourier transform.

    See \ref convolveFFT() for details.
*/
doxygen_overloaded_function(template <...> void convolveFFTComplex)

template <unsigned int N, class Real, class C1, class C2, class C3>
void
convolveFFTComplex(MultiArrayView<N, FFTWComplex<Real>, C1> in,
            MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
            MultiArrayView<N, FFTWComplex<Real>, C3> out,
            bool fourierDomainKernel)
{
    FFTWConvolvePlan<N, Real>(in, kernel, out, fourierDomainKernel).execute(in, kernel, out);
}

/** \brief Convolve a real-valued array with a sequence of kernels by means of the Fourier transform.

    See \ref convolveFFT() for details.
*/
doxygen_overloaded_function(template <...> void convolveFFTMany)

template <unsigned int N, class Real, class C1, 
          class KernelIterator, class OutIterator>
void 
convolveFFTMany(MultiArrayView<N, Real, C1> in, 
                KernelIterator kernels, KernelIterator kernelsEnd,
                OutIterator outs)
{
    FFTWConvolvePlan<N, Real> plan;
    plan.initMany(in, kernels, kernelsEnd, outs);
    plan.executeMany(in, kernels, kernelsEnd, outs);
}

/** \brief Convolve a complex-valued array with a sequence of kernels by means of the Fourier transform.

    See \ref convolveFFT() for details.
*/
doxygen_overloaded_function(template <...> void convolveFFTComplexMany)

template <unsigned int N, class Real, class C1, 
          class KernelIterator, class OutIterator>
void 
convolveFFTComplexMany(MultiArrayView<N, FFTWComplex<Real>, C1> in, 
                KernelIterator kernels, KernelIterator kernelsEnd,
                OutIterator outs,
                bool fourierDomainKernel)
{
    FFTWConvolvePlan<N, Real> plan;
    plan.initMany(in, kernels, kernelsEnd, outs, fourierDomainKernel);
    plan.executeMany(in, kernels, kernelsEnd, outs);
}
    
/********************************************************/
/*                                                      */
/*                     correlateFFT                      */
/*                                                      */
/********************************************************/

/** \brief Correlate an array with a kernel by means of the Fourier transform.
 
 Thanks to the convolution theorem of Fourier theory, a correlation in the spatial domain
 is equivalent to a multiplication of the complex conjugate in the frequency domain. Thus, for
 certain kernels (especially large, non-separable ones), it is advantageous to perform the 
 correlation by first transforming both array and kernel to the frequency domain, multiplying
 the frequency representations, and transforming the result back into the spatial domain.
 Some kernels have a much simpler definition in the frequency domain, so that they are readily
 computed there directly, avoiding Fourier transformation of those kernels.
 
 The following functions implement various variants of FFT-based convolution:
 
 <DL>
 <DT><b>correlateFFT</b><DD> Correlate a real-valued input array with a kernel such that the
 result is also real-valued. That is, the kernel is either provided
 as a real-valued array in the spatial domain, or as a
 complex-valued array in the Fourier domain, using the half-space format
 of the R2C Fourier transform (see below).
 <DT><b>correlateFFTMany</b><DD> Like <tt>correlateFFT</tt>, but you may provide many kernels at once
 (using an iterator pair specifying the kernel sequence).
 This has the advantage that the forward transform of the input array needs
 to be executed only once.
 <DT><b>correlateFFTComplex</b><DD> Correlate a complex-valued input array with a complex-valued kernel,
 resulting in a complex-valued output array. An additional flag is used to
 specify whether the kernel is defined in the spatial or frequency domain.
 <DT><b>correlateFFTComplexMany</b><DD> Like <tt>correlateFFTComplex</tt>, but you may provide many
 kernels at once (using an iterator pair specifying the kernel sequence).
 This has the advantage that the forward transform of the input array needs
 to be executed only once.
 </DL>
 
 The output arrays must have the same shape as the input arrays. In the "Many" variants of the
 convolution functions, the kernels must all have the same shape.
 
 See also \ref convolveFFT() for corresponding functionality.
 
 <b> Declarations:</b>
 
 Real-valued convolution with kernel in the spatial domain:
 \code
 namespace vigra {
 template <unsigned int N, class Real, class C1, class C2, class C3>
 void
 correlateFFT(MultiArrayView<N, Real, C1> in,
 MultiArrayView<N, Real, C2> kernel,
 MultiArrayView<N, Real, C3> out);
 }
 \endcode
 
 Real-valued convolution with kernel in the Fourier domain (half-space format):
 \code
 namespace vigra {
 template <unsigned int N, class Real, class C1, class C2, class C3>
 void
 correlateFFT(MultiArrayView<N, Real, C1> in,
 MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
 MultiArrayView<N, Real, C3> out);
 }
 \endcode
 
 Series of real-valued convolutions with kernels in the spatial or Fourier domain
 (the kernel and out sequences must have the same length):
 \code
 namespace vigra {
 template <unsigned int N, class Real, class C1,
 class KernelIterator, class OutIterator>
 void
 correlateFFTMany(MultiArrayView<N, Real, C1> in,
 KernelIterator kernels, KernelIterator kernelsEnd,
 OutIterator outs);
 }
 \endcode
 
 Complex-valued convolution (parameter <tt>fourierDomainKernel</tt> determines if
 the kernel is defined in the spatial or Fourier domain):
 \code
 namespace vigra {
 template <unsigned int N, class Real, class C1, class C2, class C3>
 void
 correlateFFTComplex(MultiArrayView<N, FFTWComplex<Real>, C1> in,
 MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
 MultiArrayView<N, FFTWComplex<Real>, C3> out,
 bool fourierDomainKernel);
 }
 \endcode
 
 Series of complex-valued convolutions (parameter <tt>fourierDomainKernel</tt>
 determines if the kernels are defined in the spatial or Fourier domain,
 the kernel and out sequences must have the same length):
 \code
 namespace vigra {
 template <unsigned int N, class Real, class C1,
 class KernelIterator, class OutIterator>
 void
 correlateFFTComplexMany(MultiArrayView<N, FFTWComplex<Real>, C1> in,
 KernelIterator kernels, KernelIterator kernelsEnd,
 OutIterator outs,
 bool fourierDomainKernel);
 }
 \endcode
 
 <b> Usage:</b>
 
 <b>\#include</b> \<vigra/multi_fft.hxx\><br>
 Namespace: vigra
 
 \code
 // correlate real array with a Gaussian (sigma=1) defined in the spatial domain
 // (implicitly uses padding by at least 4 pixels)
 MultiArray<2, double> src(Shape2(w, h)), dest(Shape2(w,h));
 
 MultiArray<2, double> spatial_kernel(Shape2(9, 9));
 Gaussian<double> gauss(1.0);
 
 for(int y=0; y<9; ++y)
 for(int x=0; x<9; ++x)
 spatial_kernel(x, y) = gauss(x-4.0)*gauss(y-4.0);
 
 correlateFFT(src, spatial_kernel, dest);
 
 // correlate real array with a Gaussian (sigma=1) defined in the Fourier domain
 // (uses no padding, because the kernel size corresponds to the input size)
 MultiArray<2, FFTWComplex<double> > fourier_kernel(fftwCorrespondingShapeR2C(src.shape()));
 int y0 = h / 2;
 
 for(int y=0; y<fourier_kernel.shape(1); ++y)
 for(int x=0; x<fourier_kernel.shape(0); ++x)
 fourier_kernel(x, y) = exp(-0.5*sq(x / double(w))) * exp(-0.5*sq((y-y0)/double(h)));
 
 correlateFFT(src, fourier_kernel, dest);
 \endcode
 */
doxygen_overloaded_function(template <...> void correlateFFT)

template <unsigned int N, class Real, class C1, class C2, class C3>
void
correlateFFT(MultiArrayView<N, Real, C1> in,
            MultiArrayView<N, Real, C2> kernel,
            MultiArrayView<N, Real, C3> out)
{
    FFTWCorrelatePlan<N, Real>(in, kernel, out).execute(in, kernel, out);
}

template <unsigned int N, class Real, class C1, class C2, class C3>
void
correlateFFT(MultiArrayView<N, Real, C1> in,
            MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
            MultiArrayView<N, Real, C3> out)
{
    FFTWCorrelatePlan<N, Real>(in, kernel, out).execute(in, kernel, out);
}

/** \brief Correlate a complex-valued array by means of the Fourier transform.
 
 See \ref correlateFFT() for details.
 */
doxygen_overloaded_function(template <...> void correlateFFTComplex)

template <unsigned int N, class Real, class C1, class C2, class C3>
void
correlateFFTComplex(MultiArrayView<N, FFTWComplex<Real>, C1> in,
                   MultiArrayView<N, FFTWComplex<Real>, C2> kernel,
                   MultiArrayView<N, FFTWComplex<Real>, C3> out,
                   bool fourierDomainKernel)
{
    FFTWCorrelatePlan<N, Real>(in, kernel, out, fourierDomainKernel).execute(in, kernel, out);
}

/** \brief Correlate a real-valued array with a sequence of kernels by means of the Fourier transform.
 
 See \ref correlateFFT() for details.
 */
doxygen_overloaded_function(template <...> void correlateFFTMany)

template <unsigned int N, class Real, class C1,
class KernelIterator, class OutIterator>
void
correlateFFTMany(MultiArrayView<N, Real, C1> in,
                KernelIterator kernels, KernelIterator kernelsEnd,
                OutIterator outs)
{
    FFTWCorrelatePlan<N, Real> plan;
    plan.initMany(in, kernels, kernelsEnd, outs);
    plan.executeMany(in, kernels, kernelsEnd, outs);
}

/** \brief Correlate a complex-valued array with a sequence of kernels by means of the Fourier transform.
 
 See \ref correlateFFT() for details.
 */
doxygen_overloaded_function(template <...> void correlateFFTComplexMany)

template <unsigned int N, class Real, class C1,
class KernelIterator, class OutIterator>
void
correlateFFTComplexMany(MultiArrayView<N, FFTWComplex<Real>, C1> in,
                       KernelIterator kernels, KernelIterator kernelsEnd,
                       OutIterator outs,
                       bool fourierDomainKernel)
{
    FFTWCorrelatePlan<N, Real> plan;
    plan.initMany(in, kernels, kernelsEnd, outs, fourierDomainKernel);
    plan.executeMany(in, kernels, kernelsEnd, outs);
}

//@}

} // namespace vigra

#endif // VIGRA_MULTI_FFT_HXX
