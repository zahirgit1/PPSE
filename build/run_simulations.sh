#!/bin/bash

# Ensure the simulator has execution permissions
chmod +x ./simulator

echo "Starting Simulation Batch..."

# Sim 1: Hard Decoding (N=128)
echo "Running Sim 1 (Hard, N=128)..."
./simulator -m 0 -M 15 -s 1 -e 100 -K 32 -N 128 -D "rep-hard"
mv ../simulations/sim.csv ../simulations/sim1.csv

# Sim 2: Soft Decoding (N=128)
echo "Running Sim 2 (Soft, N=128)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 128 -D "rep-soft"
mv ../simulations/sim.csv ../simulations/sim2.csv

# Sim 3: Soft Decoding (N=96)
echo "Running Sim 3 (Soft, N=96)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 96 -D "rep-soft"
mv ../simulations/sim.csv ../simulations/sim3.csv

# Sim 4: Soft Decoding (N=64)
echo "Running Sim 4 (Soft, N=64)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 64 -D "rep-soft"
mv ../simulations/sim.csv ../simulations/sim4.csv

# Sim 5: Soft Decoding (N=32)
echo "Running Sim 5 (Soft, N=32)..."
./simulator -m 0 -M 12 -s 1 -e 100 -K 32 -N 32 -D "rep-soft"
mv ../simulations/sim.csv ../simulations/sim5.csv

echo "All simulations complete. Check your .csv files."