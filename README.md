# PPSE
This project simulates an end-to-end communication chain on a NVIDIA Jetson Nano Developer Kit coded in C/C++. Using the Monte Carlo method, the simulation evaluates error rate performance (BER/FER) and throughput across multiple SNR points over the following pipeline:$$\text{Source Generation} \longrightarrow \text{Channel Coding} \longrightarrow \text{Modulation} \longrightarrow \text{AWGN Channel} \longrightarrow \text{Demodulation} \longrightarrow \text{Decoding} \longrightarrow \text{Performance Monitoring}$$The main focus of this implementation was optimizing simulation throughput by leveraging the ARM NEON SIMD vector processing architecture available on the Jetson Nano.For a detailed explanation of the project, architecture, and results, please refer to the Final_report section.


## arborescence :
* bpsk_mod.c, codec_repetion,source_generate : fichier source des fonctions utilisees
* main.c : the main code for running the simulation
*  notes.md : notes pour plutard
* ./build : cmake file (cleaned)
* ./simulation : files containing the graphs and the simulation .csv files
* ./simulation/compare : contains the simulations of lab 4 



