# Final PPSE Project Report


## Blocs 
 The different blocs in the simulation chain and their current performance.
 
 * baseline simulation N = 32, K = 128.
 

### Source : 

Throughput average : 26  Mbps    
%% total : 6%  
Function used : source_generate
### Encoder : 
Throughput  Average : 46 Mpbs  
%% total  : 3%  
Function used : codec_repetition_encode
### Modulator : 
Throughput  Average : 1.4 Gpbs  
%% total  : 7%  
Function used : modem_bpsk_modulate

### Channel : 
Throughput  Average : 9.95 Mpbs  
%% total  : 64%  
Function used : channel_AWGN_add_noise

### Demodulator : 
Throughput  Average : 185 Mpbs  
%% total  : 3%  
Function used : modem_BPSK_demodulate

### Decode 
Two types of decoders hard and soft.
#### No quantization 
Throughput  Average for Hard decoder : 13 Mpbs  
%% total  : 12%  
Function used : codec_repetition_hard_decode

Throughput Average for Soft decoder : 45 mpbs 
%% total : 4%  
Function used : codec_repetition_soft_decode



### Monitor : 
Throughput  Average : 150 Mpbs  
%% total  : 1%  
Function used : monitor_check_errors



### Results and graph
Fer and Ber graphs At the end of the simulation chain.    

stopped at 15 db for hard decoder and 12 for the soft decoder. 
 
Soft decoder is mopre efficient and has better rates than the hard decoder.

![alt text](hard_vs_soft_decoder.png)
![alt text](code_rates_ebn0.png) 
![alt text](code_rates_esn0.png)

## First steps of optimization
### Quantization :
The goal here was to have a better simulation throughput.
the strategy was to accelerate the decoder bloc by implementing a quantizer transforming from float to int8 then using special decoding function compatible with the int8 function.
![alt text](../simulations/hard_vs_soft_decoder.png)

![alt text](../simulations/code_rates_ebn0.png)
![alt text](../simulations/code_rates_esn0.png)
comparison between BER/FER with and without the int8 we see a worse BER/FER  and that's due to the loss of pression caused by the quantization.
as for the hard decoder no loss has been noticed since we only need the bit sign 
quantization dont affect the BEF/FER curve.
Note : for R=1 the soft decoder acts like a hard decoder there for same result as without the int8.
### Neon Function :
So even though we added a quantizer bloc the global throughput didnt improve much.
the goal here was to use the nano's special SIMD function (single instruction multiple data) that were compatible with the int8 format.

for the simulation testing we used K =32 N=8192(the number of repetition is high we will get to that later).
we focus on the Decoder's throughput
for baseline  used the decoder compatible with float format (results in ../simulations/compare/ folder)
#### soft decoder :
an average throughput of 0.7 mbps for the decoder
simulation throughput was at 0.03 mbps
##### with the quantizer bloc :
pratically the same results as we discussed earlier and a slightly worse sim throughput .
##### with the simd functions : 
decoder throughput : 5.5 mbps
big improvement but the simulation throughput didnt improve much

we also notice a degradation in the FER/BER curves and thats due to two reasons :
* The loss of precision of with the quantizer
* the saturation in the the decoder when using the neon functions, the task in lab4 fixed the instructions allowed and there for we could only use int8 for the soft decoder when calculating the sum for 256 reps its a very likelyt chance that the int8 saturates (int8 holds till 127) and thats what caused the detoriesation fo the fer and ber curves.
#### hard decoder :
an average throughput of 0.2 mbps for the decoder 
simulation throughput was at 0.029 mbps
##### with the quantizer bloc :
pratically the same results as we discussed earlier and a slightly worse sim throughput . 0.025 
##### with the simd functions : 
decoder throughput : 1.3 mbps
big improvement but the simulation throughput didnt improve much
## Mini Project grounds
Before we start i modify my decoders code to remove the malloc and memcpy which bumped out the throughput of the decoder by a whole 10 mbps on average(solely for the hard decoder, the soft didnt have the malloc).

I will focus solely on axe 2 

### Modulator : 
**Task1**
For starters i am gonna ditch the X_N result being an int32, it over complicates the things, so instead we are using int8 X_N a 1:1 correspondance with the input (the encoder).
The channel still takes int32 (subject to change later on) for the test we are going to focus on the throughput of the Modulator block so we just convert the int8 to int32 outside the bloc.
- Baseline is K 32 and N 128 :

Throughput  Average before vectorization: 1.4 Gpbs.

Throughput  Average before vectorization: 2 Gpbs. 

Quite a significant gain it gets even larger with larger N :
- Baseline K 32 N 8192 : 

Throughput  Average before vectorization: 3 Gpbs.

Throughput  Average before vectorization: 12 Gpbs.

To activate the module we use --neonmod
*Function used : void modem_bpsk_modulate_neon(const uint8_t C_N, int8_t X_N, size_t N);*
## Instructions used 
        int8x16_t vreinterpretq_s8_u8(uint8x16_t a);  Reinterprets the bits of a 128-bit NEON register from one type to another without data transformation.
        int8x8_t vbsl_s8(uint8x8_t a, int8x8_t b, int8x8_t c); bit select logic Performs bitwise selection between two vectors based on a selector mask.
