#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stddef.h>

#define MAGIC "heart"
#define MAGIC_LEN 5
#define DEFUALT_HASHMAP_SIZE INT32_MAX
uint64_t SEED  = 1207;

typedef struct Hashmap_Entry{
    uint32_t index;
    uint64_t blob_seek;
}hashmap_entry;

typedef struct header{
    char* magic;
    uint64_t map_ptr; //Offset to start of map
    uint64_t free_ptr; // Offser to first free block
}header;

//Murmurhash3 function for int64 (copied i go no clue how does this shit work)
uint64_t murmurhash3_64(const void *key, size_t len, uint64_t seed) {
    const uint64_t c1 = 0x87c37b91114253d5ULL;
    const uint64_t c2 = 0x4cf5ad432745937fULL;
    const uint8_t *data = (const uint8_t *)key;
    const int nblocks = len / 8;
    
    uint64_t h1 = seed;
    uint64_t h2 = seed;

    const uint64_t *blocks = (const uint64_t *)(data);
    for (int i = 0; i < nblocks; i++) {
        uint64_t k1 = blocks[i];

        k1 *= c1;
        k1 = (k1 << 31) | (k1 >> (64 - 31)); 
        k1 *= c2;
        h1 ^= k1;
        
        h1 = (h1 << 27) | (h1 >> (64 - 27)); 
        h1 = h1 * 5 + 0x52dce729;
    }

    const uint8_t *tail = (const uint8_t *)(data + nblocks * 8);
    uint64_t k1 = 0;

    switch (len & 7) { 
        case 7: k1 ^= ((uint64_t)tail[6]) << 48;
        case 6: k1 ^= ((uint64_t)tail[5]) << 40;
        case 5: k1 ^= ((uint64_t)tail[4]) << 32;
        case 4: k1 ^= ((uint64_t)tail[3]) << 24;
        case 3: k1 ^= ((uint64_t)tail[2]) << 16;
        case 2: k1 ^= ((uint64_t)tail[1]) << 8;
        case 1: k1 ^= ((uint64_t)tail[0]);
                k1 *= c1; k1 = (k1 << 31) | (k1 >> (64 - 31)); k1 *= c2;
                h1 ^= k1;
    }

    h1 ^= len;
    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccdULL;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53ULL;
    h1 ^= h1 >> 33;

    return h1;
}


/* 
    initial idea : 
    HEADER : MAGIC (5 bytes) + num_pages (4 bytes) + object_count (4 bytes) + last_seek_pos (8 byte) = 13 bytes
    HASH_MAP : we need to create INT32_MAX sized array for the hashmap from 14th byte to idk byte
    each entry into the HASH_MAP is hashmap_entry size ie (4 bytes + 8 bytes)
*/





void write_header(FILE* fp,header Header){
    fwrite(&Header,sizeof(header),1,fp);
}


int read_header(FILE* fp){
    char magic[5] = {0};
    if(fread(magic,sizeof(char),MAGIC_LEN,fp) != MAGIC_LEN){
        return -1;
    }

    if(strcmp(MAGIC,magic) != 0){
        fprintf(stderr,"INVALID FILE FORMAT MAGIC NUMBER MISMATCH\n");
        return -1;
    }
    else{
        printf("magic : %s\n",magic);
    }

    int max_num_records = 0;
    if(fread(&max_num_records,sizeof(uint32_t),1,fp) != 1){
        return -1;
    }
    printf("MAX NUMBER OF RECORDS : %d\n",max_num_records);
    int num_records = 0;
    if(fread(&num_records,sizeof(uint32_t),1,fp) != 1){
        return -1;
    }
    printf("NUMBER OF RECORDS PRESENT : %d\n",num_records);
    return 1;
}


int main(void){
    FILE* fp = fopen("map.obj","w+b");
    if(fp == NULL){
        perror("Error opening the file");
        exit(EXIT_FAILURE);
    }
    header Header = {
        .free_ptr = 200,
        .magic = MAGIC,
        .map_ptr = 15
    };
    write_header(fp,Header);
    rewind(fp);
    if(read_header(fp) != 1){
        perror("ERROR READING FILE HEADER");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    return 0;
}