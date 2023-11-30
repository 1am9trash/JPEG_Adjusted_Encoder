#include <iostream>
#include <iomanip>

#include "jpeg.hpp"

long long get_file_size(std::string filename) {
    std::ifstream file(filename, std::ifstream::binary);
    long long len;

    file.seekg(0, std::ios::end);
    len = file.tellg();

    file.close();

    return len;
}

int main() {
    std::vector<std::string> filenames {
        "small",
        "red",
        "green",
        "blue",
        "test_1",
        "test_2",
        "origin_field"
    };

    for (int i = 0; i < filenames.size(); i++) {
        std::string in_folder = "sample/input_ppm/";
        std::string out_folder = "sample/output_jpg/";
        std::string in_format = ".ppm";
        std::string out_format = ".jpg";

        std::string in_file = in_folder + filenames[i] + in_format;
        std::string out_file_normal = out_folder + filenames[i] + "_normal" + out_format;
        std::string out_file_DHT = out_folder + filenames[i] + "_DHT" + out_format;
        std::string out_file_DQT = out_folder + filenames[i] + "_DQT" + out_format;

        std::cout << "Process " << filenames[i] << " case.\n";

        convert_normal_jpeg(in_file, out_file_normal);
        convert_adjusted_DQT_jpeg(in_file, out_file_DQT, 1.0);
        convert_adjusted_DHT_jpeg(in_file, out_file_DHT);

        int in_size = get_file_size(in_file);
        int out_size_normal = get_file_size(out_file_normal);
        int out_size_DHT = get_file_size(out_file_DHT);
        int out_size_DQT = get_file_size(out_file_DQT);

        std::cout << "       input ppm size: " << std::setw(10) << in_size << " bytes.\n";
        std::cout << "      normal jpg size: " << std::setw(10) << out_size_normal << " bytes.";
        std::cout << " (Rate: " << std::fixed << std::setprecision(2) << (100.0 * out_size_normal / in_size)  << "%)\n";
        std::cout << "adjusted DQT jpg size: " << std::setw(10) << out_size_DQT << " bytes.";
        std::cout << " (Rate: " << std::fixed << std::setprecision(2) << (100.0 * out_size_DQT / in_size)  << "%)\n";
        std::cout << "adjusted DHT jpg size: " << std::setw(10) << out_size_DHT << " bytes.";
        std::cout << " (Rate: " << std::fixed << std::setprecision(2) << (100.0 * out_size_DHT / in_size)  << "%)\n";

        std::cout << "\n";
    }

    return 0;
}