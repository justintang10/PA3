/**
 * @file stats.cpp
 * @description implementation of Stats class used for fast computation of
 *              colour information from rectangular image regions
 *              CPSC 221 2023S PA3
 *
 *              SUBMIT THIS FILE WITH YOUR MODIFICATIONS
 */

#include "stats.h"
#include <algorithm>
#include <cmath>

#define NUMBINS 36 // number of histogram bins

Stats::Stats(const PNG& im){
    // complete your implementation below
    hist.resize(im.width(), vector<vector<unsigned int>>(im.height(), vector<unsigned int>(NUMBINS,0)));
    sumHueX.resize(im.width(), vector<double>(im.height()));
    sumHueY.resize(im.width(), vector<double>(im.height()));
    sumSat.resize(im.width(), vector<double>(im.height()));
    sumLum.resize(im.width(), vector<double>(im.height()));
    for (unsigned int y = 0; y < im.height(); y++)
    {
        for (unsigned int x = 0; x < im.width(); x++)
        {
            HSLAPixel *pixel = im.getPixel(x, y);
            if (x == 0 && y == 0)
            {
                sumHueX[x][y] = cos(PI * pixel->h / 180.0);
                sumHueY[x][y] = sin(PI * pixel->h / 180.0);
                sumSat[x][y] = pixel->s;
                sumLum[x][y] = pixel->l;

            }
            if (x != 0 && y == 0)
            {
                sumHueX[x][y] = sumHueX[x - 1][y] + cos(PI * pixel->h / 180.0);
                sumHueY[x][y] = sumHueY[x - 1][y] + sin(PI * pixel->h / 180.0);
                sumSat[x][y] = sumSat[x - 1][y] + pixel->s;
                sumLum[x][y] = sumLum[x - 1][y] + pixel->l;
            }
            if (y != 0 && x == 0)
            {
                sumHueX[x][y] = sumHueX[x][y - 1] + cos(PI * pixel->h / 180.0);
                sumHueY[x][y] = sumHueY[x][y - 1] + sin(PI * pixel->h / 180.0);
                sumSat[x][y] = sumSat[x][y - 1] + pixel->s;
                sumLum[x][y] = sumLum[x][y - 1] + pixel->l;
            }
            if (x != 0 && y != 0)
            {
                sumHueX[x][y] = sumHueX[x - 1][y] + sumHueX[x][y - 1] - sumHueX[x - 1][y - 1] + cos(PI * pixel->h / 180.0);
                sumHueY[x][y] = sumHueY[x - 1][y] + sumHueY[x][y - 1] - sumHueY[x - 1][y - 1] + sin(PI * pixel->h / 180.0);
                sumSat[x][y] = sumSat[x - 1][y] - sumSat[x - 1][y - 1] + sumSat[x][y - 1] + pixel->s;
                sumLum[x][y] = sumLum[x - 1][y] + sumLum[x][y - 1] - sumLum[x - 1][y - 1] + pixel->l;
            }

            int bin = floor(pixel->h / 10);

            for (int k = 0; k < 36; k++)
            {
                if (x == 0 && y == 0)
                {
                    hist[x][y][k] = 0;
                }
                if (x != 0 && y == 0)
                {
                    hist[x][y][k] = hist[x - 1][y][k];
                }
                else if (y != 0 && x == 0)
                {
                    //follow the top pixel
                    hist[x][y][k] = hist[x][y - 1][k];
                }
                else if (y != 0 && x != 0)
                {
                    hist[x][y][k] = hist[x - 1][y][k] + hist[x][y - 1][k] - hist[x - 1][y - 1][k];
                }
                if (k == bin)
                {
                    hist[x][y][k]++;
                }
            }
        }
    }
}

long Stats::RectArea(pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr){
    // replace the stub below with your completed implementation
     long recArea = (lr.first - ul.first + 1) * (lr.second - ul.second + 1);
    return recArea;
}

