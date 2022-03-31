#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>

extern const uint8_t woz2_signature[8];
extern const uint8_t ff40_times_4[5];

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define woz_le32toh(x) (x)
#define woz_le16toh(x) (x)
#else
#error these systems are imaginary
#define woz_le32toh(x) (__builtin_bswap32((x)))
#define woz_le16toh(x) (__builtin_bswap16((x)))
#endif

enum {
    CHUNK_INFO = 0x4F464E49,
    CHUNK_TRKS = 0x534B5254,
    CHUNK_TMAP = 0x50414D54,
};

template<typename FileType>
class WozFile {
public:
    WozFile() : file_{} {}

    bool open(FileType *file) {
        close();
        if(!check_signature(file)) { return false; }
        if(!skip_crc(file)) { return false; }
        if(!parse_chunks(file)) { return false; }
        file_ = file;
        return true;
    }

    bool check_signature(FileType *file) {
        uint8_t buf[sizeof(woz2_signature)];
        if(read(file, buf, sizeof(buf)) != sizeof(woz2_signature)) { return false; }
        return !memcmp(buf, woz2_signature, sizeof(woz2_signature));
    }

    bool skip_crc(FileType *file) { 
        readle32(file);
        return true;
    }

    bool parse_chunks(FileType *file) {
        bool saw_info=false, saw_trks=false, saw_tmap=false;
        while(!file->eof()) {
            uint32_t chunk_id = readle32(file);
            uint32_t chunk_size = readle32(file);
            uint32_t chunk_start = tellg(file);
            uint32_t next_chunk = chunk_start + chunk_size;

            printf("Chunk id = 0x%08x size = %d next = %d\n", chunk_id, chunk_size, next_chunk);
            switch(chunk_id) {
                case 0:
                    goto done;
                case CHUNK_INFO:
                    saw_info = true;
                    {
                        auto version = read8(file);
                        if(version >= 2) {
                            seekg(file, chunk_start + 39);
                            optimal_bit_timing_ = read8(file);
                        } else {
                            optimal_bit_timing_ = 32;
                        }
                        if(version >= 2) {
                            seekg(file, chunk_start + 64);
                            largest_track_ = readle16(file);
                        } else {
                            largest_track_ = 0;
                        }
                    }
                    break;

                case CHUNK_TRKS:
                    saw_trks = true;
                    for(size_t i=0; i<160; i++) {
                        track_start[i] = readle16(file);
                        largest_track_ = std::max(largest_track_, readle16(file));
                        track_size[i] = readle32(file);
                    }
                    break;

                case CHUNK_TMAP:
                    saw_tmap = true;
                    read(file, track_map, sizeof(track_map));
                    break;
            }
            seekg(file, next_chunk);
        }
done:
        return saw_info && saw_trks && saw_tmap;
    }

    void close() {
        file_ = nullptr;
        memset(track_map, 255, sizeof(track_map));
        memset(track_start, 0, sizeof(track_start));
        memset(track_size, 0, sizeof(track_size));
        largest_track_ = 0;
        optimal_bit_timing_ = 32;
    }

    size_t largest_track() const { return largest_track_; };
    size_t optimal_bit_timing() const { return optimal_bit_timing_; };

    size_t readtrack(uint8_t track, uint8_t *buf, size_t n_buf) const {
         auto idx = track_map[track];
         uint32_t bit_count=0;
         uint8_t dbuf[largest_track_*512];
         const uint8_t *data=NULL;

         if(idx == 255) {
             bit_count = 8 * sizeof(ff40_times_4);
             data = ff40_times_4;
         } else {
             bit_count = track_size[idx];
             std::cerr << "seek " << (track_start[idx]*512) << "\n";
             seekg(file_, track_start[idx]*512);
             auto byte_count = (bit_count + 7) / 8;
             auto n = read(file_, dbuf, byte_count);
             std::cout << "byte count " << byte_count << " n " << n << "\n";
             bit_count = std::min(bit_count, (uint32_t)(n / 8));
             data = dbuf;
         }
         auto byte_count = (bit_count + 7) / 8;
         if(byte_count > n_buf) {
             return (size_t)-1;
         }

         return convert(buf, data, bit_count);
    }

private:

    static size_t convert(uint8_t *buf, const uint8_t *data, size_t bit_count) {
        uint8_t b=0;;
        for(size_t i=0; i<bit_count; i++) {
            if(i % 8 == 0) {
                b = *data++;
            }
            *buf++ = !!(b & 0x80);
            b <<= 1;
        }
        return bit_count;
    }

    static uint16_t read8(FileType *file) {
        uint8_t u{};
        read(file, &u, sizeof(u));
        return u;
    }

    static uint16_t readle16(FileType *file) {
        uint16_t u{};
        read(file, &u, sizeof(u));
        return woz_le16toh(u);
    }

    static uint32_t readle32(FileType *file) {
        uint32_t u{};
        read(file, &u, sizeof(u));
        return woz_le32toh(u);
    }

    FileType *file_;
    uint8_t track_map[160];
    uint16_t track_start[160];
    uint32_t track_size[160];
    uint8_t optimal_bit_timing_;
    uint16_t largest_track_;
};

