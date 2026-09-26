"""
Cold-Weather Dataset Augmentation
===================================
Source: Engineering ToolBox - U.S. Outdoor Design Temperature & Humidity
https://www.engineeringtoolbox.com/us-outdoor-design-temperature-humidity-d_296.html

All RH values are from the JANUARY column only.
RH is averaged across the three daily readings (7:30am, 1:30pm, 7:30pm).
Avg daily temp conversion: °C = (°F - 32) × 5/9
"""

import numpy as np
import pandas as pd

# Raw January data: (City, Avg daily temp °F, RH 7:30am%, RH 1:30pm%, RH 7:30pm%)
RAW_DATA = [
    ("Los Angeles, CA",    52.1, 63, 46, 51),
    ("Denver, CO",         31.9, 54, 37, 41),
    ("Wilmington, DE",     33.3, 77, 62, 70),
    ("Washington, DC",     37.2, 73, 56, 64),
    ("Chicago, IL",        25.0, 81, 70, 75),
    ("Indianapolis, IN",   29.1, 83, 72, 78),
    ("Louisville, KY",     35.2, 78, 68, 69),
    ("New Orleans, LA",    53.9, 85, 67, 73),
    ("Portland, ME",       23.4, 81, 65, 74),
    ("Boston, MA",         31.3, 72, 59, 67),
    ("Detroit, MI",        25.9, 82, 71, 77),
    ("Minneapolis, MN",    16.0, 82, 72, 75),
    ("St. Louis, MO",      32.3, 77, 65, 68),
    ("Omaha, NE",          24.9, 82, 68, 73),
    ("Reno, NV",           34.0, 82, 67, 54),
    ("Concord, NH",        22.2, 78, 60, 69),
    ("Atlantic City, NJ",  33.6, 79, 68, 74),
    ("Albuquerque, NM",    36.6, 68, 51, 46),
    ("New York, NY",       33.8, 72, 61, 66),
    ("Bismarck, ND",       12.2, 77, 71, 75),
    ("Cleveland, OH",      28.0, 81, 72, 79),
    ("Oklahoma City, OK",  38.9, 79, 62, 65),
    ("Portland, OR",       41.0, 87, 82, 78),
    ("Pittsburgh, PA",     29.0, 77, 67, 63),
    ("Providence, RI",     29.9, 73, 60, 67),
    ("Houston, TX",        53.3, 85, 66, 73),
    ("Salt Lake City, UT", 31.0, 80, 71, 72),
    ("Burlington, VT",     19.1, 81, 69, 78),
    ("Richmond, VA",       38.5, 84, 60, 68),
    ("Seattle, WA",        41.0, 86, 80, 74),
    ("Charleston, WV",     35.1, 79, 64, 70),
    ("Milwaukee, WI",      22.3, 76, 70, 73),
    ("Cheyenne, WY",       28.2, 59, 48, 55),
]

# Convert to Celsius and compute daily average RH per city
_rows = []
for city, avg_f, rh_am, rh_pm, rh_eve in RAW_DATA:
    avg_c   = (avg_f - 32) * 5 / 9
    rh_avg  = (rh_am + rh_pm + rh_eve) / 3 / 100.0
    _rows.append({"avg_c": avg_c, "rh_avg": rh_avg})

_df = pd.DataFrame(_rows)

# Derive band means
_BANDS = [(-15, 0), (0, 10), (10, 20)]
COLD_RH_BY_BAND = {
    (lo, hi): round(_df[(_df["avg_c"] >= lo) & (_df["avg_c"] < hi)]["rh_avg"].mean(), 3)
    for lo, hi in _BANDS
}
print(f"Derived COLD_RH_BY_BAND: {COLD_RH_BY_BAND}")

ROWS_PER_CATEGORY = 300  # 300 rows per band → 900 cold rows total


