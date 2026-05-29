type cmake .. then make 
cmake -DUSE_STATS=ON .. (for the stats on or else OFF)
make

---

SIMULATOR FLAGS USED:

Basic Configuration:
  -m <int>          : Start SNR value (e.g., -m 0 for 0 dB)
  -M <int>          : End SNR value (e.g., -M 12 for 12 dB)
  -s <int>          : SNR step size (e.g., -s 1 for 1 dB steps)
  -e <int>          : Number of errors per SNR point (e.g., -e 100)
  -K <int>          : Block size in bytes (e.g., -K 256)
  -N <int>          : Number of blocks (e.g., -N 1024)
  -D <string>       : Decoder type (see DECODER TYPES below)

DECODER TYPES:

  rep-hard          : Hard input decoder (baseline)
  rep-soft          : Soft input decoder with floating-point precision
  rep-hard8         : Hard input decoder with 8-bit quantization
  rep-soft8         : Soft input decoder with 8-bit quantization
  rep-hard8-neon    : Hard input decoder with 8-bit quantization + NEON optimization
  rep-soft8-neon    : Soft input decoder with 8-bit quantization + NEON optimization

Random Number Generator:
  --mersenne        : Use Mersenne Twister RNG
  (without flag)    : Uses baseline RNG

Optimization Flags:
  --neonmod         : Use NEON-optimized modulator
  --neondemod       : Use NEON-optimized demodulator
  --neonenc         : Use NEON-optimized encoder
  --neonmon         : Use NEON-optimized monitor
  --muller          : Enable Muller soft decision algorithm

Quantization:
  --qf <int>        : Quantization factor (e.g., --qf 5)
  --qs <int>        : Quantization size in bits (e.g., --qs 8 for 8-bit)

Example Simulation Commands:

Baseline (no optimizations):
./simulator -m 0 -M 12 -s 1 -e 100 -K 256 -N 1024 -D "rep-soft8-neon" > sim_baseline.csv

Mersenne32 (Mersenne Twister):
./simulator -m 0 -M 12 -s 1 -e 100 -K 256 -N 1024 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8 > sim_mersenne32.csv

Mersenne256 (Mersenne Twister with 256-bit):
./simulator -m 0 -M 12 -s 1 -e 100 -K 256 -N 1024 -D "rep-soft8-neon" --mersenne --neonmod --neonmon --neondemod --neonenc --muller --qf 5 --qs 8 > sim_mersenne256.csv

