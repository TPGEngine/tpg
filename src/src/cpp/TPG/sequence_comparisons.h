#ifndef sequence_comparisons_h
#define sequence_comparisons_h

#include <bzlib.h>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/math/statistics/bivariate_statistics.hpp>

/******************************************************************************/
double MeanSquaredError(std::vector<double> &targets,
                        std::vector<double> &predictions) {
  double err = 0;
  for (size_t i = 0; i < targets.size(); i++) {
    err += pow(targets[i] - predictions[i], 2);
  }
  return err / targets.size();
}

/******************************************************************************/
double MeanAbsoluteError(const std::vector<double> &targets,
                         const std::vector<double> &predictions) {
  double err = 0;
  for (size_t i = 0; i < targets.size(); i++) {
    err += abs(targets[i] - predictions[i]);
  }
  return err / targets.size();
}

/******************************************************************************/
double PearsonCorrelation(std::vector<double> &targets,
                          std::vector<double> &predictions) {
  double cc =
      boost::math::statistics::correlation_coefficient(targets, predictions);
  return std::isfinite(cc) && cc >= -1.0 && cc <= 1.0 ? cc : -1.0;
}

/******************************************************************************/
double EuclideanDistSqrd(double *x, double *y, int dim) {
  double dist = 0;
  for (int i = 0; i < dim; i++) dist += (x[i] - y[i]) * (x[i] - y[i]);
  return dist;
}

/******************************************************************************/
double EuclideanDistSqrd(vector<double> &x, vector<double> &y) {
  double dist = 0;
  vector<double>::iterator xiter, yiter, enditer;
  for (xiter = x.begin(), yiter = y.begin(), enditer = x.end();
       xiter != enditer; xiter++, yiter++)
    dist += (*xiter - *yiter) * (*xiter - *yiter);
  return dist;
}

/******************************************************************************/
double EuclideanDistSqrdNorm(vector<double> &x, vector<double> &y) {
  double dist = 0;
  int numFeatures = 0;
  vector<double>::iterator xiter, yiter, enditer;
  for (xiter = x.begin(), yiter = y.begin(), enditer = x.end();
       xiter != enditer; xiter++, yiter++) {
    dist += (*xiter - *yiter) * (*xiter - *yiter);
    numFeatures++;
  }
  return dist / numFeatures;
}

/******************************************************************************/
double EuclideanDist(vector<double> &x, vector<double> &y) {
  double dist = 0;
  vector<double>::iterator xiter, yiter, enditer;
  for (xiter = x.begin(), yiter = y.begin(), enditer = x.end();
       xiter != enditer; xiter++, yiter++)
    dist += (*xiter - *yiter) * (*xiter - *yiter);

  return (double)sqrt(dist);
}

/******************************************************************************/
int hammingDist(vector<int> &x, vector<int> &y) {
  int dist = 0;
  vector<int>::iterator xiter, yiter, enditer;
  for (xiter = x.begin(), yiter = y.begin(), enditer = x.end();
       xiter != enditer; xiter++, yiter++)
    dist += *xiter == *yiter ? 0 : 1;
  return dist;
}

/******************************************************************************/
double TheilsStatistic(const std::vector<double> &targets,
                       const std::vector<double> &predictions) {
  double epsilon = 1e-10;
  double numerator = 0;
  double denominator = 0;
  for (size_t i = 0; i < predictions.size() - 1; i++) {
    numerator += pow(predictions[i] - targets[i], 2);
    denominator += pow(targets[i] - targets[i + 1], 2);
  }

  denominator = (denominator == 0) ? epsilon : denominator;

  return numerator / denominator;
}

/******************************************************************************/
double Correlation(const std::vector<double> &targets,
                   const std::vector<double> &predictions) {
  double epsilon = 1e-10;

  double mean_targets =
      accumulate(targets.begin(), targets.end(), 0.0) / targets.size();
  double mean_predictions =
      accumulate(predictions.begin(), predictions.end(), 0.0) /
      predictions.size();

  double numerator = 0;
  double denominator_1 = 0;
  double denominator_2 = 0;

  for (size_t i = 0; i < targets.size(); i++) {
    numerator +=
        (targets[i] - mean_targets) * (predictions[i] - mean_predictions);
    denominator_1 += pow(targets[i] - mean_targets, 2);
    denominator_2 += pow(predictions[i] - mean_predictions, 2);
  }

  double denominator = sqrt(denominator_1 * denominator_2);
  denominator = (denominator == 0) ? epsilon : denominator;

  return numerator / denominator;
}

