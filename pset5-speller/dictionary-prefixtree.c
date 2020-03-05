/**
 * Implements a dictionary's functionality.
 */
#include "dictionary.h"
#include <sys/stat.h>
#include <alloca.h>

static u32 DictionarySlots[SUPPORTED_CHARS];
static u32 MemoryPool[SUPPORTED_CHARS * MAX_MEM_SLOTS];
static bool MemoryAllocated = false;
static u32 DictionaryEntryCount = 0;
static u32 MemoryPoolOffset;

#define STRIDE_SIZE 108
#define POOL_SIZE STRIDE_SIZE * MAX_MEM_SLOTS
/**
 * Returns true if word is in dictionary else false.
 */
bool check(const char *word)
{
	i32 word_length = strlen(word);
	i32 letter_counter = 0;
	i32 offset = tolower(word[letter_counter]) - 'a';
	u32* current_slot = &DictionarySlots[offset];
	const u64 MemoryPoolBase = (u64)MemoryPool;
	char c;
	while(true) {
		c = word[letter_counter];
		if (letter_counter == word_length) {
			if (*current_slot & (1 << 31)) {
				return true;
			} else {
				return false;
			}
		} else if(*current_slot & (1 << 30)) {
			if(c == '\'') {
				offset = 26;
			} else {
				offset = tolower(c) - 'a';
			}
			//Set the current slot pointer to the address of the bottom 30 bits of the existing value + MemoryPool + (array)offset
			//u32* is 8 bytes wide on x86_64
			current_slot = (u32*)(((*current_slot) & ~(0b11 << 30)) + MemoryPoolBase + (offset * sizeof(u32)));
			letter_counter += 1;
		} else {
			return false;
		}
	}
	return false;
}

/**
 * Loads dictionary into memory. Returns true if successful else false.
 */
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

    char *dict_buff = calloc(1, sizeof(char) * st.st_size + 1);
    fread(dict_buff, st.st_size, 1, inptr);
    fclose(inptr);
    char *c = dict_buff;
    i32 offset = 0;
    u32* current_slot = NULL;
	const u64 MemoryPoolBase = (u64)MemoryPool;
	while(true) {
		new_word:
		if(*c == 0) {
			break;
		} else if(*c == '\'') {
			offset = 26;
		} else {
			offset = tolower(*c) - 'a';
		}
		current_slot = &DictionarySlots[offset];


		next_letter:
		if(*c == 0) {
			break;
		} else if(*c == '\n') {
			++DictionaryEntryCount;
			*current_slot |= (1 << 31);
			c += 1;
			goto new_word;
		} else if((*current_slot & (1 << 30)) == 0) {
			*current_slot += MemoryPoolOffset + (1 << 30);
			MemoryPoolOffset += STRIDE_SIZE;
			assert(MemoryPoolOffset < POOL_SIZE);
		}

		if(*c == '\'') {
			offset = 26;
		} else {
			offset = tolower(*c) - 'a';
		}

		//Set the current slot pointer to the address of the bottom 30 bits of the existing value + MemoryPool + (array)offset
		//u32* is 8 bytes wide on x86_64
		current_slot = (u32*)(((*current_slot) & ~(0b11 << 30)) + MemoryPoolBase + (offset * sizeof(u32)));
		c += 1;
		goto next_letter;
	}
	free(dict_buff);
    return true;
}

/**
 * Returns number of words in dictionary if loaded else 0 if not yet loaded.
 */
unsigned int size(void)
{
    return DictionaryEntryCount;
}

/**
 * Unloads dictionary from memory. Returns true if successful else false.
 */
bool unload(void)
{
    return true;
}
