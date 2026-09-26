import * as Location from 'expo-location';

export type WeatherData = {
  temp: number;
  feelsLike: number;
  description: string;
  condition: string;
  city: string;
  humidity: number;
};

const OPENWEATHER_API_KEY =
  (typeof process !== 'undefined' && process.env?.EXPO_PUBLIC_OPENWEATHER_API_KEY) ||
  '';

const OPENWEATHER_URL = 'https://api.openweathermap.org/data/2.5/weather';

export async function getCurrentWeather(): Promise<WeatherData | null> {
  if (!OPENWEATHER_API_KEY.trim()) {
    return null;
  }

  const { status } = await Location.requestForegroundPermissionsAsync();
  if (status !== 'granted') {
    throw new Error('Location permission denied');
  }

  const location = await Location.getCurrentPositionAsync({
    accuracy: Location.Accuracy.Balanced,
  });

  const { latitude, longitude } = location.coords;
  const url = `${OPENWEATHER_URL}?lat=${latitude}&lon=${longitude}&appid=${OPENWEATHER_API_KEY}&units=metric`;

  const res = await fetch(url);
  if (!res.ok) {
    throw new Error(`Weather API error: ${res.status}`);
  }

  const data = (await res.json()) as {
    name?: string;
    main?: { temp?: number; feels_like?: number; humidity?: number };
    weather?: Array<{ description?: string; main?: string }>;
  };

  const main = data.main ?? {};
  const weather = data.weather?.[0] ?? {};

  return {
    temp: main.temp ?? 0,
    feelsLike: main.feels_like ?? main.temp ?? 0,
    humidity: main.humidity ?? 0,
    description: weather.description ?? '',
    condition: weather.main ?? '',
    city: data.name ?? 'Unknown',
  };
}

export function isWeatherConfigured(): boolean {
  return OPENWEATHER_API_KEY.trim().length > 0;
}
