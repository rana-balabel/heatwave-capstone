"""
Symposium Poster Analysis
Heat Stroke Prediction MLP — Graphs & Results
"""

import os
import pandas as pd
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap

import tensorflow as tf
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import (
    roc_auc_score, roc_curve,
    confusion_matrix, classification_report,
    precision_recall_curve, average_precision_score,
)

# ── Style  (light poster — UW orange theme) ────────────────────────────────────
POSTER_BG   = "#FFFFFF"
PANEL_BG    = "#F7F7F7"
ACCENT_ORG  = "#E87722"   # UW orange
ACCENT_BLUE = "#1A5276"   # deep navy
ACCENT_GRAY = "#5D6D7E"
TEXT_DARK   = "#1A1A1A"
GRID_COLOR  = "#E0E0E0"
BORDER_CLR  = "#E87722"

plt.rcParams.update({
    "figure.facecolor":  POSTER_BG,
    "axes.facecolor":    PANEL_BG,
    "axes.edgecolor":    BORDER_CLR,
    "axes.labelcolor":   TEXT_DARK,
    "axes.titlecolor":   TEXT_DARK,
    "xtick.color":       ACCENT_GRAY,
    "ytick.color":       ACCENT_GRAY,
    "text.color":        TEXT_DARK,
    "grid.color":        GRID_COLOR,
    "grid.linewidth":    0.8,
    "font.family":       "DejaVu Sans",
    "font.size":         11,
    "axes.titlesize":    13,
    "axes.labelsize":    11,
    "axes.titleweight":  "bold",
    "legend.facecolor":  POSTER_BG,
    "legend.edgecolor":  GRID_COLOR,
    "legend.fontsize":   10,
})

OUT_DIR = "AI/poster_figures"
os.makedirs(OUT_DIR, exist_ok=True)

# ── Data & Model ───────────────────────────────────────────────────────────────
df = pd.read_csv("AI/Datasets/Balanced_Dataset_Fixed.csv")
FEATURES = ['Environmental temperature', 'Body temperature',
            'Relative Humidity', 'Exposure to sun', 'Heart rate']
X = df[FEATURES]
Y = df['Heat stroke']

X_train, X_test, y_train, y_test = train_test_split(
    X, Y, test_size=0.2, random_state=42)

scaler = StandardScaler()
X_train_s = scaler.fit_transform(X_train)
X_test_s  = scaler.transform(X_test)

y_train_smoothed = y_train * 0.9 + 0.05

model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(5,)),
    tf.keras.layers.Dense(
        10, activation='sigmoid',
        kernel_regularizer=tf.keras.regularizers.l2(0.01)),
    tf.keras.layers.Dropout(0.3),
    tf.keras.layers.Dense(1, activation='sigmoid'),
])
model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
early_stop = tf.keras.callbacks.EarlyStopping(
    monitor='val_loss', patience=5, restore_best_weights=True)

history = model.fit(
    X_train_s, y_train_smoothed,
    epochs=100, batch_size=8,
    validation_split=0.2, verbose=0,
    callbacks=[early_stop],
)

# ── Predictions & Metrics ──────────────────────────────────────────────────────
train_probs = model.predict(X_train_s, verbose=0).flatten()
test_probs  = model.predict(X_test_s,  verbose=0).flatten()
test_preds  = (test_probs >= 0.5).astype(int)

train_acc = np.mean((train_probs >= 0.5).astype(int) == y_train.to_numpy())
test_acc  = np.mean(test_preds == y_test.to_numpy())
auc       = roc_auc_score(y_test, test_probs)
ap        = average_precision_score(y_test, test_probs)

report    = classification_report(y_test, test_preds, target_names=["No Stroke", "Heat Stroke"],
                                   output_dict=True)
precision = report["Heat Stroke"]["precision"]
recall    = report["Heat Stroke"]["recall"]
f1        = report["Heat Stroke"]["f1-score"]

print(f"Train Accuracy : {train_acc:.4f}")
print(f"Test  Accuracy : {test_acc:.4f}")
print(f"AUC-ROC        : {auc:.4f}")
print(f"Avg Precision  : {ap:.4f}")
print(f"Precision      : {precision:.4f}")
print(f"Recall         : {recall:.4f}")
print(f"F1 Score       : {f1:.4f}")

