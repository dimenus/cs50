/**
 * Implements a dictionary's functionality.
 */
#include "dictionary.h"
#include <sys/stat.h>
#include <alloca.h>

/**
 * Returns true if word is in dictionary else false.
 */

bool hash_gen(const char *str, size_t *out_hash) {
    size_t i = 0;
    uint64_t shift = 7;
    uint64_t hash = 0;
    while (i < 8) {
        if (str[i] == 0 || str[i] == '\n') { 
            *out_hash = hash;
            return true; 
        };
        uint64_t v = str[i] & ~0x20;
        hash += v << (shift * 8); 
        shift -= 1;
        i += 1;
    }
    *out_hash = hash;
    if (str[8] != 0 && str[8] != '\n') {
        return false;
    } else {
        return true;
    }
}

typedef uint64_t SmallHash;

typedef struct LargeHash {
    uint64_t high;
    uint64_t low;
} LargeHash;

/**
 * Loads dictionary into memory. Returns true if successful else false.
 */

#define MAX_SMALL_HASHES 75000
SmallHash smallHashTable[MAX_SMALL_HASHES];
size_t smallIndices[26][26];
static size_t smallHashIdx = 0;

#define MAX_APOS_HASHES 1000
SmallHash aposHashTable[MAX_APOS_HASHES];
static size_t aposHashIdx = 0;
void smallhash_push(uint64_t hash)
{
    static size_t last_first_offset = 0;
    static size_t last_second_offset = 0;
    uint8_t first_byte = (uint8_t)((hash & ((uint64_t)0xFF << 56L)) >> 56L);
    uint8_t second_byte = (uint8_t)((hash & ((uint64_t)0xFF << 48LL)) >> 48L);
    size_t first_offset = first_byte - 'A';
    size_t second_offset = (second_byte == 0) ? 0 : second_byte - 'A';
    assert(first_byte != 0x07);
    if (second_byte == 0x07) {
        aposHashTable[aposHashIdx++] = hash;
        if (last_first_offset != first_offset) {
            for (size_t i = last_second_offset + 1; i < 26; i++) {
                smallIndices[last_first_offset][i] = smallHashIdx;
            }
            smallIndices[first_offset][0] = smallHashIdx;
        }
        second_offset = 0;
    } else {
        if (last_first_offset != first_offset) {
            for (size_t i = last_second_offset + 1; i < 26; i++) {
                smallIndices[last_first_offset][i] = smallHashIdx;
            }
            for (size_t i = 0; i <= second_offset; i++) {
                smallIndices[first_offset][i] = smallHashIdx;
            }
        } else if (last_second_offset != second_offset) {
            for (size_t i = last_second_offset + 1; i <= second_offset; i++) {
                smallIndices[last_first_offset][i] = smallHashIdx;
            }
        }
        smallHashTable[smallHashIdx++] = hash;
    }
    last_first_offset = first_offset;
    last_second_offset = second_offset;
    assert(smallHashIdx < MAX_SMALL_HASHES);
    assert(aposHashIdx < MAX_APOS_HASHES);
}

bool smallhash_search_recurse(uint64_t *list, uint64_t targ, size_t min, size_t max) {
    size_t mid = (max + min) / 2;
    if (targ < list[min] || targ > list[max]) {
        return false;
    } 
    if (targ == list[min]) {
        return true;
    }
    if (targ == list[max]) {
        return true;
    }
    if (targ == list[mid]) { 
        return true;
    }
    if (targ < list[mid]) {
        return smallhash_search_recurse(list, targ, min + 1, mid - 1);
    } else if (targ > list[mid]) {
        return smallhash_search_recurse(list, targ, mid + 1, max - 1);
    }
    assert(false);
}

