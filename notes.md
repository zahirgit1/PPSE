# Notes pour plustard
## rand :
generates a random int number to have the value in the correct bracket(0,N) we need 
to devise by /(N+1) 



## cmake : for now auto generated
 * Very important to include the header files or else without a prototype functions are undifined behavior adn can cause interpretations problem example floats were promoted to integers because the compiler had no prototype 
## Gaussien white noise : 
Found two methods on the internet : https://www.qualityassignmenthelp.com/wp-content/uploads/2017/03/Gaussian-noise_How-to-Generate-.pdf

Central Limit Thorem Method :
```
X=0
for i = 1 to N
U = uniform()
X = X + U
end
/* for uniform randoms in [0,1], mu = 0.5 and var = 1/12 */
/* adjust X so mu = 0 and var = 1 */
X = X - N/2 /* set mean to 0 */
X = X * sqrt(12 / N) /* adjust variance to 1 */
```

simpler to code but its an approximations
Box-Muller Method :
```
do                  /* if U1==0, then the log() below will blow up */
U1=uniform()
while U1==0
U2=uniform()
X=sqrt(-2 * log(U1)) * cos(2pi * U2)
Y=sqrt(-2 * log(U1)) * sin(2pi * U2)
```
heavier on the calculation but mathematically exact 

## time-dating :
* will be using clock_gettime rather than clock stable and more precise in longer simulations
* throughput calculation for each block is conducted by a macro called MEASURE_BLOCK 
## MEASURE_BLOCK :
* struct : total time spent on the called function ,minimum and maximum latency thats updated each snr point, number of bits output of the function, and the name of the called function
* init_stat 
* updating the stats
## Quantization module
the logic behind is to not just cast float to int8 because we will lose information, example 0.75 will be 0 if converted directly to int8 (L8_N[i] = (int8_t)L_N[i];)
// transform numbers from floating-point representation to fixed-point representation
// `s` is the number of bits used in the quantizer block
// `f` is the number of bits of the fractional part (`s` >= `f`)
* f represents the step its the factor  (2^f) so if we take our example and we want to transfer our float 0.75 to int8 our value will be 0 75*2^f, knowing the f we can recover our initial float information.

* this operation is done on int32 before being converted to int8 to convert it we must use roundf to the scaled float to round to the nearest integer then we cast from int32 to int8 (of course we need to cap it at our max and min values designated my the argument s or else we will have an overflow)
* the goal here is to find the best pair of s and f that outputs the best throughput
## Best pairs :
* sim1 =(0,1) hard decode only needs the sign.
* sim2 =(2,4) needs more precision .
* sim3 =(3,4) . 
* sim4 = (3,4) .
* sim5 = (0,1) acts like a hard decoder
*for the final soft decode simulation i will be using s = 6 f = 1 for the FER to be close to each other 
For the neon decoder we aRE LIMITED BY THE LOSS of precision of the int8 if we use accumulative adding by vector(check the decode file its the technique i used to callculate the sun of mu;itple arrays) using only the instructions given i cannot prevent further loss we will use s- 5 and f = 0 for the neon decoder 

  