import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Define file paths for last 3 simulations across all variants
variants = {
    'baseline': {3: 'sim3.csv', 4: 'sim4.csv', 5: 'sim5.csv'},
    'mersenne32': {3: 'sim3_neon_mersenne32.csv', 4: 'sim4_neon_mersenne32.csv', 5: 'sim5_neon_mersenne32.csv'},
    'mersenne256': {3: 'sim3_neon_mersenne256.csv', 4: 'sim4_neon_mersenne256.csv', 5: 'sim5_neon_mersenne256.csv'}
}

# Load data for all variants
data = {}
for variant, files in variants.items():
    data[variant] = {}
    for sim_num, filename in files.items():
        data[variant][sim_num] = pd.read_csv(filename)

def apply_common_styling(ax, xlabel, ylabel, title):
    ax.set_yscale('log')
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title, loc='left', fontsize=14, color='#003366', pad=15)
    ax.grid(True, which="both", linestyle='-', alpha=0.4)
    ax.legend(fontsize=9)

# Color palette for simulations
colors_sim = {3: 'tab:red', 4: 'orange', 5: 'darkviolet'}
rates = {3: '1/3', 4: '1/2', 5: '1'}

# ========== COMPARISON 1: BASELINE VS MERSENNE32 ==========

# Line styles for Baseline vs Mersenne32
line_styles_comp1 = {
    'baseline': '-',
    'mersenne32': '--'
}

markers_comp1 = {
    'baseline': 'o',
    'mersenne32': 's'
}

# --- Plot 1a: Code Rates (Eb/N0) - Baseline vs Mersenne32 ---
fig1a, ax1a = plt.subplots(figsize=(12, 7))

for variant in ['baseline', 'mersenne32']:
    for sim_num in [3, 4, 5]:
        df = data[variant][sim_num]
        label = f'Sim{sim_num} (R={rates[sim_num]}) - {variant.replace("_", " ").title()}'
        ax1a.plot(df['Eb/N0(dB)'], df['FER'], 
                marker=markers_comp1[variant], 
                linestyle=line_styles_comp1[variant],
                color=colors_sim[sim_num], 
                label=label,
                markersize=5,
                alpha=0.8)

apply_common_styling(ax1a, r'Signal-to-Noise Ratio ($E_b/N_0$ (dB))', 'Frame Error Rate', 
                     r'Code Rates: Baseline vs Mersenne32 ($E_b/N_0$)')

plt.savefig('code_rates_ebn0_baseline_vs_m32.png', dpi=300, bbox_inches='tight')
print("Saved: code_rates_ebn0_baseline_vs_m32.png")

# --- Plot 1b: Code Rates (Es/N0) - Baseline vs Mersenne32 ---
fig1b, ax1b = plt.subplots(figsize=(12, 7))

for variant in ['baseline', 'mersenne32']:
    for sim_num in [3, 4, 5]:
        df = data[variant][sim_num]
        label = f'Sim{sim_num} (R={rates[sim_num]}) - {variant.replace("_", " ").title()}'
        ax1b.plot(df['Es/N0(dB)'], df['FER'], 
                marker=markers_comp1[variant], 
                linestyle=line_styles_comp1[variant],
                color=colors_sim[sim_num], 
                label=label,
                markersize=5,
                alpha=0.8)

apply_common_styling(ax1b, r'Signal-to-Noise Ratio ($E_s/N_0$ (dB))', 'Frame Error Rate', 
                     r'Code Rates: Baseline vs Mersenne32 ($E_s/N_0$)')

plt.savefig('code_rates_esn0_baseline_vs_m32.png', dpi=300, bbox_inches='tight')
print("Saved: code_rates_esn0_baseline_vs_m32.png")

# --- Plot 1c: Hard vs Soft Decoder - Baseline vs Mersenne32 ---
fig1c, ax1c = plt.subplots(figsize=(12, 7))

for variant in ['baseline', 'mersenne32']:
    # Sim3 - Soft decoder (1/3 rate)
    df3 = data[variant][3]
    ax1c.plot(df3['Eb/N0(dB)'], df3['BER'], 
            marker=markers_comp1[variant], 
            linestyle=line_styles_comp1[variant],
            color='darkgreen', 
            label=f'Sim3 BER (Soft) - {variant.replace("_", " ").title()}',
            markersize=5,
            alpha=0.8)
    ax1c.plot(df3['Eb/N0(dB)'], df3['FER'], 
            marker=markers_comp1[variant], 
            linestyle=line_styles_comp1[variant],
            color='darkgreen', 
            label=f'Sim3 FER (Soft) - {variant.replace("_", " ").title()}',
            markersize=5,
            alpha=0.8,
            markerfacecolor='none')
    
    # Sim4 - Hard decoder (1/2 rate) 
    df4 = data[variant][4]
    ax1c.plot(df4['Eb/N0(dB)'], df4['BER'], 
            marker=markers_comp1[variant], 
            linestyle=line_styles_comp1[variant],
            color='darkblue', 
            label=f'Sim4 BER (Hard) - {variant.replace("_", " ").title()}',
            markersize=5,
            alpha=0.8)
    ax1c.plot(df4['Eb/N0(dB)'], df4['FER'], 
            marker=markers_comp1[variant], 
            linestyle=line_styles_comp1[variant],
            color='darkblue', 
            label=f'Sim4 FER (Hard) - {variant.replace("_", " ").title()}',
            markersize=5,
            alpha=0.8,
            markerfacecolor='none')

apply_common_styling(ax1c, r'Signal-to-Noise Ratio ($E_b/N_0$ (dB))', 'Bit/Frame Error Rate', 
                     'Hard vs Soft Decoder: Baseline vs Mersenne32')