bool smallhash_check(uint64_t hash) {
    uint8_t first_byte = (uint8_t)((hash & ((uint64_t)0xFF << 56L)) >> 56L);
    uint8_t second_byte = (uint8_t)((hash & ((uint64_t)0xFF << 48LL)) >> 48L);
    if (second_byte == 0x07) {
        return smallhash_search_recurse(aposHashTable, hash, 0, aposHashIdx-1);
    } else {
        uint32_t first_offset = first_byte - 'A';
        uint32_t second_offset = (second_byte == 0) ? 0 : second_byte - 'A';
        uint32_t begin = smallIndices[first_offset][second_offset];
        uint32_t end = 0;
        if (second_byte == 25) {
            end = smallIndices[first_offset+1][0];
        } else {
            end = smallIndices[first_offset][second_offset+1];
        }
        return smallhash_search_recurse(smallHashTable, hash, begin, end);
    }
}

#define MAX_LARGE_HASHES 100000
LargeHash largeHashTable[MAX_LARGE_HASHES];
size_t largeIndices[26][26];
static size_t largeHashIdx = 0;

#define MAX_LARGE_APOS_HASHES 1000
LargeHash largeAposHashTable[MAX_LARGE_APOS_HASHES];
static size_t largeAposHashIdx = 0;
void largehash_push(LargeHash hash)
{
    static uint8_t last_first_offset = 0;
    static uint8_t last_second_offset = 0;
    uint8_t first_byte = (uint8_t)((hash.high & ((uint64_t)0xFF << 56L)) >> 56L);
    uint8_t second_byte = (uint8_t)((hash.high & ((uint64_t)0xFF << 48LL)) >> 48L);
    uint8_t first_offset = first_byte - 'A';
    uint32_t second_offset = (second_byte == 0) ? 0 : second_byte - 'A';
    assert(first_byte != 0x07);
    if (second_byte == 0x07) {
        if (last_first_offset != first_offset) {
            for (size_t i = last_second_offset + 1; i < 26; i++) {
                largeIndices[last_first_offset][i] = largeHashIdx;
            }
            largeIndices[first_offset][0] = largeHashIdx;
        }
        second_offset = 0;
        largeAposHashTable[largeAposHashIdx++] = hash;
    } else {
        if (last_first_offset != first_offset) {
            for (size_t i = last_second_offset + 1; i < 26; i++) {
                largeIndices[last_first_offset][i] = largeHashIdx;
            }
            for (size_t i = 0; i <= second_offset; i++) {
                largeIndices[first_offset][i] = largeHashIdx;
            }
        } else if (last_second_offset != second_offset) {
            for (size_t i = last_second_offset + 1; i <= second_offset; i++) {
                largeIndices[last_first_offset][i] = largeHashIdx;
            }
        }
        largeHashTable[largeHashIdx++] = hash;
    }
    last_first_offset = first_offset;
    last_second_offset = second_offset;
    assert(largeAposHashIdx < MAX_LARGE_APOS_HASHES);
    assert(largeHashIdx < MAX_LARGE_HASHES);
}

bool largehash_search_recurse(LargeHash *list, LargeHash targ, size_t min, size_t max) {
    if (targ.high < list[min].high || targ.high > list[max].high) {
        return false;
    } 
    if (targ.high == list[min].high && targ.low == list[min].low) {
        return true;
    }
    if (targ.high == list[max].high && targ.low == list[max].low) {
        return true;
    }
    size_t mid = (max + min) / 2;
    if (targ.high == list[mid].high && targ.low == list[mid].low) {
        return true;
    }
    size_t low_bound_max = mid - 1;
    size_t high_bound_min = mid + 1;
    if (targ.high == list[mid].high) {
        while ((low_bound_max > 0) && list[low_bound_max].high == targ.high) {
            if (list[low_bound_max].low == targ.low) {
                return true;
            }
            low_bound_max -= 1;
        }
        while ((high_bound_min < max) && list[high_bound_min].high == targ.high) {
            if (list[high_bound_min].low == targ.low) {
                return true;
            }
            high_bound_min += 1;
        }
        return false;
    }
    if (targ.high > list[mid].high) {
        return largehash_search_recurse(list, targ, high_bound_min, max - 1);
    } else {
        return largehash_search_recurse(list, targ, min + 1, low_bound_max);
    }
    assert(false);
}

