/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing the properties of air
 * Author:   Jason Forthofer <jforthofer@gmail.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include "air.h"

Air::Air()
{
    nRows = 35;

    t = new double[nRows];
    rho = new double[nRows];
    cSubP = new double[nRows];
    mu = new double[nRows];
    v = new double[nRows];
    k = new double[nRows];
    alpha = new double[nRows];
    pr = new double[nRows];

    //TEMPERATURE->K
    t[0] = 100;
    t[1] = 150;
    t[2] = 200;
    t[3] = 250;
    t[4] = 300;
    t[5] = 350;
    t[6] = 400;
    t[7] = 450;
    t[8] = 500;
    t[9] = 550;
    t[10] = 600;
    t[11] = 650;
    t[12] = 700;
    t[13] = 750;
    t[14] = 800;
    t[15] = 850;
    t[16] = 900;
    t[17] = 950;
    t[18] = 1000;
    t[19] = 1100;
    t[20] = 1200;
    t[21] = 1300;
    t[22] = 1400;
    t[23] = 1500;
    t[24] = 1600;
    t[25] = 1700;
    t[26] = 1800;
    t[27] = 1900;
    t[28] = 2000;
    t[29] = 2100;
    t[30] = 2200;
    t[31] = 2300;
    t[32] = 2400;
    t[33] = 2500;
    t[34] = 3000;

    //RHO->KG/M^3
    rho[0] = 3.5562;
    rho[1] = 2.3364;
    rho[2] = 1.7458;
    rho[3] = 1.3947;
    rho[4] = 1.1614;
    rho[5] = 0.995;
    rho[6] = 0.8711;
    rho[7] = 0.774;
    rho[8] = 0.6964;
    rho[9] = 0.6329;
    rho[10] = 0.5804;
    rho[11] = 0.5356;
    rho[12] = 0.4975;
    rho[13] = 0.4643;
    rho[14] = 0.4354;
    rho[15] = 0.4097;
    rho[16] = 0.3868;
    rho[17] = 0.3666;
    rho[18] = 0.3482;
    rho[19] = 0.3166;
    rho[20] = 0.2902;
    rho[21] = 0.2679;
    rho[22] = 0.2488;
    rho[23] = 0.2322;
    rho[24] = 0.2177;
    rho[25] = 0.2049;
    rho[26] = 0.1935;
    rho[27] = 0.1833;
    rho[28] = 0.1741;
    rho[29] = 0.1658;
    rho[30] = 0.1582;
    rho[31] = 0.1513;
    rho[32] = 0.1448;
    rho[33] = 0.1389;
    rho[34] = 0.1135;

    //CSUBP->J/KG*K
    cSubP[0] =1032;
    cSubP[1] =1012;
    cSubP[2] =1007;
    cSubP[3] =1006;
    cSubP[4] =1007;
    cSubP[5] =1009;
    cSubP[6] =1014;
    cSubP[7] =1021;
    cSubP[8] =1030;
    cSubP[9] = 1040;
    cSubP[10] = 1051;
    cSubP[11] = 1063;
    cSubP[12] = 1075;
    cSubP[13] = 1087;
    cSubP[14] = 1099;
    cSubP[15] = 1110;
    cSubP[16] = 1121;
    cSubP[17] = 1131;
    cSubP[18] = 1141;
    cSubP[19] = 1159;
    cSubP[20] = 1175;
    cSubP[21] = 1189;
    cSubP[22] = 1207;
    cSubP[23] = 1230;
    cSubP[24] = 1248;
    cSubP[25] = 1267;
    cSubP[26] = 1286;
    cSubP[27] = 1307;
    cSubP[28] = 1337;
    cSubP[29] = 1372;
    cSubP[30] = 1417;
    cSubP[31] = 1478;
    cSubP[32] = 1558;
    cSubP[33] = 1665;
    cSubP[34] = 2726;

    //M->N*S/M^2
    mu[0] = 0.00000711;
    mu[1] = 0.00001034;
    mu[2] = 0.00001325;
    mu[3] = 0.00001596;
    mu[4] = 0.00001846;
    mu[5] = 0.00002082;
    mu[6] = 0.00002301;
    mu[7] = 0.00002507;
    mu[8] = 0.00002701;
    mu[9] = 0.00002884;
    mu[10] = 0.00003058;
    mu[11] = 0.00003225;
    mu[12] = 0.00003388;
    mu[13] = 0.00003546;
    mu[14] = 0.00003698;
    mu[15] = 0.00003843;
    mu[16] = 0.00003981;
    mu[17] = 0.00004113;
    mu[18] = 0.00004244;
    mu[19] = 0.0000449;
    mu[20] = 0.0000473;
    mu[21] = 0.0000496;
    mu[22] = 0.000053;
    mu[23] = 0.0000557;
    mu[24] = 0.0000584;
    mu[25] = 0.0000611;
    mu[26] = 0.0000637;
    mu[27] = 0.0000663;
    mu[28] = 0.0000689;
    mu[29] = 0.0000715;
    mu[30] = 0.000074;
    mu[31] = 0.0000766;
    mu[32] = 0.0000792;
    mu[33] = 0.0000818;
    mu[34] = 0.0000955;

    //V->M^2/S
    v[0] = 0.000002;
    v[1] = 0.000004426;
    v[2] = 0.00000759;
    v[3] = 0.00001144;
    v[4] = 0.00001589;
    v[5] = 0.00002092;
    v[6] = 0.00002641;
    v[7] = 0.00003239;
    v[8] = 0.00003879;
    v[9] = 0.00004557;
    v[10] = 0.00005269;
    v[11] = 0.00006021;
    v[12] = 0.0000681;
    v[13] = 0.00007637;
    v[14] = 0.00008493;
    v[15] = 0.0000938;
    v[16] = 0.0001029;
    v[17] = 0.0001122;
    v[18] = 0.0001219;
    v[19] = 0.0001418;
    v[20] = 0.0001629;
    v[21] = 0.0001851;
    v[22] = 0.000213;
    v[23] = 0.00024;
    v[24] = 0.000268;
    v[25] = 0.000298;
    v[26] = 0.000329;
    v[27] = 0.000362;
    v[28] = 0.000396;
    v[29] = 0.000431;
    v[30] = 0.000468;
    v[31] = 0.000506;
    v[32] = 0.000547;
    v[33] = 0.000589;
    v[34] = 0.000841;

    //K->W/M*K
    k[0] = 0.00934;
    k[1] = 0.0138;
    k[2] = 0.0181;
    k[3] = 0.0223;
    k[4] = 0.0263;
    k[5] = 0.03;
    k[6] = 0.0338;
    k[7] = 0.0373;
    k[8] = 0.0407;
    k[9] = 0.0439;
    k[10] = 0.0469;
    k[11] = 0.0497;
    k[12] = 0.0524;
    k[13] = 0.0549;
    k[14] = 0.0573;
    k[15] = 0.0596;
    k[16] = 0.062;
    k[17] = 0.0643;
    k[18] = 0.0667;
    k[19] = 0.0715;
    k[20] = 0.0763;
    k[21] = 0.082;
    k[22] = 0.091;
    k[23] = 0.1;
    k[24] = 0.106;
    k[25] = 0.113;
    k[26] = 0.12;
    k[27] = 0.128;
    k[28] = 0.137;
    k[29] = 0.147;
    k[30] = 0.16;
    k[31] = 0.175;
    k[32] = 0.196;
    k[33] = 0.222;
    k[34] = 0.486;

    //ALPHA->M^2/S
    alpha[0] = 0.00000254;
    alpha[1] = 0.00000584;
    alpha[2] = 0.0000103;
    alpha[3] = 0.0000159;
    alpha[4] = 0.0000225;
    alpha[5] = 0.0000299;
    alpha[6] = 0.0000383;
    alpha[7] = 0.0000472;
    alpha[8] = 0.0000567;
    alpha[9] = 0.0000667;
    alpha[10] = 0.0000769;
    alpha[11] = 0.0000873;
    alpha[12] = 0.000098;
    alpha[13] = 0.000109;
    alpha[14] = 0.00012;
    alpha[15] = 0.000131;
    alpha[16] = 0.000143;
    alpha[17] = 0.000155;
    alpha[18] = 0.000168;
    alpha[19] = 0.000195;
    alpha[20] = 0.000224;
    alpha[21] = 0.000238;
    alpha[22] = 0.000303;
    alpha[23] = 0.00035;
    alpha[24] = 0.00039;
    alpha[25] = 0.000435;
    alpha[26] = 0.000482;
    alpha[27] = 0.000534;
    alpha[28] = 0.000589;
    alpha[29] = 0.000646;
    alpha[30] = 0.000714;
    alpha[31] = 0.000783;
    alpha[32] = 0.000869;
    alpha[33] = 0.00096;
    alpha[34] = 0.00157;

    //PR->N/A
    pr[0] = 0.786;
    pr[1] = 0.758;
    pr[2] = 0.737;
    pr[3] = 0.72;
    pr[4] = 0.707;
    pr[5] = 0.7;
    pr[6] = 0.69;
    pr[7] = 0.686;
    pr[8] = 0.684;
    pr[9] = 0.683;
    pr[10] = 0.685;
    pr[11] = 0.69;
    pr[12] = 0.695;
    pr[13] = 0.702;
    pr[14] = 0.709;
    pr[15] = 0.716;
    pr[16] = 0.72;
    pr[17] = 0.723;
    pr[18] = 0.726;
    pr[19] = 0.728;
    pr[20] = 0.728;
    pr[21] = 0.719;
    pr[22] = 0.703;
    pr[23] = 0.685;
    pr[24] = 0.688;
    pr[25] = 0.685;
    pr[26] = 0.683;
    pr[27] = 0.677;
    pr[28] = 0.672;
    pr[29] = 0.667;
    pr[30] = 0.655;
    pr[31] = 0.647;
    pr[32] = 0.63;
    pr[33] = 0.613;
    pr[34] = 0.536;
}

Air::~Air()
{
}