plt.savefig('hardvssoft_baseline_vs_m32.png', dpi=300, bbox_inches='tight')
print("Saved: hardvssoft_baseline_vs_m32.png")

# ========== COMPARISON 2: MERSENNE32 VS MERSENNE256 ==========

# Line styles for Mersenne32 vs Mersenne256
line_styles_comp2 = {
    'mersenne32': '-',
    'mersenne256': '--'
}

markers_comp2 = {
    'mersenne32': 'o',
    'mersenne256': 's'
}

# --- Plot 2a: Code Rates (Eb/N0) - Mersenne32 vs Mersenne256 ---
fig2a, ax2a = plt.subplots(figsize=(12, 7))

for variant in ['mersenne32', 'mersenne256']:
    for sim_num in [3, 4, 5]:
        df = data[variant][sim_num]
        label = f'Sim{sim_num} (R={rates[sim_num]}) - {variant.replace("_", " ").title()}'
        ax2a.plot(df['Eb/N0(dB)'], df['FER'], 
                marker=markers_comp2[variant], 
                linestyle=line_styles_comp2[variant],
                color=colors_sim[sim_num], 
                label=label,
                markersize=5,
                alpha=0.8)

apply_common_styling(ax2a, r'Signal-to-Noise Ratio ($E_b/N_0$ (dB))', 'Frame Error Rate', 
                     r'Code Rates: Mersenne32 vs Mersenne256 ($E_b/N_0$)')

plt.savefig('code_rates_ebn0_m32_vs_m256.png', dpi=300, bbox_inches='tight')
print("Saved: code_rates_ebn0_m32_vs_m256.png")

# --- Plot 2b: Code Rates (Es/N0) - Mersenne32 vs Mersenne256 ---
fig2b, ax2b = plt.subplots(figsize=(12, 7))

for variant in ['mersenne32', 'mersenne256']:
    for sim_num in [3, 4, 5]:
        df = data[variant][sim_num]
        label = f'Sim{sim_num} (R={rates[sim_num]}) - {variant.replace("_", " ").title()}'
        ax2b.plot(df['Es/N0(dB)'], df['FER'], 
                marker=markers_comp2[variant], 
                linestyle=line_styles_comp2[variant],
                color=colors_sim[sim_num], 
                label=label,
                markersize=5,
                alpha=0.8)

apply_common_styling(ax2b, r'Signal-to-Noise Ratio ($E_s/N_0$ (dB))', 'Frame Error Rate', 
                     r'Code Rates: Mersenne32 vs Mersenne256 ($E_s/N_0$)')

plt.savefig('code_rates_esn0_m32_vs_m256.png', dpi=300, bbox_inches='tight')
print("Saved: code_rates_esn0_m32_vs_m256.png")

# --- Plot 2c: Hard vs Soft Decoder - Mersenne32 vs Mersenne256 ---
fig2c, ax2c = plt.subplots(figsize=(12, 7))

for variant in ['mersenne32', 'mersenne256']:
    # Sim3 - Soft decoder (1/3 rate)
    df3 = data[variant][3]
    ax2c.plot(df3['Eb/N0(dB)'], df3['BER'], 
            marker=markers_comp2[variant], 
            linestyle=line_styles_comp2[variant],
            color='darkgreen', 
            label=f'Sim3 BER (Soft) - {variant.replace("_", " ").title()}',
            markersize=5,
            alpha=0.8)
    ax2c.plot(df3['Eb/N0(dB)'], df3['FER'], 
            marker=markers_comp2[variant], 
            linestyle=line_styles_comp2[variant],
            color='darkgreen', 
            label=f'Sim3 FER (Soft) - {variant.replace("_", " ").title()}',
            markersize=5,
            alpha=0.8,
            markerfacecolor='none')
    
    # Sim4 - Hard decoder (1/2 rate) 
    df4 = data[variant][4]
    ax2c.plot(df4['Eb/N0(dB)'], df4['BER'], 
            marker=markers_comp2[variant], 
            linestyle=line_styles_comp2[variant],
            color='darkblue', 
            label=f'Sim4 BER (Hard) - {variant.replace("_", " ").title()}',
            markersize=5,
            alpha=0.8)
    ax2c.plot(df4['Eb/N0(dB)'], df4['FER'], 
            marker=markers_comp2[variant], 
            linestyle=line_styles_comp2[variant],
            color='darkblue', 
            label=f'Sim4 FER (Hard) - {variant.replace("_", " ").title()}',
            markersize=5,
            alpha=0.8,
            markerfacecolor='none')

apply_common_styling(ax2c, r'Signal-to-Noise Ratio ($E_b/N_0$ (dB))', 'Bit/Frame Error Rate', 
                     'Hard vs Soft Decoder: Mersenne32 vs Mersenne256')

plt.savefig('hardvssoft_m32_vs_m256.png', dpi=300, bbox_inches='tight')
print("Saved: hardvssoft_m32_vs_m256.png")

plt.close('all')
print("\nAll comparison plots generated successfully!")
print("\nComparison 1 (Baseline vs Mersenne32):")
print("  - code_rates_ebn0_baseline_vs_m32.png")
print("  - code_rates_esn0_baseline_vs_m32.png")
print("  - hardvssoft_baseline_vs_m32.png")
print("\nComparison 2 (Mersenne32 vs Mersenne256):")
print("  - code_rates_ebn0_m32_vs_m256.png")
print("  - code_rates_esn0_m32_vs_m256.png")
print("  - hardvssoft_m32_vs_m256.png")
