#ifndef sequence_comparisons_h
#define sequence_comparisons_h

#include <bzlib.h>
#include <cmath>  // Add this line
#include <numeric>  // Add this line for std::accumulate

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

// int compressedLength(char * source){
//    ostringstream oss;
//    int blockSize100k = 9;
//    int verbosity = 0;
//    int workFactor = 30; // 0 = USE THE DEFAULT VALUE
//    unsigned int sourceLength = strlen(source);
//    unsigned int destLength = 1.01 * sourceLength + 600;
//    // Official formula, Big enough to hold output.  Will change to real size.
//    char *dest = (char*)malloc(destLength); int returnCode =
//    BZ2_bzBuffToBuffCompress( dest, &destLength, source, sourceLength,
//    blockSize100k, verbosity, workFactor );

//    if (returnCode == BZ_OK)
//    {
//       free(dest);
//       return destLength;
//    }
//    else
//    {
//       free(dest);
//       cout << " Can't get compressed length. " << "Error code:" <<
//       returnCode; return returnCode;
//    }
// }

// /******************************************************************************/
// string decompressString(std::string& data)
// {
//   namespace bio = boost::iostreams;

//   std::stringstream compressed(data);
//   std::stringstream decompressed;

//   bio::filtering_streambuf<bio::input> out;
//   out.push(bio::gzip_decompressor());
//   out.push(compressed);
//   bio::copy(out, decompressed);

//   return decompressed.str();
// }

// /******************************************************************************/
// string compressString(std::string& data)
// {
//   namespace bio = boost::iostreams;

//   std::stringstream compressed;
//   std::stringstream origin(data);

//   bio::filtering_streambuf<bio::input> out;
//   out.push(bio::gzip_compressor(bio::gzip_params(bio::gzip::best_compression)));
//   out.push(origin);
//   bio::copy(out, compressed);

//   //return compressed.str();
//   string decomp = compressed.str();
//   return decompressString(decomp);
// }

// /******************************************************************************/
// double normalizedCompressionDistance(vector<int>& v1, vector<int>& v2){
//    if (v1 == v2)
//       return 0;
//    ostringstream o;
//    o << VectorToStringNoSpace(v1);
//    int Zx = compressedLength((char*) o.str().c_str());
//    o.str("");
//    o << VectorToStringNoSpace(v2);
//    int Zy = compressedLength((char*) o.str().c_str());
//    o.str("");
//    o << VectorToStringNoSpace(v1) << VectorToStringNoSpace(v2);
//    int Zxy = compressedLength((char*) o.str().c_str());
//    o.str("");
//    int nom = Zxy-min(Zx,Zy);
//    int denom = max(Zx,Zy);
//    return ((double)nom/denom)/MAX_NCD;
// }

// /******************************************************************************/
// double normalizedCompressionDistance(string& v1, string& v2){
//    if (v1 == v2)
//       return 0;
//    int Zx = compressedLength((char*) v1.c_str());
//    int Zy = compressedLength((char*) v2.c_str());
//    int Zxy = compressedLength((char*) (v1+v2).c_str());
//    int nom = Zxy-min(Zx,Zy);
//    int denom = max(Zx,Zy);
//    return ((double)nom/denom)/MAX_NCD;
// }

#endif