# ═══════════════════════════════════════════════════════════════════════════════
# 1. Feature Distributions  (positive vs negative)
# ═══════════════════════════════════════════════════════════════════════════════
short_labels = {
    'Environmental temperature': 'Env. Temp',
    'Body temperature':          'Body Temp',
    'Relative Humidity':         'Humidity',
    'Exposure to sun':           'Sun Exposure',
    'Heart rate':                'Heart Rate',
}

pos_df = df[df['Heat stroke'] == 1]
neg_df = df[df['Heat stroke'] == 0]

fig, axes = plt.subplots(1, 5, figsize=(18, 4))
fig.patch.set_facecolor(POSTER_BG)

handles = [
    plt.Rectangle((0, 0), 1, 1, fc=ACCENT_BLUE, alpha=0.7),
    plt.Rectangle((0, 0), 1, 1, fc=ACCENT_ORG,  alpha=0.7),
]

for ax, feat in zip(axes, FEATURES):
    ax.hist(neg_df[feat], bins=25, color=ACCENT_BLUE, alpha=0.7,
            density=True, edgecolor='white', linewidth=0.3)
    ax.hist(pos_df[feat], bins=25, color=ACCENT_ORG,  alpha=0.7,
            density=True, edgecolor='white', linewidth=0.3)
    ax.set_title(short_labels.get(feat, feat), fontsize=10)
    ax.set_ylabel('Density' if feat == FEATURES[0] else '')
    ax.grid(True, alpha=0.5)
    ax.set_facecolor(PANEL_BG)
    for spine in ax.spines.values():
        spine.set_edgecolor(BORDER_CLR)

# Place legend inside the last subplot (Heart Rate) at upper left to avoid overlap
axes[-1].legend(handles, ['No Stroke', 'Heat Stroke'],
                loc='upper left', fontsize=9, framealpha=0.8)

plt.tight_layout()
fig.savefig(f"{OUT_DIR}/1_feature_distributions.png", dpi=200,
            bbox_inches='tight', facecolor=POSTER_BG)
plt.close()
print("Saved: 1_feature_distributions.png")

# ═══════════════════════════════════════════════════════════════════════════════
# 2. Summary Metrics Bar
# ═══════════════════════════════════════════════════════════════════════════════
metrics = {
    'Accuracy':  test_acc,
    'AUC-ROC':   auc,
    'Precision': precision,
    'Recall':    recall,
    'F1 Score':  f1,
}

fig, ax = plt.subplots(figsize=(7, 4))
bar_colors = [ACCENT_ORG, ACCENT_BLUE, ACCENT_BLUE, ACCENT_BLUE, ACCENT_BLUE]
bars = ax.bar(metrics.keys(), metrics.values(), color=bar_colors,
              width=0.55, edgecolor='white')
for bar, val in zip(bars, metrics.values()):
    ax.text(bar.get_x() + bar.get_width() / 2,
            bar.get_height() + 0.01,
            f'{val:.3f}', ha='center', va='bottom',
            fontsize=13, fontweight='bold', color=TEXT_DARK)
ax.set_ylim(0, 1.15)
ax.set_ylabel('Score')
ax.set_title('Model Performance Summary')
ax.grid(True, axis='y', alpha=0.5)
for spine in ax.spines.values():
    spine.set_edgecolor(BORDER_CLR)

fig.savefig(f"{OUT_DIR}/2_metrics_summary.png", dpi=200,
            bbox_inches='tight', facecolor=POSTER_BG)
plt.close()
print("Saved: 2_metrics_summary.png")

# ── Print report ────────────��──────────────────────────────────────────────────
print("\n" + "=" * 50)
print("   CLASSIFICATION REPORT")
print("=" * 50)
print(classification_report(y_test, test_preds,
                             target_names=["No Stroke", "Heat Stroke"]))
print(f"AUC-ROC          : {auc:.4f}")
print(f"Average Precision: {ap:.4f}")
print("=" * 50)
print(f"\nAll figures saved to: {OUT_DIR}/")
