#ifndef sequence_comparisons_h
#define sequence_comparisons_h

#include <bzlib.h>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/math/statistics/bivariate_statistics.hpp>
#include <vector>

/******************************************************************************/
double MeanSquaredError(std::vector<double>& targets,
                        std::vector<double>& predictions);

/******************************************************************************/
double MeanAbsoluteError(const std::vector<double>& targets,
                         const std::vector<double>& predictions);

/******************************************************************************/
double PearsonCorrelation(std::vector<double>& targets,
                          std::vector<double>& predictions);

/******************************************************************************/
double EuclideanDistSqrd(double* x, double* y, int dim);

/******************************************************************************/
double EuclideanDistSqrd(std::vector<double>& x, std::vector<double>& y);

/******************************************************************************/
double EuclideanDistSqrdNorm(std::vector<double>& x, std::vector<double>& y);

/******************************************************************************/
double EuclideanDist(std::vector<double>& x, std::vector<double>& y);

/******************************************************************************/
int hammingDist(std::vector<int>& x, std::vector<int>& y);

/******************************************************************************/
double TheilsStatistic(const std::vector<double>& targets,
                       const std::vector<double>& predictions);

/******************************************************************************/
double Correlation(const std::vector<double>& targets,
                   const std::vector<double>& predictions);

double calculateTheils_Multi(const std::vector<double>& targets,
                             const std::vector<double>& predictions);

/******************************************************************************/
double calculateMSE_Multi(const std::vector<double>& targets,
                          const std::vector<double>& predictions);

/******************************************************************************/
double calculatePearson_Multi(const std::vector<double>& targets,
                              const std::vector<double>& predictions);

/******************************************************************************/

#endif