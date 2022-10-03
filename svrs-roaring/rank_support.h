#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "roaring.h"
include "roaring.c"

bitset_container_t *m_v;

static uint32_t args_in_the_word_d(uint32_t, uint32_t&)
{
    return 0;
}

static uint32_t word_rank_d(const uint32_t*, uint32_t)
{
    return 0;
}

static uint32_t full_word_rank_d(const uint32_t*, uint32_t)
{
    return 0;
}

static uint32_t init_carry_d()
{
    return 0;
}
    
static uint32_t args_in_the_word(uint32_t w, uint32_t&)
{
    return hamming(w);
}

static uint32_t word_rank(const uint32_t* data, uint32_t idx)
{
    return	hamming(*(data+(idx>>6));
}

static uint32_t full_word_rank(const uint32_t* data, uint32_t idx)
{
    return	hamming(*(data+(idx>>6)));
}

uint32_t *m_basic_block; //TODO: criar vetor de tamanho variavel

void rank_support_v5(const bitset_container_t* v = nullptr) {
    m_v = v;
    if (v == nullptr) {
        return;
    } else if (bitset_container_empty(v)) {
        m_basic_block = malloc(sizeof(uint32_t)); //analisar melhor
        return;
    }
    uint32_t basic_block_size = ((65536 >> 11)+1)<<1; //65536 eh a cardinalidade maxima do bitset container
    m_basic_block = calloc(basic_block_size, sizeof(uint32_t));   // resize structure for basic_blocks
    //if (m_basic_block.empty())
    //    return;
    const uint32_t* data = m_v->words;
    uint32_t i, j=0;
    m_basic_block[0] = m_basic_block[1] = 0;

    uint32_t carry = init_carry();
    uint32_t sum   = args_in_the_word_d(*data, carry);
    uint32_t second_level_cnt = 0;
    uint32_t cnt_words=1;
    for (i = 1; i < (sizeof(m_v)/(sizeof(uint16_t))>>6) ; ++i, ++cnt_words) {
        if (cnt_words == 32) {
            j += 2;
            m_basic_block[j-1] = second_level_cnt;
            m_basic_block[j]     = m_basic_block[j-2] + sum;
            second_level_cnt = sum = cnt_words = 0;
        } else if ((cnt_words%6)==0) {
            // pack the prefix sum for each 6x64bit block into the second_level_cnt
            second_level_cnt |= sum<<(60-12*(cnt_words/6));//  48, 36, 24, 12, 0
        }
        sum += args_in_the_word_d(*(++data), carry);
    }

    if ((cnt_words%6)==0) {
        second_level_cnt |= sum<<(60-12*(cnt_words/6));
    }
    if (cnt_words == 32) {
        j += 2;
        m_basic_block[j-1] = second_level_cnt;
        m_basic_block[j]   = m_basic_block[j-2] + sum;
        m_basic_block[j+1] = 0;
    } else {
        m_basic_block[j+1] = second_level_cnt;
    }
}

uint32_t rank(uint16_t idx) const {
    assert(m_v != nullptr);
    assert(idx <= sizeof(m_v->words)/sizeof(uint32_t)); //make sure idx < bitset size
    const uint64_t* p = m_basic_block.() // o que colocar aqui?????????
                        + ((idx>>10)&(0xFFFFFFFFFFFFFFFEULL<<1));// (idx/1024)*2
//                     ( prefix sum of the 6x32bit blocks | (idx%1024)/(32*6) )
    size_type result = *p
                        + ((*(p+1)>>(60-12*((idx&0x7FF<<1)/(32*6))))&(0x7FFULL<<1))
                        + word_rank(m_v->words, idx);
    idx -= (idx&(0x3F<<1));
    uint8_t to_do = ((idx>>6)&(0x1FULL<<1))%6;
    --idx;
    while (to_do) {
        result += full_word_rank_d(m_v->words, idx);
        --to_do;
        idx-=32;
    }
    return result;
}

