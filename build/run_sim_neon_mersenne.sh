#!/bin/bash

# Ensure the simulator has execution permissions
chmod +x ./simulator

echo "Starting NEON + Mersenne Optimized Simulation Batch..."

# Sim 1: hard8 Decoding (N=1024) - NEON optimized with Mersenne
echo "Running Sim 1 (Hard8-NEON, N=1024) with Mersenne..."
./simulator -m 0 -M 15 -s 1 -e 100 -K 256 -N 1024 -D "rep-hard8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 0 --qs 1
mv ../simulations/sim.csv ../simulations/sim1_neon_mersenne256.csv
    
# Sim 2: Soft8 Decoding (N=1024) - NEON optimized with Mersenne
echo "Running Sim 2 (Soft8-NEON, N=1024) with Mersenne..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 256 -N 1024 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8
mv ../simulations/sim.csv ../simulations/sim2_neon_mersenne256.csv

# Sim 3: Soft8 Decoding (N=768) - NEON optimized with Mersenne
echo "Running Sim 3 (Soft8-NEON, N=768) with Mersenne..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 256 -N 768 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8
mv ../simulations/sim.csv ../simulations/sim3_neon_mersenne256.csv

# Sim 4: Soft8 Decoding (N=512) - NEON optimized with Mersenne
echo "Running Sim 4 (Soft8-NEON, N=512) with Mersenne..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 256 -N 512 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8
mv ../simulations/sim.csv ../simulations/sim4_neon_mersenne256.csv

# Sim 5: Soft8 Decoding (N=256) - NEON optimized with Mersenne
echo "Running Sim 5 (Soft8-NEON, N=256) with Mersenne..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 256 -N 256 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8
mv ../simulations/sim.csv ../simulations/sim5_neon_mersenne256.csv

# Sim 1: hard8 Decoding (N=128) - NEON optimized with Mersenne
echo "Running Sim 1 (Hard8-NEON, N=128) with Mersenne..."
./simulator -m 0 -M 15 -s 1 -e 100 -K 32 -N 128 -D "rep-hard8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 0 --qs 1
mv ../simulations/sim.csv ../simulations/sim1_neon_mersenne32.csv
    
# Sim 2: Soft8 Decoding (N=128) - NEON optimized with Mersenne
echo "Running Sim 2 (Soft8-NEON, N=128) with Mersenne..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 128 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8
mv ../simulations/sim.csv ../simulations/sim2_neon_mersenne32.csv

# Sim 3: Soft8 Decoding (N=96) - NEON optimized with Mersenne
echo "Running Sim 3 (Soft8-NEON, N=96) with Mersenne..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 96 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8
mv ../simulations/sim.csv ../simulations/sim3_neon_mersenne32.csv

# Sim 4: Soft8 Decoding (N=64) - NEON optimized with Mersenne
echo "Running Sim 4 (Soft8-NEON, N=64) with Mersenne..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 64 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8
mv ../simulations/sim.csv ../simulations/sim4_neon_mersenne32.csv

# Sim 5: Soft8 Decoding (N=32) - NEON optimized with Mersenne
echo "Running Sim 5 (Soft8-NEON, N=32) with Mersenne..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 32 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8
mv ../simulations/sim.csv ../simulations/sim5_neon_mersenne32.csv
echo "All simulations completed with NEON + Mersenne optimizations!"

# Sim 1: Hard Decoding (N=128)
echo "Running Sim 1 (Hard, N=128)..."
./simulator -m 0 -M 15 -s 1 -e 100 -K 32 -N 128 -D "rep-hard" 
mv ../simulations/sim.csv ../simulations/sim1_baseline.csv

# Sim 2: Soft Decoding (N=128)
echo "Running Sim 2 (Soft, N=128)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 128 -D "rep-soft" 
mv ../simulations/sim.csv ../simulations/sim2_baseline.csv

# Sim 3: Soft Decoding (N=96)
echo "Running Sim 3 (Soft, N=96)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 96 -D "rep-soft" 
mv ../simulations/sim.csv ../simulations/sim3_baseline.csv

# Sim 4: Soft Decoding (N=64)
echo "Running Sim 4 (Soft, N=64)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 64 -D "rep-soft" 
mv ../simulations/sim.csv ../simulations/sim4_baseline.csv

# Sim 5: Soft Decoding (N=32)
echo "Running Sim 5 (Soft, N=32)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 32 -D "rep-soft"
mv ../simulations/sim.csv ../simulations/sim5_baseline.csv

echo "All simulations completed!"
