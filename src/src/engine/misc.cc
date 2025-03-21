#include "misc.h"

/******************************************************************************/

void die(const char* file, const char* func, const int line, const char* msg) {
   cerr << "error in file " << string(file) << " function " << string(func);
   cerr << " line " << line << ": " << string(msg) << "... exiting" << endl;
   abort();
}

/******************************************************************************/
bool isEqual(vector<int>& x, vector<int>& y) {
   if (x.size() != y.size())
      return false;

   vector<int>::iterator xiter, yiter, enditer;

   for (xiter = x.begin(), yiter = y.begin(), enditer = x.end();
        xiter != enditer; xiter++, yiter++)
      if (*xiter != *yiter)
         return false;

   return true;
}

/******************************************************************************/
bool isEqual(vector<double>& x, vector<double>& y, double e) {
   if (x.size() != y.size())
      return false;

   vector<double>::iterator xiter, yiter, enditer;

   for (xiter = x.begin(), yiter = y.begin(), enditer = x.end();
        xiter != enditer; xiter++, yiter++)
      if (isEqual(*xiter, *yiter, e) == false)
         return false;

   return true;
}

/******************************************************************************/
template <class T>
bool isEqual(vector<T>& x, vector<T>& y, double e) {
   if (x.size() != y.size())
      return false;

   typename vector<T>::iterator xiter, yiter, enditer;

   for (xiter = x.begin(), yiter = y.begin(), enditer = x.end();
        xiter != enditer; xiter++, yiter++)
      if (isEqual(*xiter, *yiter, e) == false)
         return false;

   return true;
}

/******************************************************************************/
int readMap(string fileName, map<string, string>& args) {
   ostringstream o;
   o << "cannot open map file: " << fileName;
   int pairs = 0;

   ifstream infile(fileName.c_str(), ios::in);

   if (!infile)
      die(__FILE__, __FUNCTION__, __LINE__, o.str().c_str());

   do {
      string key, value;

      if (infile)
         infile >> key;
      else
         break;
      if (infile)
         infile >> value;
      else
         break;

      args.insert(map<string, string>::value_type(key, value));
      pairs++;

   } while (true);

   infile.close();

   return pairs;
}

/******************************************************************************/
int stringToInt(string s) {
   istringstream buffer(s);

   int i;

   buffer >> i;

   return i;
}

/******************************************************************************/
long stringToLong(string s) {
   istringstream buffer(s);

   long l;

   buffer >> l;

   return l;
}

/******************************************************************************/
double stringToDouble(string s) {
   istringstream buffer(s);

   double d;

   buffer >> d;

   return d;
}

/******************************************************************************/
double stdDev(vector<double> vec) {
   double sum = std::accumulate(vec.begin(), vec.end(), 0.0);
   double mean = sum / vec.size();
   vector<double> diff(vec.size());
   transform(vec.begin(), vec.end(), diff.begin(),
             bind2nd(std::minus<double>(), mean));
   double sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
   double stdev = sqrt(sq_sum / (vec.size() - 1));
   return stdev;
}

/******************************************************************************/
vector<string>& SplitString(const string& s, char delim,
                            vector<string>& elems) {
   elems.clear();
   stringstream ss(s);
   string item;
   while (getline(ss, item, delim)) {
      elems.push_back(item);
   }
   return elems;
}

/******************************************************************************/
vector<string> SplitString(const string& s, char delim) {
   vector<string> elems;
   SplitString(s, delim, elems);
   return elems;
}

/******************************************************************************/
// TODO(skelly): ChatGPT code:
std::string ExpandEnvVars(const std::string& str) {
   std::string result;
   size_t pos = 0;

   while (pos < str.length()) {
      if (str[pos] == '$') {
         size_t start = pos + 1;
         size_t end = start;

         // Handle ${VAR} format
         if (start < str.length() && str[start] == '{') {
            end = str.find('}', start);
            if (end != std::string::npos) {
               std::string varName = str.substr(start + 1, end - start - 1);
               const char* varValue = getenv(varName.c_str());
               if (varValue) {
                  result += varValue;
               }
               pos = end + 1;
               continue;
            }
         }

         // Handle $VAR format
         while (end < str.length() && (isalnum(str[end]) || str[end] == '_')) {
            ++end;
         }
         std::string varName = str.substr(start, end - start);
         const char* varValue = getenv(varName.c_str());
         if (varValue) {
            result += varValue;
         }
         pos = end;
      } else {
         result += str[pos];
         ++pos;
      }
   }
   return result;
}
