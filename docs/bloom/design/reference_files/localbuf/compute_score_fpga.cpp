#include<iostream>
#include<ctime>
#include<utility>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include"sizes.h"
#include"MurmurHash2.c"



const unsigned int bloom_filter_size = 1<<bloom_size;


extern "C" 
{


void runOnfpga (unsigned int* doc_sizes,unsigned int* input_doc_words,unsigned int* bloom_filter,unsigned long* profile_weights,unsigned long* fpga_profileScore,unsigned int total_num_docs,bool load_weights) {


#pragma HLS INTERFACE s_axilite port=return bundle=control
#pragma HLS INTERFACE s_axilite port=doc_sizes bundle=control
#pragma HLS INTERFACE s_axilite port=input_doc_words bundle=control
#pragma HLS INTERFACE s_axilite port=bloom_filter bundle=control
#pragma HLS INTERFACE s_axilite port=profile_weights bundle=control
#pragma HLS INTERFACE s_axilite port=fpga_profileScore bundle=control
#pragma HLS INTERFACE s_axilite port=total_num_docs bundle=control
#pragma HLS INTERFACE s_axilite port=load_weights bundle=control

#pragma HLS INTERFACE m_axi port=doc_sizes offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=input_doc_words offset=slave bundle=gmem2
#pragma HLS INTERFACE m_axi port=bloom_filter offset=slave bundle=gmem3
#pragma HLS INTERFACE m_axi port=profile_weights offset=slave bundle=gmem4
#pragma HLS INTERFACE m_axi port=fpga_profileScore offset=slave bundle=gmem4

   unsigned int size_offset=0;
   static unsigned int bloom_filter_local[bloom_filter_size];

   if(load_weights==true)
   memcpy(bloom_filter_local,bloom_filter,bloom_filter_size*sizeof(unsigned int));
   
   for(unsigned int doc=0;doc<total_num_docs;doc++) {

   unsigned int size = doc_sizes[doc];
   unsigned long ans=0;
   for (unsigned int i=0; i < size; i++)
   {
    
         unsigned int curr_entry = input_doc_words[size_offset+i];
         unsigned int frequency = curr_entry & 0x00ff;
         unsigned int word_id = curr_entry >> 8;
         unsigned hash_pu = MurmurHash2( &word_id,3,1);
         unsigned hash_lu = MurmurHash2( &word_id,3,5);
         bool doc_end= (word_id==docTag); 
         unsigned hash1 = hash_pu&hash_bloom; 
         unsigned hash2=(hash_pu+hash_lu)&hash_bloom;
         bool inh1 = (!doc_end) && (bloom_filter_local[ hash1 >> 5 ] & ( 1 << (hash1 & 0x1f)));
         bool inh2 = (!doc_end) && (bloom_filter_local[ hash2 >> 5 ] & ( 1 << (hash2 & 0x1f)));

         if (inh1 && inh2)
         {
           ans += profile_weights[word_id] * (unsigned long)frequency;
         }
  }
        size_offset+=size;
        fpga_profileScore[doc]=ans;
 } 

}

}
