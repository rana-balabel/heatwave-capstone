"""
Unit tests for heatwave TFLite model inference.

Input features (in order): Environmental temperature, Body temperature,
                            Relative Humidity, Exposure to sun, Heart rate

Risk thresholds:
    Low risk  : score < 0.5
    Mild risk : 0.50 <= score < 0.75
    High risk : score >= 0.75

Run from project root:
    python -m pytest AI/tests/test_inference.py -v
"""

import os
import unittest
import numpy as np
import joblib
import tensorflow as tf

_DIR = os.path.dirname(os.path.abspath(__file__))
MODEL_PATH  = os.path.join(_DIR, "..", "heatwave_model_updated.tflite")
SCALER_PATH = os.path.join(_DIR, "scaler.joblib")

scaler = joblib.load(SCALER_PATH)


def run_inference(env_temp, body_temp, humidity, sun_exposure, heart_rate):
    """Scale inputs then run a single inference, returning the raw risk score (0–1)."""
    interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
    interpreter.allocate_tensors()
    input_details  = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    raw_input = np.array([[env_temp, body_temp, humidity, sun_exposure, heart_rate]], dtype=np.float32)
    scaled_input = scaler.transform(raw_input).astype(np.float32)
    interpreter.set_tensor(input_details[0]['index'], scaled_input)
    interpreter.invoke()
    return float(interpreter.get_tensor(output_details[0]['index'])[0][0])


def assertLowRisk(test, score, msg=""):
    test.assertLess(score, 0.50, f"Expected low risk (< 0.5), got {score:.4f}. {msg}")

def assertMildRisk(test, score, msg=""):
    test.assertGreaterEqual(score, 0.50, f"Expected mild risk (>= 0.5), got {score:.4f}. {msg}")
    test.assertLess(score, 0.75, f"Expected mild risk (< 0.75), got {score:.4f}. {msg}")

def assertHighRisk(test, score, msg=""):
    test.assertGreaterEqual(score, 0.75, f"Expected high risk (>= 0.75), got {score:.4f}. {msg}")


class TestHeatstrokePositiveCases(unittest.TestCase):
    """Clear heatstroke conditions — expect HIGH risk (>= 0.75)."""

    def test_classic_heatstroke(self):
        """High body temp, high heart rate, hot environment, strong sun."""
        score = run_inference(env_temp=42, body_temp=41.0, humidity=0.5,
                              sun_exposure=120, heart_rate=155)
        assertHighRisk(self, score)

    def test_heatstroke_moderate_sun(self):
        """High body temp and HR with moderate sun exposure."""
        score = run_inference(env_temp=38, body_temp=40.5, humidity=0.4,
                              sun_exposure=70, heart_rate=140)
        assertHighRisk(self, score)

    def test_heatstroke_max_conditions(self):
        """Extreme conditions across all features."""
        score = run_inference(env_temp=45, body_temp=41.5, humidity=0.68,
                              sun_exposure=160, heart_rate=167)
        assertHighRisk(self, score)

    def test_non_exertional_heatstroke_hot_room(self):
        """Elderly person in a hot unventilated room: high body temp, no sun."""
        score = run_inference(env_temp=40, body_temp=40.5, humidity=0.55,
                              sun_exposure=0, heart_rate=128)
        assertHighRisk(self, score)

    def test_non_exertional_heatstroke_hot_car(self):
        """Infant in a hot enclosed space: extreme body temp, no direct sun."""
        score = run_inference(env_temp=42, body_temp=41.2, humidity=0.6,
                              sun_exposure=0, heart_rate=145)
        assertHighRisk(self, score)


class TestHeatstrokeMildRiskCases(unittest.TestCase):
    """Outdoor exertion with elevated but safe body temp (38–39°C) and high HR.
    Not heatstroke, but body is under stress — expect MILD risk (0.5 <= score < 0.75)."""
    def test_cyclist_sunny(self):
        """Cyclist on a hot sunny day: very high HR, body temp at upper safe limit."""
        score = run_inference(env_temp=35, body_temp=38.4, humidity=0.38,
                              sun_exposure=90, heart_rate=160)
        assertMildRisk(self, score)
    
    def test_runner_hot_day_end_race(self):
        """Marathon runner end-race: high HR, body temp elevated but not at high heatstroke threshold."""
        # https://pmc.ncbi.nlm.nih.gov/articles/PMC10988464/
        score = run_inference(env_temp=32, body_temp=39.0, humidity=0.31,
                              sun_exposure=120, heart_rate=160)
        assertMildRisk(self, score)

    def test_non_exertional_heatstroke_warm_car_onset(self):
        """Person in a hot enclosed space: extreme body temp, no direct sun."""
        score = run_inference(env_temp=36, body_temp=39.0, humidity=0.6,
                              sun_exposure=0, heart_rate=145)
        assertMildRisk(self, score)