bool largehash_check(LargeHash hash)
{
    uint8_t first_byte = (uint8_t)((hash.high & ((uint64_t)0xFF << 56L)) >> 56L);
    uint8_t second_byte = (uint8_t)((hash.high & ((uint64_t)0xFF << 48LL)) >> 48L);
    if (second_byte == 0x07) {
        return largehash_search_recurse(largeAposHashTable, hash, 0, largeAposHashIdx-1);
    } else {
        uint32_t first_offset = first_byte - 'A';
        uint32_t second_offset = (second_byte == 0) ? 0 : second_byte - 'A';
        uint32_t begin = largeIndices[first_offset][second_offset];
        uint32_t end = 0;
        if (second_byte == 25) {
            end = largeIndices[first_offset+1][0];
        } else {
            end = largeIndices[first_offset][second_offset+1];
        }
        return largehash_search_recurse(largeHashTable, hash, begin, end);
    }
}


typedef struct OverflowHash {
    size_t len;
    char *str;
} OverflowHash;

#define BUF_LEN 255
#define MAX_OVER_HASHES 5000
OverflowHash overHashTable[MAX_OVER_HASHES];
static size_t overHashIdx = 0;
void overflowhash_push(char *str)
{
    uint64_t len = 8 + 8;
    while (str[len] != 0 && str[len] != '\n') { len += 1; }
    assert(overHashIdx < MAX_OVER_HASHES);
    char *ptr = (char*)malloc(len);
    memcpy(ptr, str, len);
    overHashTable[overHashIdx++] = (OverflowHash){
        .len = len,
        .str = ptr,
    };
}
bool overflowhash_check(const char *str)
{
    char buf[BUF_LEN] = { 0 };
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        buf[i] = str[i] | 0x20;
    }
    for (size_t i = 0; i < overHashIdx; i++) {
        if (len != overHashTable[i].len) {
            continue;
        } else if (memcmp(buf, overHashTable[i].str, len) == 0) {
            return true;
        }
    }
    return false;
}

bool load(const char *dictionary)
{
    struct stat st;
    if(stat(dictionary, &st) != 0) {
        return false;
    }

	FILE* inptr = fopen(dictionary, "rb");
	if(inptr == NULL) {
		return false;
	}

    char buf[BUF_LEN] = { 0 };
    size_t num_overflows = 0;
    while (fgets(buf, BUF_LEN, inptr)) {
        uint64_t high_hash = 0;
        if (hash_gen(buf, &high_hash)) {
            smallhash_push(high_hash);
            continue;
        }
        uint64_t low_hash = 0;
        if (hash_gen(&buf[8], &low_hash)) {
            largehash_push((LargeHash){ .high = high_hash , .low = low_hash, });
            continue;
        }
        num_overflows += 1;
        overflowhash_push(buf);
    }

    //for (size_t i = 0; i < 26; i++) {
    //    char c = 'A';
    //    printf("%c = { ", (char)(c + i));
    //    for (size_t j = 0; j < 25; j++) {
    //        printf("%c = %zu(%zu), ", (char)(c + j), largeIndices[i][j], largeIndices[i][j+1]-largeIndices[i][j]);
    //    }
    //    printf("%c = %zu(%zu) }\n", (char)(c + 25), largeIndices[i][25], largeIndices[i+1][0]-largeIndices[i][25]);
    //}
    //printf("large apos: %zu\n", largeAposHashIdx);
    return true;
}

bool check(const char *word)
{
    uint64_t low_hash = 0;
    uint64_t high_hash = 0;
    if (hash_gen(word, &high_hash)) {
        return smallhash_check(high_hash);
    } else if (hash_gen(&word[8], &low_hash)) {
        return largehash_check((LargeHash){ .high = high_hash, .low = low_hash, });
    } else {
        return overflowhash_check(word);
    }
}


/**
 * Returns number of words in dictionary if loaded else 0 if not yet loaded.
 */
unsigned int size(void)
{
    return smallHashIdx + largeHashIdx + overHashIdx;
}

/**
 * Unloads dictionary from memory. Returns true if successful else false.
 */
bool unload(void)
{
    return true;
}
