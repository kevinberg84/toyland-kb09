"""Clip-style waveform comparison of the exact two offline audition excerpts."""
from pathlib import Path
import sys
import json

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / '.tmp/dsp-analysis-packages'))
import numpy as np
from scipy.io import wavfile
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator, FuncFormatter

OUT = ROOT / 'artifacts/AnalogInvestigation/LimiterPrototypes'
rate, audio = wavfile.read(OUT / 'audition-current-then-prototype.wav')
assert rate == 96000 and audio.ndim == 1 and len(audio) == 1200000
assert np.isfinite(audio).all()
clips = [audio[:6 * rate], audio[int(6.5 * rate):]]
assert all(len(clip) == 6 * rate for clip in clips)
assert np.count_nonzero(audio[6 * rate:int(6.5 * rate)]) == 0

plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 12})
fig, axes = plt.subplots(1, 2, figsize=(15, 4.5), sharex=True, sharey=True)
fig.patch.set_facecolor('#292b2d')
fig.subplots_adjust(left=.065, right=.985, top=.75, bottom=.22, wspace=.09)
fig.text(.065, .93, 'CURRENT KB-09  /  PROPOSED FILTER', color='#f4f4f1',
         fontsize=20, weight='bold')
fig.text(.065, .855, 'The same six-second sweep excerpt from the audio comparison',
         color='#d0d2d3', fontsize=12)

titles = ['Current KB-09 equations', 'Proposed headroom + resonance limiter']
statistics = []
for ax, clip, title in zip(axes, clips, titles):
    # Extrema preserve short peaks that sample skipping would hide.
    samples_per_bin = 320
    bins = clip.reshape(-1, samples_per_bin)
    time = (np.arange(len(bins)) + .5) * samples_per_bin / rate
    low, high = bins.min(axis=1), bins.max(axis=1)
    ax.set_facecolor('#ff373a')
    ax.fill_between(time, low, high, color='#26090a', linewidth=0, zorder=2)
    ax.axhline(0, color='#26090a', linewidth=.5, alpha=.6, zorder=3)
    ax.set_xlim(0, 6)
    ax.set_ylim(-.5, .5)
    ax.set_yticks([-.5, -.25, 0, .25, .5])
    ax.xaxis.set_major_locator(MultipleLocator(1))
    ax.xaxis.set_major_formatter(FuncFormatter(lambda value, _: f'{value:g}s'))
    ax.grid(axis='x', color='#6a1517', alpha=.4, linewidth=1, zorder=1)
    ax.tick_params(colors='#dedfdf', labelsize=10, length=0, pad=8)
    for spine in ax.spines.values():
        spine.set_color('#202123')
        spine.set_linewidth(1.5)
    # Separate clip-name strip above the waveform, as in a DAW clip view.
    ax.add_patch(plt.Rectangle((0, 1.015), 1, .17, transform=ax.transAxes,
                              color='#ff373a', clip_on=False, linewidth=0))
    ax.text(.018, 1.10, title, transform=ax.transAxes, va='center',
            color='#26090a', fontsize=12, weight='bold')
    statistics.append({'clip': title, 'samples': len(clip),
                       'peak': float(np.max(np.abs(clip))),
                       'rms': float(np.sqrt(np.mean(clip.astype(float) ** 2)))})
axes[0].set_ylabel('Amplitude', color='#dedfdf', fontsize=11)
fig.text(.065, .09, 'Identical time and amplitude scales. Fixed calibration gains retained; no peak normalization or automatic leveling.',
         color='#d0d2d3', fontsize=10)
fig.text(.065, .035, 'Offline diagnostic models — the proposed filter is not installed in the plugin.',
         color='#a6a9ab', fontsize=10)
fig.savefig(OUT / 'current-vs-proposed-waveforms.png', dpi=160,
            facecolor=fig.get_facecolor())
fig.savefig(OUT / 'current-vs-proposed-waveforms.svg', facecolor=fig.get_facecolor())
plt.close(fig)
(OUT / 'waveform-comparison-notes.json').write_text(json.dumps({
    'source': 'audition-current-then-prototype.wav',
    'rate': rate, 'duration_each_seconds': 6,
    'waveform_display': 'Minimum and maximum within non-overlapping 320-sample bins; both use +/-0.5 amplitude limits.',
    'gain': 'Unmodified source audition excerpts; no additional normalization.',
    'clips': statistics,
}, indent=2))
print(OUT / 'current-vs-proposed-waveforms.png')
