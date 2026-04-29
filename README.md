# PPSE
PPSE coder and decoder chain simulation

## arborescence :
* bpsk_mod.c, codec_repetion,source_generate : fichier source des fonctions utilisees
* main.c : the main code for running the simulation
*  notes.md : notes pour plutard
* ./build : cmake file (cleaned)
* ./simulation : files containing the graphs and the simulation .csv files
## simulation throuput :
### src all zeros:
in this i will only take in considiration for the time being an snr 9 point in a soft decoder
* sans --src-all-zeros :
``` 
--- Block Statistics for SNR 9.00 dB ---
Block        | Avg Lat(s) | Min        | Max        | Thr(Mbps)  | %%    
Source       | 1.21e-06 | 1.12e-06 | 1.02e-04 | 26.37      | 6.6%
Encoder      | 6.71e-07 | 5.76e-07 | 1.06e-05 | 47.68      | 3.7%
Modulator    | 1.50e-06 | 8.00e-07 | 8.53e-05 | 85.62      | 8.2%
Channel      | 1.26e-05 | 1.23e-05 | 2.08e-04 | 10.16      | 68.8%
Demodulator  | 7.12e-07 | 5.76e-07 | 4.35e-04 | 179.80     | 3.9%
Decoder      | 7.81e-07 | 7.04e-07 | 1.43e-05 | 40.96      | 4.3%
Monitor      | 2.16e-07 | 1.60e-07 | 6.34e-06 | 148.43     | 1.2%
---------------------------------------------
dec_avg_lat: 7.8126e-07 seconds, dec_thr: 4.0959e+01 Mbps
Time taken for SNR 9.00000000 dB: 1.67520292 seconds
Average time per frame: 1.8310e-05 seconds
rep-soft: Bit Errors = 100, Total Frames = 91491 Frame Errors = 100, BER = 3.4156e-05, FER = 1.0930e-03, Sim_thr = 1.7477e+06
```
* avec --src-all-zeros
``` 
--- Block Statistics for SNR 9.00 dB ---
Block        | Avg Lat(s) | Min        | Max        | Thr(Mbps)  | %%    
Source       | 6.55e-08 | 3.20e-08 | 5.41e-06 | 488.90     | 0.4%
Encoder      | 6.67e-07 | 5.76e-07 | 3.29e-04 | 47.95      | 4.1%
Modulator    | 6.74e-07 | 5.76e-07 | 7.16e-05 | 189.95     | 4.1%
Channel      | 1.26e-05 | 1.22e-05 | 5.04e-04 | 10.18      | 77.1%
Demodulator  | 7.04e-07 | 6.08e-07 | 1.21e-05 | 181.89     | 4.3%
Decoder      | 7.77e-07 | 7.04e-07 | 1.10e-04 | 41.19      | 4.8%
Monitor      | 2.12e-07 | 1.60e-07 | 7.71e-06 | 151.06     | 1.3%
---------------------------------------------
dec_avg_lat: 7.7697e-07 seconds, dec_thr: 4.1185e+01 Mbps
Time taken for SNR 9.00000000 dB: 1.63841297 seconds
Average time per frame: 1.6293e-05 seconds
rep-soft: Bit Errors = 100, Total Frames = 100559 Frame Errors = 100, BER = 3.1076e-05, FER = 9.9444e-04, Sim_thr = 1.9640e+06
```
we have slightly better simulation throughput but for the decoding part the variation is only due to the randomness of the simulation so no significant imporvent have been noticed.
### mod all ones


