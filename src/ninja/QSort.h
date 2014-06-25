/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  sorting function
 * Author:   
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

void QSort(double *x, long n)
{
    //std::cout << "Sorting Data..." << endl;
    int Q_ALLOC = 16; 
    int ip, iup, lp;
    int *lv, *iv, *temp, numalloc=1;
    register double y;

    lv=new int[Q_ALLOC];
    iv=new int[Q_ALLOC];

    lv[0] = 0;
    iv[0] = n - 1;
    ip = 0;

    while(ip >= 0)
    {
        if((iv[ip] - lv[ip]) < 1)
        {       
            ip--;
            continue;
        }

        lp = lv[ip] - 1;
        iup = iv[ip];
        y = x[iup];

        for(;;)
        {
            if((iup - lp) < 2)
                break;
            if(x[++lp] < y)
                continue;
            x[iup] = x[lp];

            for(;;)
            {
                if(((iup--) - lp) < 2)
                    break;
                if(x[iup] >= y)
                    continue;
                x[lp] = x[iup];
                break;
            }
        }

        x[iup] = y;
        if(ip+1 >= (numalloc*Q_ALLOC))
        {
            temp=new int[(numalloc+1)*Q_ALLOC];
            memcpy(temp, lv, numalloc*Q_ALLOC*sizeof(int));
            delete[] lv;
            lv=temp;
            temp=0;

            temp=new int[(numalloc+1)*Q_ALLOC];
            memcpy(temp, iv, numalloc*Q_ALLOC*sizeof(int));
            delete[] iv;
            iv=temp;
            temp=0;

            numalloc++;
        }

        if((iup - lv[ip]) < (iv[ip] - iup))
        {
            lv[ip + 1] = lv[ip];
            iv[ip + 1] = iup - 1;
            lv[ip]     = iup + 1;
        }
        else
        {
            lv[ip + 1] = iup + 1;
            iv[ip + 1] = iv[ip];
            iv[ip] = iup - 1;
        }
        ip++;
    }
    delete[] iv;
    delete[] lv;
    //std::cout << "Data Sorted." << endl;
}
