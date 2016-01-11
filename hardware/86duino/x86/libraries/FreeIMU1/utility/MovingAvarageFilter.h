/*
https://github.com/sebnil/Moving-Avarage-Filter--Arduino-Library-
*/
#ifndef MovingAvarageFilter_h
#define MovingAvarageFilter_h

#define MAX_DATA_POINTS 20

class MovingAvarageFilter {
public:
  //construct without coefs
  MovingAvarageFilter(unsigned int newDataPointsCount);

  double process(double in);

private:
  double values[MAX_DATA_POINTS];
  int k; // k stores the index of the current array read to create a circular memory through the array
  int dataPointsCount;
  double out;
  int i; // just a loop counter
};
#endif

