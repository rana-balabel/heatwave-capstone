import { useCallback, useState } from 'react';
import {
  ActivityIndicator,
  Pressable,
  StyleSheet,
  Text,
  View,
} from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';

import {
  getCurrentWeather,
  isWeatherConfigured,
  type WeatherData,
} from '../services/weatherService';

export function WeatherScreen() {
  const [weather, setWeather] = useState<WeatherData | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const fetchWeather = useCallback(async () => {
    if (!isWeatherConfigured()) {
      setError('API key not set');
      return;
    }
    setLoading(true);
    setError(null);
    setWeather(null);
    try {
      const data = await getCurrentWeather();
      setWeather(data);
    } catch (e) {
      setError(e instanceof Error ? e.message : 'Failed to load weather');
    } finally {
      setLoading(false);
    }
  }, []);

  const configured = isWeatherConfigured();

  return (
    <SafeAreaView style={styles.container} edges={['top', 'left', 'right']}>
      <Text style={styles.title}>Weather</Text>
      <Text style={styles.subtitle}>
        Local conditions for heat risk context (OpenWeatherMap).
      </Text>

      {!configured && (
        <View style={styles.card}>
          <Text style={styles.cardLabel}>Not configured</Text>
          <Text style={styles.cardBody}>
            Set EXPO_PUBLIC_OPENWEATHER_API_KEY in your .env or app config to
            enable weather. Get a free key at openweathermap.org/api.
          </Text>
        </View>
      )}

      {configured && (
        <>
          {loading && (
            <View style={styles.centered}>
              <ActivityIndicator size="large" />
              <Text style={styles.loadingText}>Getting location & weather…</Text>
            </View>
          )}

          {error && !loading && (
            <View style={styles.card}>
              <Text style={styles.errorText}>{error}</Text>
              <Pressable style={styles.button} onPress={fetchWeather}>
                <Text style={styles.buttonLabel}>Retry</Text>
              </Pressable>
            </View>
          )}

          {weather && !loading && (
            <View style={styles.card}>
              <Text style={styles.city}>{weather.city}</Text>
              <Text style={styles.temp}>{Math.round(weather.temp)}°C</Text>
              <Text style={styles.feels}>
                Feels like {Math.round(weather.feelsLike)}°C
              </Text>
              <Text style={styles.condition}>
                {weather.condition} – {weather.description}
              </Text>
              <Text style={styles.meta}>Humidity: {weather.humidity}%</Text>
              <Pressable style={styles.refreshButton} onPress={fetchWeather}>
                <Text style={styles.buttonLabel}>Refresh</Text>
              </Pressable>
            </View>
          )}

          {!weather && !loading && !error && (
            <Pressable style={styles.primaryButton} onPress={fetchWeather}>
              <Text style={styles.buttonLabel}>Get current weather</Text>
            </Pressable>
          )}
        </>
      )}
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 16,
    gap: 12,
    backgroundColor: '#fff',
  },
  title: {
    fontSize: 28,
    fontWeight: '700',
  },
  subtitle: {
    fontSize: 14,
    opacity: 0.7,
  },
  card: {
    padding: 20,
    borderRadius: 14,
    backgroundColor: 'rgba(0,0,0,0.05)',
    gap: 8,
  },
  cardLabel: {
    fontSize: 14,
    fontWeight: '600',
    opacity: 0.8,
  },
  cardBody: {
    fontSize: 14,
    opacity: 0.8,
    lineHeight: 20,
  },
  centered: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    gap: 12,
  },
  loadingText: {
    fontSize: 14,
    opacity: 0.8,
  },
  errorText: {
    fontSize: 14,
    color: '#b71c1c',
  },
  city: {
    fontSize: 20,
    fontWeight: '700',
  },
  temp: {
    fontSize: 42,
    fontWeight: '800',
  },
  feels: {
    fontSize: 16,
    opacity: 0.8,
  },
  condition: {
    fontSize: 14,
    textTransform: 'capitalize',
    opacity: 0.9,
  },
  meta: {
    fontSize: 13,
    opacity: 0.7,
  },
  button: {
    marginTop: 8,
    paddingVertical: 10,
    paddingHorizontal: 16,
    borderRadius: 10,
    backgroundColor: 'rgba(0,0,0,0.08)',
    alignSelf: 'flex-start',
  },
  refreshButton: {
    marginTop: 12,
    paddingVertical: 10,
    paddingHorizontal: 16,
    borderRadius: 10,
    backgroundColor: '#1976d2',
    alignSelf: 'flex-start',
  },
  primaryButton: {
    marginTop: 16,
    paddingVertical: 14,
    borderRadius: 12,
    backgroundColor: '#1976d2',
    alignItems: 'center',
  },
  buttonLabel: {
    fontSize: 16,
    fontWeight: '600',
    color: '#fff',
  },
});
