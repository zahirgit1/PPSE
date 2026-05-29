#!/bin/bash

# Ensure the simulator has execution permissions
chmod +x ./simulator

echo "Starting Simulation Batch..."

# Sim 1: Hard Decoding (N=128)
echo "Running Sim 1 (Hard8, N=128)..."
./simulator -m 0 -M 14 -s 1 -e 100 -K 32 -N 128 -D "rep-hard8" --qf 0 --qs 1
mv ../simulations/sim.csv ../simulations/sim1_8.csv

# Sim 2: Soft Decoding (N=128)
echo "Running Sim 2 (Soft8, N=128)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 128 -D "rep-soft8" --qf 2 --qs 4
mv ../simulations/sim.csv ../simulations/sim2_8.csv

# Sim 3: Soft Decoding (N=96)
echo "Running Sim 3 (Soft8, N=96)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 96 -D "rep-soft8" --qf 3 --qs 4
mv ../simulations/sim.csv ../simulations/sim3_8.csv

# Sim 4: Soft Decoding (N=64)
echo "Running Sim 4 (Soft8, N=64)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 64 -D "rep-soft8" --qf 2 --qs 4
mv ../simulations/sim.csv ../simulations/sim4_8.csv

# Sim 5: Soft Decoding (N=32)
echo "Running Sim 5 (Soft8, N=32)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 32 -D "rep-soft8" --qf 0 --qs 1
mv ../simulations/sim.csv ../simulations/sim5_8.csv

echo "All simulations complete. Check .csv files."