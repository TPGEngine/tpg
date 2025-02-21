#ifndef RecursiveForecast_h
#define RecursiveForecast_h

#include <TaskEnv.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <limits>
#include <algorithm>
#include <iostream>

using namespace std;

class RecursiveForecast : public TaskEnv {
   public:
    // Data. One vec per timestep. Supports multivariate
    vector<vector<double>> data;
    // Starting points to slice datasets. One vec each for train, val, test
    vector<vector<int>> t_start;

    int n_eval_train_;
    int n_eval_val_;
    int n_prime_;
    // Number of samples (horizon) for training, validation, test
    int n_predict_[3];

    string task_;

    vector<int> uniq_discrete_univars_;

    class CSVReader {
        const string filename;
        const string delim;

       public:
        CSVReader(const string& f, const string& dlm = " ") : filename(f), delim(dlm) {}
        vector<vector<double>> ReadData() {
            ifstream file(filename);
            vector<vector<double>> data_vec;
            string line;
            while (getline(file, line)) {
                if (line.size() == 0) continue;
                vector<double> row_values;
                stringstream s(line);
                string word;
                while (getline(s, word, ',')) {
                    row_values.push_back(stod(word.c_str()));
                }
                data_vec.push_back(row_values);
            }
            file.close();
            return data_vec;
        }
    };

    RecursiveForecast(string task) {
        eval_type_ = "RecursiveForecast";
        task_ = task;
    }

    ~RecursiveForecast() {}

    int GetNumEval(int phase) {
        return static_cast<int>(t_start[phase].size());
    }

    void PrepareData(mt19937& rng) {
        // import data
        CSVReader* reader;
        if (task_ == "Sunspots")
            reader =
                new CSVReader("./datasets/SN_ms_tot_V2.0_Nov1834-June1926.csv");
        else if (task_ == "Mackey")
            reader = new CSVReader("./datasets/Mackey-1100.csv");
        else if (task_ == "Laser")
            reader = new CSVReader("./datasets/Laser-10000-1000-2100.csv");
        else if (task_ == "Audio")
            reader = new CSVReader("./datasets/2024-05-22-ali-10sec.dat");
        else if (task_ == "Offset")
            reader = new CSVReader("./datasets/ali_offset_diff.csv");
        else if (task_ == "Duration")
            reader = new CSVReader("./datasets/ali_duration.csv");
        else if (task_ == "Pitch")
            reader = new CSVReader("./datasets/ali_pitch.csv");
        else if (task_ == "PitchBach")
            reader = new CSVReader("./datasets/ali_bach_pitch.csv");
        else if (task_ == "Bach")
            reader = new CSVReader(
                "./datasets/input_bach_first_order_offset.csv", ",");
        else {
            cerr << "Unrecognised RecursiveForecast task" << endl;
            exit(1);
        }
        data = reader->ReadData();
        delete reader;

        // start steps for priming, for each phase (train, validate, test)
        t_start.resize(3);

        if (task_ == "Sunspots" || task_ == "Mackey" || task_ == "Laser") {
            // train (original, 19 start points)
            for (int s = 0; s <= 900; s += 50)
                t_start[0].push_back(s);

            // validation (original, 9 start points)
            for (int s = 50; s <= 850; s += 100)
                t_start[1].push_back(s);

            // // train (original*2, 37 start points)
            // for (int s = 0; s <= 900; s += 25) t_start[0].push_back(s);

            // // validation (original*2, 18 start points)
            // for (int s = 50; s <= 850; s += 50) t_start[1].push_back(s);

            // // Randomized train and validation slices
            // mt19937 rng_data(42);
            // uniform_int_distribution<int> dis_train(0, 900);
            // for (int s = 0; s <= n_eval_train_; s++)
            //   t_start[0].push_back(dis_train(rng_data));
            // uniform_int_distribution<int> dis_val(0, 850);
            // for (int s = 0; s <= n_eval_val_; s++)
            //   t_start[1].push_back(dis_val(rng_data));

            // test (original single start point)
            t_start[2].insert(t_start[2].begin(), {950});

        } else if (task_ == "Offset" || task_ == "Duration" ||
                   task_ == "Pitch" || task_ == "PitchBach" ||
                   task_ == "Bach") {
            uniform_int_distribution<int> DisTrain(
                0, data.size() - (n_prime_ + n_predict_[0]));
            uniform_int_distribution<int> DisVal(
                0, data.size() - (n_prime_ + n_predict_[1]));

            // Train
            for (int i = 0; i < n_eval_train_; i++)
                t_start[0].push_back(DisTrain(rng));

            // Validation
            for (int i = 0; i < n_eval_val_; i++)
                t_start[1].push_back(DisVal(rng));

            // Test
            t_start[2] = t_start[1];  // TODO(spkelly): test==val, fix
        }
        cout << "time series train slices: " << t_start[0].size()
             << endl;
        cout << "time series validation slices: "
             << t_start[1].size() << endl;
        cout << "time series test slices: " << t_start[2].size()
             << endl;
    }

    // Normalize data in each column to the range [0,1]
    void Normalize() {
        size_t n_col = data[0].size();
        for (size_t col = 0; col < n_col; col++) {
            double max_feature = numeric_limits<double>::lowest();
            double min_feature = numeric_limits<double>::max();
            // Find min/max values
            for (size_t row = 0; row < data.size(); row++) {
                max_feature = max(max_feature, data[row][col]);
                min_feature = min(min_feature, data[row][col]);
            }
            // Normalize
            for (size_t row = 0; row < data.size(); row++) {
                data[row][col] = (data[row][col] - min_feature) /
                                 (max_feature - min_feature);
            }
        }
    }
};
#endif
