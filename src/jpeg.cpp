#include <cstdio>
#include <cmath>
#include <iostream>
#include <cassert>

#include "huffman.hpp"
#include "jpeg.hpp"

// PPM
void remove_PPM_comment(std::ifstream &file) {
    static char buf[1024];

    char c;
    while (c = file.peek()) {
        if (c == '\n' || c == '\r') {
            file.get();
        } else if (c == '#') {
            file.getline(buf, 1023);
        } else {
            break;
        }
    }
}

PPM load_PPM(std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    PPM image;
    int size;

    remove_PPM_comment(file);
    file >> image.version;
    remove_PPM_comment(file);
    file >> image.width >> image.height;
    remove_PPM_comment(file);
    file >> image.max_value;

    size = image.width * image.height * 3;
    image.data = new unsigned char[size];

    remove_PPM_comment(file);
    file.read((char *)image.data, size);

    file.close();

    return image;
}

std::vector<std::vector<RGB>> PPM_data_to_vector(PPM &image) {
    std::vector<std::vector<RGB>> RGB_data(std::vector(image.height, std::vector(image.width, RGB {0, 0, 0})));

    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
            unsigned char *offset = image.data + i * image.width * 3 + j * 3;
            RGB_data[i][j] = RGB {*offset, *(offset + 1), *(offset + 2)};
        }
    }

    return RGB_data;
}

// RGB to YCbCr
std::vector<std::vector<dYCbCr>> RGB_to_YCbCr(std::vector<std::vector<RGB>> &RGB_data) {
    std::vector<std::vector<dYCbCr>> YCbCr_data(std::vector(RGB_data.size(), std::vector(RGB_data[0].size(), dYCbCr {0.0, 0.0, 0.0})));

    for (int i = 0; i < RGB_data.size(); i++) {
        for (int j = 0; j < RGB_data[0].size(); j++) {
            YCbCr_data[i][j].y = 0.257 * RGB_data[i][j].r + 0.564 * RGB_data[i][j].g + 0.098 * RGB_data[i][j].b + 16.0;
            YCbCr_data[i][j].cb = -0.148 * RGB_data[i][j].r - 0.291 * RGB_data[i][j].g + 0.439 * RGB_data[i][j].b + 128.0;
            YCbCr_data[i][j].cr = 0.439 * RGB_data[i][j].r - 0.368 * RGB_data[i][j].g - 0.071 * RGB_data[i][j].b + 128.0;
        }
    }

    return YCbCr_data;
}

// JPEG constant
// Quantization table
std::vector<int> quan_lum = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 36, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
}; 

std::vector<int> quan_chrom = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