HSLAPixel Stats::GetAvg(pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr){
    // replace the stub below with your completed implementation
    int ul_x = ul.first;
    int ul_y = ul.second;
    int lr_x = lr.first;
    int lr_y = lr.second;
    double area = RectArea(ul, lr);
    // std::cout << "areaa " << area << endl;
    double totalHueX;
    double totalHueY;

    HSLAPixel pixel;
    if (ul_x == lr_x && ul_y == lr_y) area = 1;
    if (ul_x == 0 && ul_y == 0)
    {
        totalHueX = (sumHueX[lr_x][lr_y]);
        totalHueY = (sumHueY[lr_x][lr_y]);
        pixel.s = (sumSat[lr_x][lr_y]) / area;
        pixel.l = (sumLum[lr_x][lr_y] / area);
        //std::cout << sumLum[lr_x][lr_y] << endl;
    }
    if (ul_x > 0 && ul_y > 0)
    {   
        totalHueX = (sumHueX[lr_x][lr_y] - sumHueX[ul_x - 1][lr_y] - sumHueX[lr_x][ul_y - 1] + sumHueX[ul_x - 1][ul_y - 1]);
        totalHueY = (sumHueY[lr_x][lr_y] - sumHueY[ul_x - 1][lr_y] - sumHueY[lr_x][ul_y - 1] + sumHueY[ul_x - 1][ul_y - 1]);
        pixel.s = ((sumSat[lr_x][lr_y] - sumSat[ul_x - 1][lr_y] - sumSat[lr_x][ul_y - 1] + sumSat[ul_x - 1][ul_y - 1]) / area);
        pixel.l = ((sumLum[lr_x][lr_y] - sumLum[ul_x - 1][lr_y] - sumLum[lr_x][ul_y - 1] + sumLum[ul_x - 1][ul_y - 1]) / area);
        // std::cout <<  sumLum[lr_x][lr_y] << "and" << sumLum[ul_x][lr_y]<< "and" << sumLum[lr_x][ul_y-1] << endl;
    }
    else if(ul_x == 0 && ul_y !=0) {
        totalHueX = sumHueX[lr_x][lr_y] - sumHueX[lr_x][ul_y -1];
        totalHueY = sumHueY[lr_x][lr_y] - sumHueY[lr_x][ul_y -1];
        pixel.s = ((sumSat[lr_x][lr_y] - sumSat[lr_x][ul_y -1]) / area);
        pixel.l = ((sumLum[lr_x][lr_y] - sumLum[lr_x][ul_y -1]) / area);
    }
    else if(ul_x != 0 && ul_y ==0) {
        totalHueX = sumHueX[lr_x][lr_y] - sumHueX[ul_x -1][lr_y];
        totalHueY = sumHueY[lr_x][lr_y] - sumHueY[ul_x -1][lr_y];
        pixel.s = ((sumSat[lr_x][lr_y] - sumSat[ul_x -1][lr_y]) / area);
        pixel.l = ((sumLum[lr_x][lr_y] - sumLum[ul_x -1][lr_y]) / area);
    }
    pixel.a = 1.0;
    double avgH;
    avgH = atan2(totalHueY, totalHueX) * (180.0 / PI);
    if (avgH < 0)
    {
        avgH += 360;
    }
    avgH = fmod(avgH, 360.0);
    // std::cout << "avgHue " << pixel.h  << endl;
    pixel.h = avgH;
    return pixel;
}

double Stats::Entropy(pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr){
  
  vector<int> distribution(NUMBINS);
  
  double entropy = 0;
  long area = RectArea(ul,lr);

    /**
    *  Complete your implementation below.
    *  Using private member hist, assemble the distribution over the
    *  given rectangle defined by points ul, and lr into variable distribution.
    *  You will use distribution to compute the entropy over the rectangle.
    *  if any bin in the distribution has frequency 0, then do not add that 
    *  term to the entropy total. see .h file for more details.
    */

    /* your code should include the following lines (or something similar):
        if (distribution[i] > 0 ) 
            entropy += ((double) distribution[i]/(double) area) 
                                    * log2((double) distribution[i]/(double) area);
    */
     // get the distribution from the hist
    int ulx = ul.first; int uly = ul.second;
    int lrx = lr.first; int lry = lr.second;

    if (ulx == 0 && uly == 0) { // starts at (0,0)
        for (int k = 0; k < NUMBINS; k++) {
            distribution[k] = hist[lrx][lry][k];
        }
    } else if (ulx != 0 && uly == 0) { // starts at (i,0)
        for (int k = 0; k < NUMBINS; k++) {
            distribution[k] = hist[lrx][lry][k] - hist[ulx-1][lry][k];
        }
    } else if (ulx == 0 && uly != 0) { // starts at (0,i)
        for (int k = 0; k < NUMBINS; k++) {
            distribution[k] = hist[lrx][lry][k] - hist[lrx][uly-1][k];
        }
    } else { // starts at (i,j)
        for (int k = 0; k < NUMBINS; k++) {
            distribution[k] = hist[lrx][lry][k] - hist[ulx-1][lry][k] - hist[lrx][uly-1][k] + hist[ulx-1][uly-1][k];
        }
    }

    /* your code should include the following lines (or something similar):
        if (distribution[i] > 0 ) 
            entropy += ((double) distribution[i]/(double) area) 
                                    * log2((double) distribution[i]/(double) area);
    */
    
    for (int i = 0; i < NUMBINS; i++) {
        if (distribution[i] > 0 ) 
            entropy += ((double) distribution[i]/(double) area) 
                        * log2((double) distribution[i]/(double) area);
    }
    
    return  -1 * entropy;
}

/**
 * Add your private Stats function implementations below
 */
