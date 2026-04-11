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
  