// DC, AC Huffman table
std::vector<int> huffman_lum_ac = {
    0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

std::vector<int> huffman_lum_dc = {
    0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b
};

std::vector<int> huffman_chrom_ac = {
    0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

std::vector<int> huffman_chrom_dc = {
    0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b
};

// process image with JPEG standard
int around(double value) {
    return value >= 0.0 ? int(value + 0.5) : int (value - 0.5);
}

std::vector<iYCbCr> do_2d_DCT(std::vector<std::vector<dYCbCr>> &YCbCr_data, int row, int col, int block) {
    std::vector<iYCbCr> block_DCT_data(block * block, iYCbCr {0, 0, 0});
    std::vector<dYCbCr> tmp_block_DCT_data(block * block, dYCbCr {0.0, 0.0, 0.0});

    for (int m = 0; m < block; m++) {
        for (int l = 0; l < block; l++) {
            double alpha = l == 0 ? std::sqrt(1.0 / block) : std::sqrt(2.0 / block);
            dYCbCr tmp = { 0.0, 0.0, 0.0 };

            for (int n = 0; n < block; n++) {
                tmp.y += (YCbCr_data[m + row][n + col].y - 128.0) * std::cos((2.0 * n + 1.0) * l * M_PI / (2.0 * block));
                tmp.cb += (YCbCr_data[m + row][n + col].cb - 128.0) * std::cos((2.0 * n + 1.0) * l * M_PI / (2.0 * block));
                tmp.cr += (YCbCr_data[m + row][n + col].cr - 128.0) * std::cos((2.0 * n + 1.0) * l * M_PI / (2.0 * block));
            }

            tmp_block_DCT_data[m * block + l].y = alpha * tmp.y;
            tmp_block_DCT_data[m * block + l].cb = alpha * tmp.cb;
            tmp_block_DCT_data[m * block + l].cr = alpha * tmp.cr;
        }
    }

    for (int l = 0; l < block; l++) {
        for (int k = 0; k < block; k++) {
            double beta = k == 0 ? std::sqrt(1.0 / block) : std::sqrt(2.0 / block);
            dYCbCr tmp = { 0.0, 0.0, 0.0 };

            for (int m = 0; m < block; m++) {
                tmp.y += tmp_block_DCT_data[m * block + l].y * std::cos((2.0 * m + 1.0) * k * M_PI / (2.0 * block));
                tmp.cb += tmp_block_DCT_data[m * block + l].cb * std::cos((2.0 * m + 1.0) * k * M_PI / (2.0 * block));
                tmp.cr += tmp_block_DCT_data[m * block + l].cr * std::cos((2.0 * m + 1.0) * k * M_PI / (2.0 * block));
            }

            block_DCT_data[k * block + l].y = around(beta * tmp.y);
            block_DCT_data[k * block + l].cb = around(beta * tmp.cb);
            block_DCT_data[k * block + l].cr = around(beta * tmp.cr);
        }
    }

    return block_DCT_data;
}

std::vector<int> get_adjusted_quantize_table(std::vector<std::vector<int>> &data, float scale, int use_lum) {
    static const std::vector<float> standard_error_lum = {
        7.5, 3.487, 2.801, 3.9, 5.082, 5.796, 6.218, 4.759,
        4.158, 3.454, 3.635, 4.306, 4.994, 7.525, 6.061, 4.77,
        3.926, 3.556, 4.133, 5.433, 6.355, 7.268, 6.096, 4.643,
        3.757, 4.334, 5.244, 6.218, 7.975, 7.534, 5.717, 4.378,
        4.416, 5.07, 7.171, 8.629, 8.28, 7.322, 5.461, 4.066,
        5.238, 6.696, 7.962, 8.005, 7.566, 6.343, 4.885, 3.521,
        6.999, 7.303, 7.069, 6.611, 6.095, 5.15, 4.119, 3.085,
        5.731, 5.486, 5.33, 4.753, 4.339, 3.913, 3.263, 2.628
    };
    static const std::vector<float> standard_error_chrom = {
        8.063, 3.821, 3.019, 2.782, 2.283, 2.041, 1.861, 1.687,
        4.325, 3.354, 2.839, 2.582, 2.154, 2.077, 1.75, 1.526,
        3.624, 3.092, 3.018, 2.485, 2.08, 2.01, 1.728, 1.488,
        3.566, 3.069, 2.653, 2.367, 2.158, 1.928, 1.632, 1.418,
        2.865, 2.546, 2.399, 2.286, 2.075, 1.861, 1.58, 1.386,
        2.48, 2.316, 2.218, 2.095, 1.935, 1.719, 1.479, 1.243,
        2.208, 2.05, 1.943, 1.826, 1.717, 1.555, 1.344, 1.155,
        1.937, 1.684, 1.648, 1.527, 1.45, 1.353, 1.217, 1.075
    };

    assert(standard_error_lum.size() == data.size());

    std::vector<int> quantize_table(64);
    for (int i = 0; i < data.size(); i++) {
        int l = 5, r = 512;

        while (r - l > 1) {
            int mid = (l + r) >> 1;
            int error = 0;

            for (int j = 0; j < data[i].size(); j++) {
                error += abs(data[i][j] - data[i][j] / mid * mid);
            }

            if (use_lum && (1.0 * error / data[i].size()) < (scale * standard_error_lum[i])) {
                l = mid;
            } else if (!use_lum && (1.0 * error / data[i].size()) < (scale * standard_error_chrom[i])) {
                l = mid;
            } else {
                r = mid;
            }
        }

        quantize_table[i] = l;
    }

    return quantize_table;
}

std::vector<iYCbCr> quantize(std::vector<iYCbCr> block_data, std::vector<int> &quan_lum, std::vector<int> &quan_chrom) {
    std::vector<iYCbCr> block_quan_data(block_data.size(), iYCbCr {0, 0, 0});

    for (int i = 0; i < block_data.size(); i++) {
        block_quan_data[i].y = block_data[i].y / quan_lum[i];
        block_quan_data[i].cb = block_data[i].cb / quan_chrom[i];
        block_quan_data[i].cr = block_data[i].cr / quan_chrom[i];
    }

    return block_quan_data;
}

std::vector<std::vector<int>> get_zigzag_order(int block) {
    static const int d[2][2] = {{1, -1}, {-1, 1}};
    static const int corner[2][4] = {{1, 0, 0, 1}, {0, 1, 1, 0}};

    std::vector<std::vector<int>> order(block * block, std::vector<int>(2, 0));

    int cnt = 0;
    int i = 0, j = 0, up = 1;
    bool turned = false;

    while (i < block && j < block) {
        order[cnt] = {i, j};
        cnt++;

        if (i == 0 || j == 0 || i == block - 1 || j == block - 1) {
            if (!turned) {
                int k = 2 * (up * (j / (block - 1)) | (1 - up) * (i / (block - 1)));
                i += corner[up][k];
                j += corner[up][k + 1];
                turned = true;
                up = 1 - up;
                continue;
            } else {
                turned = false;
            }
        }
        i += d[up][0];
        j += d[up][1];
    }

    return order;
}

std::vector<iYCbCr> zigzag(std::vector<iYCbCr> block_data) {
    std::vector<iYCbCr> block_zigzag_data(block_data.size(), iYCbCr {0, 0, 0});

    int block = std::sqrt(block_data.size());
    std::vector<std::vector<int>> order = get_zigzag_order(block);

    for (int i = 0; i < order.size(); i++) {
        block_zigzag_data[i].y = block_data[order[i][0] * block + order[i][1]].y;
        block_zigzag_data[i].cb = block_data[order[i][0] * block + order[i][1]].cb;
        block_zigzag_data[i].cr = block_data[order[i][0] * block + order[i][1]].cr;
    }

    return block_zigzag_data;
}

std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> get_statistics_before_quantize(std::vector<std::vector<dYCbCr>> &YCbCr_data) {
    const int block = 8;

    int height = YCbCr_data.size();
    int width = YCbCr_data[0].size();
    int block_num = (height / block) * (width / block);

    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> statistics_data = {
        std::vector<std::vector<int>>(block * block, std::vector<int>(0)),
        std::vector<std::vector<int>>(block * block, std::vector<int>(0))
    };

    for (int i = 0; i < block_num; i++) {
        int col = i % (width / block) * block;
        int row = i / (width / block) * block;

        // dct
        std::vector<iYCbCr> block_DCT_data = do_2d_DCT(YCbCr_data, row, col, block);
        
        for (int j = 0; j < block_DCT_data.size(); j++) {
            statistics_data.first[j].push_back(block_DCT_data[j].y);
            statistics_data.second[j].push_back(block_DCT_data[j].cb);
            statistics_data.second[j].push_back(block_DCT_data[j].cr);
        }
    }

    return statistics_data;
}

std::vector<std::vector<iYCbCr>> do_partition_process(std::vector<std::vector<dYCbCr>> &YCbCr_data, std::vector<int> &quan_lum=quan_lum, std::vector<int> &quan_chrom=quan_chrom) {
    const int block = 8;

    int height = YCbCr_data.size();
    int width = YCbCr_data[0].size();
    int block_num = (height / block) * (width / block);
    std::vector<std::vector<iYCbCr>> blocks_data(std::vector(block_num, std::vector(block * block, iYCbCr {0, 0, 0})));

    for (int i = 0; i < block_num; i++) {
        int col = i % (width / block) * block;
        int row = i / (width / block) * block;

        // dct
        std::vector<iYCbCr> block_DCT_data = do_2d_DCT(YCbCr_data, row, col, block);

        // quantize
        std::vector<iYCbCr> block_quan_data = quantize(block_DCT_data, quan_lum, quan_chrom);

        // zig zag
        std::vector<iYCbCr> block_zigzag_data = zigzag(block_quan_data);

        for (int j = 0; j < block * block; j++) {
            blocks_data[i][j].y = block_zigzag_data[j].y;
            blocks_data[i][j].cb = block_zigzag_data[j].cb;
            blocks_data[i][j].cr = block_zigzag_data[j].cr;
        }
    }

    return blocks_data;
}

// bit vector
void BitVector::add_bit(unsigned char b) {
    data[data.size() - 1] |= b << space;
    if (space == 0) {
        space = 7;
        data.push_back(0);
    } else {
        space--;
    }
}

void BitVector::add_bits(int value, int length) {
    for (int i = length - 1; i >= 0; i--) {
        if (value & (1 << i)) {
            add_bit(1);
        } else {
            add_bit(0);
        }
    }
}

void BitVector::print_binary() {
    for (int i = 0; i < data.size(); i++) {
        std::cout << "0b";
        int limit = 0;

        if (i == data.size() - 1) {
            limit = space + 1;
        }

        for (int j = 7; j >= limit; j--) {
            if (data[i] & (1 << j)) {
                std::cout << 1;
            } else {
                std::cout << 0;
            }
        }
        std::cout << "\n";
    }
}

void BitVector::write_binary(std::ofstream &file) {
    for (int i = 0; i < data.size(); i++) {
        file.put(data[i]);
        if (data[i] == 0xFF) {
            file.put(0x00);
        }
    }
}

// encoding to binary format
int get_VLI(int value) {
    int size = 0;

    value = abs(value);
    for (int i = 31; i >= 0; i--) {
        if (value & (1 << i))
            break;
        size++;
    }
    size = 32 - size;

    return size;
}

void to_binary_str(int code, int n_bits, BitVector &in) {
    std::vector<int> reverse = { 0, 1 };
    if (code < 0) {
        reverse = { 1, 0 };
    }

    for (int i = n_bits - 1; i >= 0; i--) {
        if (abs(code) & 1 << i) {
            in.add_bit(reverse[1]);
        } else {
            in.add_bit(reverse[0]);
        }
    }
}

std::map<int, HuffmanInfo> preprocess_DHT(const std::vector<int> &table) {
    std::map<int, HuffmanInfo> DHT_info;

    int symbol_offset = 16;
    int code = 0;

    for (int i = 0; i < 16; i++) {
        int num = table[i];
        int n_bits = i + 1;

        for (int j = 0; j < num; j++) {
            int symbol = table[symbol_offset];

            DHT_info[symbol] = HuffmanInfo {n_bits, code};
            code++;
            symbol_offset++;
        }
        code <<= 1;
    }

    return DHT_info;
}

void write_SOI_section(std::ofstream &file) {
    file.put(0xFF);
    file.put(0xD8);
}

void write_SOF0_section(std::ofstream &file, int height, int width) {
    int SOF0_len = 2 + 1 + 2 + 2 + 1 + 3 * 3;
    file.put(0xFF);
    file.put(0xC0);
    file.put(SOF0_len >> 8);
    file.put(SOF0_len >> 0);
    file.put(0x08);
    file.put(height >> 8);
    file.put(height >> 0);
    file.put(width >> 8);
    file.put(width >> 0);
    file.put(0x03);

    file.put(0x01); file.put(0x11); file.put(0x00);
    file.put(0x02); file.put(0x11); file.put(0x01);
    file.put(0x03); file.put(0x11); file.put(0x01);
}

void write_DQT_section(std::ofstream &file, int num, const std::vector<int> &table) {
    int DQT_len = 2 + 1 + 64;

    file.put(0xFF);
    file.put(0xDB);
    file.put(DQT_len >> 8);
    file.put(DQT_len >> 0);

    file.put(num);

    int block = std::sqrt(table.size());
    std::vector<std::vector<int>> order = get_zigzag_order(block);

    for (int i = 0; i < order.size(); i++) {
        file.put(table[order[i][0] * block + order[i][1]]);
    }
}

void write_huffman_section(std::ofstream &file, int num, const std::vector<int> &table) {
    int HT_len = 16 + 2 + 1;
    for (int i = 0; i < 16; i++) {
        HT_len += table[i];
    }

    file.put(0xFF);
    file.put(0xC4);
    file.put(HT_len >> 8);
    file.put(HT_len >> 0);
    file.put(num);

    assert(HT_len - 3 == table.size());

    for (int i = 0; i < HT_len - 3; i++) {
        file.put(table[i]);
    }
}

void write_SOS_section(std::ofstream &file) {
    int SOS_len = 2 + 1 + 2 * 3 + 3;

    file.put(0xFF);
    file.put(0xDA);
    file.put(SOS_len >> 8);
    file.put(SOS_len >> 0);
    file.put(0x03);

    file.put(0x01); file.put(0x00);
    file.put(0x02); file.put(0x11);
    file.put(0x03); file.put(0x11);

    file.put(0x00); 
    file.put(0x3F);
    file.put(0x00); 
}

void write_data_section(
    std::ofstream &file, std::vector<std::vector<iYCbCr>> &blocks_data,
    int get_statistics,
    void *huffman_lum_ac,
    void *huffman_lum_dc,
    void *huffman_chrom_ac,
    void *huffman_chrom_dc
) {
    BitVector bit_data;

    for (int i = 0; i < blocks_data.size(); i++) {
        for (int channel = 0; channel < 3; channel++) {
            // DC
            int dc_value;
            if (i == 0) {
                dc_value = (channel == 0) ? blocks_data[i][0].y
                    : (channel == 1) ? blocks_data[i][0].cb
                    : blocks_data[i][0].cr;
            } else {
                dc_value = (channel == 0) ? blocks_data[i][0].y - blocks_data[i - 1][0].y
                    : (channel == 1) ? blocks_data[i][0].cb - blocks_data[i - 1][0].cb
                    : blocks_data[i][0].cr - blocks_data[i - 1][0].cr;
            }

            int len = get_VLI(dc_value);
            if (!get_statistics) {
                HuffmanInfo info = (channel == 0) ? (*(std::map<int, HuffmanInfo> *)huffman_lum_dc)[len] : (*(std::map<int, HuffmanInfo> *)huffman_chrom_dc)[len];
                to_binary_str(info.code, info.n_bits, bit_data);
                to_binary_str(dc_value, len, bit_data);
            } else {
                if (channel == 0) {
                    (*(std::vector<int> *)huffman_lum_dc)[len]++;
                } else {
                    (*(std::vector<int> *)huffman_chrom_dc)[len]++;
                }
            }

            // AC
            int zero_cnt = 0;
            for (int j = 1; j < blocks_data[0].size(); j++) {
                int ac_value = (channel == 0) ? blocks_data[i][j].y
                    : (channel == 1) ? blocks_data[i][j].cb
                    : blocks_data[i][j].cr;

                if (ac_value == 0) {
                    zero_cnt++;
                    if (zero_cnt == 16) {
                        if (!get_statistics) {
                            HuffmanInfo info = (channel == 0) ? (*(std::map<int, HuffmanInfo> *)huffman_lum_ac)[0xF0] : (*(std::map<int, HuffmanInfo> *)huffman_chrom_ac)[0xF0];
                            to_binary_str(info.code, info.n_bits, bit_data);
                        } else {
                            if (channel == 0) {
                                (*(std::vector<int> *)huffman_lum_ac)[0xF0]++;
                            } else {
                                (*(std::vector<int> *)huffman_chrom_ac)[0xF0]++;
                            }
                        }

                        zero_cnt = 0;
                    }
                } else {
                    int len = get_VLI(ac_value);
                    int merge_num = (zero_cnt << 4) + len;

                    if (!get_statistics) {
                        HuffmanInfo info = (channel == 0) ? (*(std::map<int, HuffmanInfo> *)huffman_lum_ac)[merge_num] : (*(std::map<int, HuffmanInfo> *)huffman_chrom_ac)[merge_num];
                        to_binary_str(info.code, info.n_bits, bit_data);
                        to_binary_str(ac_value, len, bit_data);
                    } else {
                        if (channel == 0) {
                            (*(std::vector<int> *)huffman_lum_ac)[merge_num]++;
                        } else {
                            (*(std::vector<int> *)huffman_chrom_ac)[merge_num]++;
                        }
                    }

                    zero_cnt = 0;
                }
            }

            if (zero_cnt != 0) {
                if (!get_statistics) {
                    HuffmanInfo info = (channel == 0) ? (*(std::map<int, HuffmanInfo> *)huffman_lum_ac)[0x00] : (*(std::map<int, HuffmanInfo> *)huffman_chrom_ac)[0x00];
                    to_binary_str(info.code, info.n_bits, bit_data);
                } else {
                    if (channel == 0) {
                        (*(std::vector<int> *)huffman_lum_ac)[0x00]++;
                    } else {
                        (*(std::vector<int> *)huffman_chrom_ac)[0x00]++;
                    }
                }
            }
                
        }
    }

    if (!get_statistics) {
        bit_data.write_binary(file);
    }
}

void write_EOI_section(std::ofstream &file) {
    file.put(0xFF);
    file.put(0xD9);
}

void write_jpeg(
    std::string &filename, int height, int width, std::vector<std::vector<iYCbCr>> &blocks_data,
    std::vector<int> &quan_lum,
    std::vector<int> &quan_chrom,
    std::vector<int> &huffman_lum_ac,
    std::vector<int> &huffman_lum_dc,
    std::vector<int> &huffman_chrom_ac,
    std::vector<int> &huffman_chrom_dc
) {
    std::ofstream file(filename, std::ios::binary);

    std::map<int, HuffmanInfo> huffman_info_lum_ac = preprocess_DHT(huffman_lum_ac);
    std::map<int, HuffmanInfo> huffman_info_lum_dc = preprocess_DHT(huffman_lum_dc);
    std::map<int, HuffmanInfo> huffman_info_chrom_ac = preprocess_DHT(huffman_chrom_ac);
    std::map<int, HuffmanInfo> huffman_info_chrom_dc = preprocess_DHT(huffman_chrom_dc);

    // SOI
    write_SOI_section(file);

    // DQT
    write_DQT_section(file, 0, quan_lum);
    write_DQT_section(file, 1, quan_chrom);

    // SOF0
    write_SOF0_section(file, height, width);

    // DHT AC, DC
    write_huffman_section(file, 0 + 0x10, huffman_lum_ac);
    write_huffman_section(file, 1 + 0x10, huffman_chrom_ac);
    write_huffman_section(file, 0 + 0x00, huffman_lum_dc);
    write_huffman_section(file, 1 + 0x00, huffman_chrom_dc);

    // SOS
    write_SOS_section(file);

    // data
    write_data_section(
        file, blocks_data,
        0,
        &huffman_info_lum_ac,
        &huffman_info_lum_dc,
        &huffman_info_chrom_ac,
        &huffman_info_chrom_dc
    );

    // EOI
    write_EOI_section(file);

    file.flush();
    file.close();
}

// convert
void convert_normal_jpeg(std::string &in_filename, std::string &out_filename) {
    PPM image = load_PPM(in_filename);

    std::vector<std::vector<RGB>> RGB_data = PPM_data_to_vector(image);
    std::vector<std::vector<dYCbCr>> YCbCr_data = RGB_to_YCbCr(RGB_data);
    std::vector<std::vector<iYCbCr>> blocks_data = do_partition_process(YCbCr_data);

    image.width -= image.width % 8;
    image.height -= image.height % 8;

    write_jpeg(
        out_filename, image.height, image.width, blocks_data,
        quan_lum,
        quan_chrom,
        huffman_lum_ac,
        huffman_lum_dc,
        huffman_chrom_ac,
        huffman_chrom_dc
    );
}

void convert_adjusted_DHT_jpeg(std::string &in_filename, std::string &out_filename) {
    PPM image = load_PPM(in_filename);

    std::vector<std::vector<RGB>> RGB_data = PPM_data_to_vector(image);
    std::vector<std::vector<dYCbCr>> YCbCr_data = RGB_to_YCbCr(RGB_data);
    std::vector<std::vector<iYCbCr>> blocks_data = do_partition_process(YCbCr_data);

    std::vector<int> lum_ac_cnt(0xFF + 1, 0);
    std::vector<int> lum_dc_cnt(0xFF + 1, 0);
    std::vector<int> chrom_ac_cnt(0xFF + 1, 0);
    std::vector<int> chrom_dc_cnt(0xFF + 1, 0);

    std::ofstream useless_file;
    write_data_section(
        useless_file, blocks_data,
        1,
        &lum_ac_cnt, 
        &lum_dc_cnt,
        &chrom_ac_cnt,
        &chrom_dc_cnt
    );

    // setup JPEG
    std::vector<int> huffman_lum_ac = huffman_encode(lum_ac_cnt);
    std::vector<int> huffman_lum_dc = huffman_encode(lum_dc_cnt);
    std::vector<int> huffman_chrom_ac = huffman_encode(chrom_ac_cnt);
    std::vector<int> huffman_chrom_dc = huffman_encode(chrom_dc_cnt);

    image.width -= image.width % 8;
    image.height -= image.height % 8;

    write_jpeg(
        out_filename, image.height, image.width, blocks_data,
        quan_lum,
        quan_chrom,
        huffman_lum_ac,
        huffman_lum_dc,
        huffman_chrom_ac,
        huffman_chrom_dc
    );
}

void convert_adjusted_DQT_jpeg(std::string &in_filename, std::string &out_filename, float scale=1.0) {
    PPM image = load_PPM(in_filename);

    std::vector<std::vector<RGB>> RGB_data = PPM_data_to_vector(image);
    std::vector<std::vector<dYCbCr>> YCbCr_data = RGB_to_YCbCr(RGB_data);
    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> statistics_data = get_statistics_before_quantize(YCbCr_data);
    std::vector<int> quan_lum = get_adjusted_quantize_table(statistics_data.first, scale, 1);
    std::vector<int> quan_chrom = get_adjusted_quantize_table(statistics_data.second, scale, 0);

    std::vector<std::vector<iYCbCr>> blocks_data = do_partition_process(YCbCr_data, quan_lum, quan_chrom);

    image.width -= image.width % 8;
    image.height -= image.height % 8;

    write_jpeg(
        out_filename, image.height, image.width, blocks_data,
        quan_lum,
        quan_chrom,
        huffman_lum_ac,
        huffman_lum_dc,
        huffman_chrom_ac,
        huffman_chrom_dc
    );
}