class TestHeatstrokeNegativeCases(unittest.TestCase):
    """Safe conditions — expect LOW risk (< 0.5)."""

    def test_cold_weather(self):
        """Cold environment should never produce heatstroke."""
        score = run_inference(env_temp=-5, body_temp=37.0, humidity=0.704,
                              sun_exposure=30, heart_rate=90)
        assertLowRisk(self, score)

    def test_freezing_temperature(self):
        """Freezing temperature with no sun."""
        score = run_inference(env_temp=-10, body_temp=36.8, humidity=0.71,
                              sun_exposure=0, heart_rate=65)
        assertLowRisk(self, score)

    def test_cool_temperature(self):
        """Cool but not cold — 15°C should not trigger heatstroke."""
        score = run_inference(env_temp=15, body_temp=37.0, humidity=0.677,
                              sun_exposure=20, heart_rate=75)
        assertLowRisk(self, score)

    def test_indoor_gym(self):
        """Indoor training for 30 year old: high HR but no sun exposure."""
        # https://www.heart.org/en/healthy-living/fitness/fitness-basics/target-heart-rates
        score = run_inference(env_temp=24, body_temp=38.2, humidity=0.31,
                              sun_exposure=0, heart_rate=160)
        assertLowRisk(self, score)

    def test_indoor_hot_day(self):
        """Hot outside but indoors (sun=0) — no heatstroke expected."""
        score = run_inference(env_temp=38, body_temp=37.1, humidity=0.35,
                              sun_exposure=0, heart_rate=80)
        assertLowRisk(self, score)

    def test_normal_outdoor_mild_weather(self):
        """Mild weather, normal vitals, some sun exposure."""
        score = run_inference(env_temp=25, body_temp=37.0, humidity=0.4,
                              sun_exposure=40, heart_rate=78)
        assertLowRisk(self, score)

    def test_high_env_temp_normal_vitals(self):
        """Hot day but body temp and HR are normal — env temp alone is a weak signal."""
        score = run_inference(env_temp=44, body_temp=37.2, humidity=0.3,
                              sun_exposure=50, heart_rate=85)
        assertLowRisk(self, score)

    def test_construction_worker(self):
        """Outdoor construction worker: moderate exertion in strong sun, body temp safe."""
        score = run_inference(env_temp=38, body_temp=37.9, humidity=0.5,
                              sun_exposure=100, heart_rate=130)
        assertLowRisk(self, score)

    def test_runner_hot_day(self):
        """Marathon runner mid-race: high HR, body temp elevated but not at heatstroke threshold."""
        # https://pmc.ncbi.nlm.nih.gov/articles/PMC10988464/
        score = run_inference(env_temp=32, body_temp=38.2, humidity=0.45,
                              sun_exposure=80, heart_rate=155)
        assertLowRisk(self, score)

    def test_stressed_rana(self):
        """"Rana is stressed because she has 4 interviews and 2 coop job offers but doesnt know what to select"""
        score = run_inference(env_temp=20, body_temp=37.2, humidity=0.35,
                              sun_exposure=10, heart_rate=135)
        assertLowRisk(self, score)

class TestModelOutput(unittest.TestCase):
    """Sanity checks on model output format."""

    def test_output_is_between_0_and_1(self):
        """Model output must always be a valid probability."""
        score = run_inference(env_temp=35, body_temp=38.0, humidity=0.45,
                              sun_exposure=60, heart_rate=100)
        self.assertGreaterEqual(score, 0.0)
        self.assertLessEqual(score, 1.0)

    def test_output_is_scalar(self):
        """Model should return a single float value."""
        score = run_inference(env_temp=35, body_temp=38.0, humidity=0.45,
                              sun_exposure=60, heart_rate=100)
        self.assertIsInstance(score, float)


if __name__ == "__main__":
    unittest.main(verbosity=2)
