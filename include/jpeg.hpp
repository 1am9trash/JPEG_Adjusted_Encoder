#pragma once

#include <vector>
#include <string>
#include <map>
#include <fstream>

// PPM
struct PPM {
    std::string version;
    int width;
    int height;
    int max_value;
    unsigned char *data;
};

struct RGB {
    int r;
    int g;
    int b;
};

void remove_PPM_comment(std::ifstream &file);
PPM load_PPM(std::string &filename);
std::vector<std::vector<RGB>> PPM_data_to_vector(PPM &image);

// RGB to YCbCr
template <typename T> 
struct YCbCr {
    T y;
    T cb;
    T cr;
};

typedef YCbCr<int> iYCbCr;
typedef YCbCr<double> dYCbCr;

std::vector<std::vector<dYCbCr>> RGB_to_YCbCr(std::vector<std::vector<RGB>> &RGB_data);

// JPEG constant
// Quantization table
extern std::vector<int> quan_lum;
extern std::vector<int> quan_chrom;

// DC, AC Huffman table
extern std::vector<int> huffman_lum_ac;
extern std::vector<int> huffman_lum_dc;
extern std::vector<int> huffman_chrom_ac;
extern std::vector<int> huffman_chrom_dc;

// process image with JPEG standard
int around(double value);
std::vector<iYCbCr> do_2d_DCT(std::vector<std::vector<dYCbCr>> &YCbCr_data, int row, int col, int block);
std::vector<int> get_adjusted_quantize_table(std::vector<std::vector<int>> &data, float scale, int use_lum);
std::vector<iYCbCr> quantize(std::vector<iYCbCr> block_data, std::vector<int> &quan_lum, std::vector<int> &quan_chrom);
std::vector<std::vector<int>> get_zigzag_order(int block);
std::vector<iYCbCr> zigzag(std::vector<iYCbCr> block_data);
std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> get_statistics_before_quantize(std::vector<std::vector<dYCbCr>> &YCbCr_data);
std::vector<std::vector<iYCbCr>> do_partition_process(std::vector<std::vector<dYCbCr>> &YCbCr_data, std::vector<int> &quan_lum, std::vector<int> &quan_chrom);

// bit vector
struct BitVector {
    std::vector<unsigned char> data = {0};
    int space = 7;

    void add_bit(unsigned char b);
    void add_bits(int value, int length);
    void print_binary();
    void write_binary(std::ofstream &file);
};

// encoding to binary format
struct HuffmanInfo {
    int n_bits;
    int code;
};

int get_VLI(int value);
void to_binary_str(int code, int n_bits, BitVector &in);
std::map<int, HuffmanInfo> preprocess_DHT(const std::vector<int> &table);

void write_SOI_section(std::ofstream &file);
void write_SOF0_section(std::ofstream &file, int height, int width);
void write_DQT_section(std::ofstream &file, int num, const std::vector<int> &table);
void write_huffman_section(std::ofstream &file, int num, const std::vector<int> &table);
void write_SOS_section(std::ofstream &file);
void write_data_section(
    std::ofstream &file, std::vector<std::vector<iYCbCr>> &blocks_data,
    int get_statistics,
    void *huffman_lum_ac,
    void *huffman_lum_dc,
    void *huffman_chrom_ac,
    void *huffman_chrom_dc
);
void write_EOI_section(std::ofstream &file);

void write_jpeg(
    std::string &filename, int height, int width, std::vector<std::vector<iYCbCr>> &blocks_data,
    std::vector<int> &quan_lum,
    std::vector<int> &quan_chrom,
    std::vector<int> &huffman_lum_ac,
    std::vector<int> &huffman_lum_dc,
    std::vector<int> &huffman_chrom_ac,
    std::vector<int> &huffman_chrom_dc
);

// convert
void convert_normal_jpeg(std::string &in_filename, std::string &out_filename);
void convert_adjusted_DHT_jpeg(std::string &in_filename, std::string &out_filename);
void convert_adjusted_DQT_jpeg(std::string &in_filename, std::string &out_filename, float scale);