double calculateTheils_Multi(const std::vector<double> &targets,
                             const std::vector<double> &predictions) {
  int8_t num_variables = 3;
  size_t num_elements = targets.size() / num_variables;

  double numerator_offset = 0, denominator_offset = 0;
  double numerator_duration = 0, denominator_duration = 0;
  double numerator_pitch = 0, denominator_pitch = 0;

  double epsilon = 1e-10;

  for (size_t i = 0; i < num_elements - 1; i++) {
    numerator_offset +=
        pow(predictions[i * num_variables] - targets[i * num_variables], 2);
    denominator_offset +=
        pow(targets[i * num_variables] - targets[(i + 1) * num_variables], 2);

    numerator_duration += pow(
        predictions[i * num_variables + 1] - targets[i * num_variables + 1], 2);
    denominator_duration += pow(
        targets[i * num_variables + 1] - targets[(i + 1) * num_variables + 1],
        2);

    numerator_pitch += pow(
        predictions[i * num_variables + 2] - targets[i * num_variables + 2], 2);
    denominator_pitch += pow(
        targets[i * num_variables + 2] - targets[(i + 1) * num_variables + 2],
        2);
  }

  denominator_offset = (denominator_offset == 0) ? epsilon : denominator_offset;
  denominator_duration =
      (denominator_duration == 0) ? epsilon : denominator_duration;
  denominator_pitch = (denominator_pitch == 0) ? epsilon : denominator_pitch;

  double theils_u1 = numerator_offset / denominator_offset;
  double theils_u2 = numerator_duration / denominator_duration;
  double theils_u3 = numerator_pitch / denominator_pitch;
  return theils_u1 + theils_u2 + theils_u3;
}

/******************************************************************************/
double calculateMSE_Multi(const std::vector<double> &targets,
                          const std::vector<double> &predictions) {
  int8_t num_variables = 3;
  size_t num_elements = targets.size() / num_variables;

  double mse_offset = 0;
  double mse_duration = 0;
  double mse_pitch = 0;

  for (size_t i = 0; i < num_elements; i++) {
    mse_offset +=
        pow(predictions[i * num_variables] - targets[i * num_variables], 2);
    mse_duration += pow(
        predictions[i * num_variables + 1] - targets[i * num_variables + 1], 2);
    mse_pitch += pow(
        predictions[i * num_variables + 2] - targets[i * num_variables + 2], 2);
  }

  mse_offset /= num_elements;
  mse_duration /= num_elements;
  mse_pitch /= num_elements;

  return mse_offset + mse_duration +
         mse_pitch;  // Multiply by weights if needed    
}

/******************************************************************************/
double calculatePearson_Multi(const std::vector<double> &targets,
                              const std::vector<double> &predictions) {
  int8_t num_variables = 3;
  size_t num_elements = targets.size() / num_variables;

  vector<vector<double>> x(num_variables);
  vector<vector<double>> y(num_variables);
  for (auto &v : x) v.resize(num_elements);
  for (auto &v : y) v.resize(num_elements);

  for (size_t i = 0; i < num_elements; i++) {
    for (int j = 0; j < num_variables; j++) {
      x[j][i] = targets[i * num_variables + j];
      y[j][i] = predictions[i * num_variables + j];
    }
  }

  double corr = 0;
  for (int j = 0; j < num_variables; j++) {
    corr += boost::math::statistics::correlation_coefficient(x[j], y[j]);
  }
  return corr;
}

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
//    o << vecToStrNoSpace(v1);
//    int Zx = compressedLength((char*) o.str().c_str());
//    o.str("");
//    o << vecToStrNoSpace(v2);
//    int Zy = compressedLength((char*) o.str().c_str());
//    o.str("");
//    o << vecToStrNoSpace(v1) << vecToStrNoSpace(v2);
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