import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import roc_auc_score
import joblib
import tensorflow as tf

# Load dataset
df = pd.read_csv("AI/Datasets/Balanced_Dataset_Fixed.csv")
X = df[['Environmental temperature', 'Body temperature', 'Relative Humidity', 'Exposure to sun', 'Heart rate']]
Y = df['Heat stroke']

X_train, X_test, y_train, y_test = train_test_split(X, Y, test_size=0.2, random_state=42)

scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)
joblib.dump(scaler, "AI/scaler.joblib")

# Label smoothing: convert 0 -> 0.05, 1 -> 0.95
y_train_smoothed = y_train * 0.9 + 0.05

# Build model
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(5,)),
    tf.keras.layers.Dense(10, activation='sigmoid', kernel_regularizer=tf.keras.regularizers.l2(0.01)),
    tf.keras.layers.Dropout(0.3),
    tf.keras.layers.Dense(1, activation='sigmoid')
])

model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])

# Early stopping
early_stop = tf.keras.callbacks.EarlyStopping(monitor='val_loss', patience=5, restore_best_weights=True)

# Train
model.fit(X_train, y_train_smoothed, epochs=100, batch_size=8, validation_split=0.2, verbose=1, callbacks=[early_stop])

# Evaluate
train_predictions = model.predict(X_train).flatten()
train_accuracy = np.mean((train_predictions >= 0.5).astype(int) == y_train.to_numpy())

predictions = model.predict(X_test).flatten()
test_accuracy = np.mean((predictions >= 0.5).astype(int) == y_test.to_numpy())
auc = roc_auc_score(y_test, predictions)

print(f"\nTrain Accuracy : {train_accuracy:.4f}")
print(f"Test Accuracy  : {test_accuracy:.4f}")
print(f"AUC            : {auc:.4f}")

# Save predictions
pd.DataFrame({
    'Predicted Risk Score': predictions,
    'True Label': y_test.to_numpy()
}).to_csv("model_predictions.csv", index=False)

print("Saved: model_predictions.csv")

# Export the model to TensorFlow Lite (.tflite)
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open("heatwave_model_updated.tflite", "wb") as f:
    f.write(tflite_model)

print("Model converted to TFLite and saved as 'heatwave_model_updated.tflite'")
