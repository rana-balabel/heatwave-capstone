import pandas as pd

df = pd.read_csv("AI/Datasets/Balanced_Dataset_75_25.csv")

positive = df[df["Heat stroke"] == 1]
negative = df[df["Heat stroke"] == 0]
indoor = df[df["Exposure to sun"] == 0]
indoor_positive = indoor[indoor["Heat stroke"] == 1]
indoor_negative = indoor[indoor["Heat stroke"] == 0]

hr_pos = positive["Heart rate"]
env_pos = positive["Environmental temperature"]
body_pos = positive["Body temperature"]
humidity_pos = positive["Relative Humidity"]

print("=" * 55)
print("         DATASET STATISTICS SUMMARY")
print("=" * 55)

print(f"\n--- Overview ---")
print(f"  Total records            : {len(df)}")
print(f"  Total features           : {len(df.columns) - 1}")
print(f"  Missing values           : {df.isnull().sum().sum()}")

print(f"\n--- Heatstroke Labels ---")
print(f"  Positive cases (1)       : {len(positive)}  ({len(positive)/len(df)*100:.1f}%)")
print(f"  Negative cases (0)       : {len(negative)}  ({len(negative)/len(df)*100:.1f}%)")

print(f"\n--- Indoor Cases (Exposure to sun == 0) ---")
print(f"  Total indoor             : {len(indoor)}")
print(f"  Indoor positive          : {len(indoor_positive)}")
print(f"  Indoor negative          : {len(indoor_negative)}")

print(f"\n--- Heart Rate (Heatstroke Positive) ---")
print(f"  Average BPM              : {hr_pos.mean():.2f}")
print(f"  Max BPM                  : {hr_pos.max():.2f}")
print(f"  Min BPM                  : {hr_pos.min():.2f}")
print(f"  Std Dev BPM              : {hr_pos.std():.2f}")

print(f"\n--- Heart Rate (Heatstroke Negative) ---")
hr_neg = negative["Heart rate"]
print(f"  Average BPM              : {hr_neg.mean():.2f}")
print(f"  Max BPM                  : {hr_neg.max():.2f}")
print(f"  Min BPM                  : {hr_neg.min():.2f}")

print(f"\n--- Environmental Temperature ---")
print(f"  Positive avg             : {env_pos.mean():.2f} °C")
print(f"  Negative avg             : {negative['Environmental temperature'].mean():.2f} °C")
print(f"  Overall range            : {df['Environmental temperature'].min():.1f} – {df['Environmental temperature'].max():.1f} °C")

print(f"\n--- Body Temperature ---")
print(f"  Positive avg             : {body_pos.mean():.2f} °C")
print(f"  Negative avg             : {negative['Body temperature'].mean():.2f} °C")
print(f"  Positive range           : {body_pos.min():.1f} – {body_pos.max():.1f} °C")

print(f"\n--- Relative Humidity ---")
print(f"  Overall avg              : {df['Relative Humidity'].mean():.3f}")
print(f"  Positive avg             : {humidity_pos.mean():.3f}")
print(f"  Negative avg             : {negative['Relative Humidity'].mean():.3f}")

print(f"\n--- Sun Exposure ---")
print(f"  Positive avg exposure    : {positive['Exposure to sun'].mean():.2f}")
print(f"  Negative avg exposure    : {negative['Exposure to sun'].mean():.2f}")
print(f"  Max exposure (overall)   : {df['Exposure to sun'].max():.1f}")

print(f"\n--- Feature Correlations with Heat Stroke ---")
corr = df.corr(numeric_only=True)['Heat stroke'].drop('Heat stroke').sort_values(ascending=False)
for feature, val in corr.items():
    print(f"  {feature:<30} : {val:.3f}")

print("=" * 55)
