import pandas as pd
import matplotlib.pyplot as plt


sim_files = {i: f'sim{i}.csv' for i in range(1, 6)}
data = {i: pd.read_csv(sim_files[i]) for i in range(1, 6)}

def apply_common_styling(ax, xlabel, ylabel, title):
    ax.set_yscale('log')
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title, loc='left', fontsize=14, color='#003366', pad=15)
    ax.grid(True, which="both", linestyle='-', alpha=0.4)
    ax.legend()

# --- Plot 1: Hard Input Decoder VS Soft Input Decoder ---
fig1, ax1 = plt.subplots(figsize=(10, 6))
ax1.plot(data[1]['Eb/N0(dB)'], data[1]['BER'], 'g--o', mfc='none', label='$Sim_1$ BER')
ax1.plot(data[1]['Eb/N0(dB)'], data[1]['FER'], 'g-x', label='$Sim_1$ FER')
ax1.plot(data[2]['Eb/N0(dB)'], data[2]['BER'], 'C0--o', mfc='none', label='$Sim_2$ BER')
ax1.plot(data[2]['Eb/N0(dB)'], data[2]['FER'], 'C0-x', label='$Sim_2$ FER')
apply_common_styling(ax1, r'Signal-to-Noise Ratio ($E_b/N_0$ (dB))', 'Frame/Bit Error Rate', 'Hard Input Decoder VS Soft Input Decoder')

plt.savefig('hard_vs_soft_decoder.png', dpi=300, bbox_inches='tight')
print("Saved: hard_vs_soft_decoder.png")

# --- Plot 2: Various Code Rates depending on Eb/N0 ---
fig2, ax2 = plt.subplots(figsize=(10, 6))
colors = {2: 'C0', 3: 'tab:red', 4: 'orange', 5: 'purple'}
rates = {2: '1/4', 3: '1/3', 4: '1/2', 5: '1'}

for i in range(2, 6):
    ax2.plot(data[i]['Eb/N0(dB)'], data[i]['FER'], marker='x', color=colors[i], label=f'$Sim_{i}$ (R={rates[i]})')

apply_common_styling(ax2, r'Signal-to-Noise Ratio ($E_b/N_0$ (dB))', 'Frame Error Rate', r'Various Code Rates depending on $E_b/N_0$')

plt.savefig('code_rates_ebn0.png', dpi=300, bbox_inches='tight')
print("Saved: code_rates_ebn0.png")

# --- Plot 3: Various Code Rates depending on Es/N0 ---
fig3, ax3 = plt.subplots(figsize=(10, 6))
for i in range(2, 6):
    ax3.plot(data[i]['Es/N0(dB)'], data[i]['FER'], marker='x', color=colors[i], label=f'$Sim_{i}$ (R={rates[i]})')

apply_common_styling(ax3, r'Signal-to-Noise Ratio ($E_s/N_0$ (dB))', 'Frame Error Rate', r'Various Code Rates depending on $E_s/N_0$')

plt.savefig('code_rates_esn0.png', dpi=300, bbox_inches='tight')
print("Saved: code_rates_esn0.png")

plt.close('all')