def build_fixed_dataset(input_csv, output_csv, seed=42):
    """
    Loads the original dataset, removes physiologically invalid rows
    (body temp < 35°C), appends 300 cold-environment negative cases per
    temperature band, and saves the result to output_csv.
    """
    rng = np.random.default_rng(seed)

    df_orig = pd.read_csv(input_csv)
    df_clean = df_orig[df_orig["Body temperature"] >= 35].copy()
    removed = len(df_orig) - len(df_clean)

    cold_chunks = []
    for (lo, hi), rh_val in COLD_RH_BY_BAND.items():
        env_temps = rng.uniform(lo, hi, ROWS_PER_CATEGORY).round(1)
        chunk = pd.DataFrame({
            "Environmental temperature": env_temps,
            "Body temperature":          rng.uniform(36.0, 37.8, ROWS_PER_CATEGORY).round(1),
            "Relative Humidity":         rh_val,
            "Exposure to sun":           np.where(
                                             rng.random(ROWS_PER_CATEGORY) < 0.70,
                                             0.0,
                                             rng.uniform(1, 40, ROWS_PER_CATEGORY).round(1)
                                         ),
            "Heart rate":                rng.uniform(57, 100, ROWS_PER_CATEGORY).round(1),
            "Heat stroke":               0,
        })
        cold_chunks.append(chunk)

    df_final = pd.concat([df_clean] + cold_chunks, ignore_index=True)
    df_final.to_csv(output_csv, index=False)

    print(f"Input          : {input_csv}")
    print(f"Output         : {output_csv}")
    print(f"Removed        : {removed} rows (body temp < 35°C)")
    for (lo, hi), rh_val in COLD_RH_BY_BAND.items():
        print(f"Added          : {ROWS_PER_CATEGORY} rows  [{lo}°C to {hi}°C]  RH={rh_val}")
    print(f"Total rows     : {len(df_final)}")
    pos = (df_final["Heat stroke"] == 1).sum()
    neg = (df_final["Heat stroke"] == 0).sum()
    print(f"Positive       : {pos} ({pos/len(df_final)*100:.1f}%)")
    print(f"Negative       : {neg} ({neg/len(df_final)*100:.1f}%)")
    print(f"Env temp range : {df_final['Environmental temperature'].min()}°C – {df_final['Environmental temperature'].max()}°C")


INDOOR_EXERCISE_ROWS = 200  # small batch, enough to teach the pattern, won't skew balance


def add_indoor_exercise_cases(input_csv, output_csv, seed=42):
    """
    Appends indoor exercise negative cases to an existing dataset.

    Covers the dataset gap where the model incorrectly flags heatstroke for
    high HR + normal body temp indoors (e.g. gym training, indoor cycling).
    All cases have sun_exposure=0 and body_temp 37.5–38.4°C (safe exercise range
    per: https://getnice.com/blogs/articles-rocc/strength-training-and-core-body-temperature).
    Heart rate spans the full intense training range (108–167 bpm) matching
    target HR zones for ages 20–30 per American Heart Association guidelines.
    https://www.heart.org/en/healthy-living/fitness/fitness-basics/target-heart-rates

    200 rows added — brings positive class from 24.1% to ~23.8%, negligible shift.
    """
    rng = np.random.default_rng(seed)
    n = INDOOR_EXERCISE_ROWS

    indoor_exercise = pd.DataFrame({
        "Environmental temperature": rng.uniform(18, 25, n).round(1),   # typical gym/indoor range
        "Body temperature":          rng.uniform(37.5, 38.5, n).round(1),
        "Relative Humidity":         rng.uniform(0.30, 0.60, n).round(3),
        "Exposure to sun":           0.0,
        "Heart rate":                rng.uniform(108, 167, n).round(1),
        "Heat stroke":               0,
    })

    df = pd.read_csv(input_csv)
    df_final = pd.concat([df, indoor_exercise], ignore_index=True)
    df_final.to_csv(output_csv, index=False)

    pos = (df_final["Heat stroke"] == 1).sum()
    neg = (df_final["Heat stroke"] == 0).sum()
    print(f"Added          : {n} indoor exercise rows (sun=0, body=37.5–38.4°C, hr=108–167)")
    print(f"Total rows     : {len(df_final)}")
    print(f"Positive       : {pos} ({pos/len(df_final)*100:.1f}%)")
    print(f"Negative       : {neg} ({neg/len(df_final)*100:.1f}%)")


if __name__ == "__main__":
    build_fixed_dataset(
        input_csv  = "AI/Datasets/Balanced_Dataset_75_25.csv",
        output_csv = "AI/Datasets/Balanced_Dataset_Fixed.csv",
    )
    add_indoor_exercise_cases(
        input_csv  = "AI/Datasets/Balanced_Dataset_Fixed.csv",
        output_csv = "AI/Datasets/Balanced_Dataset_Fixed.csv",
    )
