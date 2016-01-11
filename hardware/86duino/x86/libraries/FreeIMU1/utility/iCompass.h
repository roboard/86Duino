#ifndef iCompass_h
#define iCompass_h

#include <Arduino.h> // for byte data type
#include "RunningAverage.h"

class iCompass
{
  public:
    iCompass(void);
    iCompass(double dAngle);
    iCompass(double dAngle, unsigned int windSize);
    iCompass(double dAngle, unsigned int windSize, unsigned int maxS);

    //double heading(void);
    double iheading(int ix, int iy, int iz, double ax, double ay, double az, double mx, double my, double mz);
    
  private:
    template <typename T> struct vector
    {
      T x, y, z;
    };

    vector<double> a; // accelerometer readings
    vector<double> m; // magnetometer readings
  
    // vector functions
    template <typename Ta, typename Tb, typename To> static void vector_cross(const vector<Ta> *a, const vector<Tb> *b, vector<To> *out);
    template <typename Ta, typename Tb> static double vector_dot(const vector<Ta> *a,const vector<Tb> *b);
    static void vector_normalize(vector<double> *a);
    
    double oldHeading;
    RunningAverage myRA;
    unsigned int maxSamples;
    unsigned int samples;
    double declinationAngle;
    
    //=============================================================
    // converted from Michael Shimniok Data Bus code
    // http://mbed.org/users/shimniok/code/AVC_20110423/

    double clamp360(double x) {
        while ((x) >= 360.0) (x) -= 360.0; 
        while ((x) < 0) (x) += 360.0; 
        return x;
    }

    //==============================================================
    //
    double HeadingAvgCorr(double newx, double oldx) {
        while ((newx + 180.0) < oldx) (newx) += 360.0;
        while ((newx - 180.0) > oldx) (newx) -= 360.0;
        while ((newx) == 360.0) (newx) = 0.0;
        return newx;
    }


    //=======================================
    double iround(double number, double decimal) {
      int ix;
      ix = round(number*pow(10, decimal));
      return double(ix)/pow(10, decimal);
    }

    //=======================================
};
#endif



