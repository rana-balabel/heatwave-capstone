import AsyncStorage from '@react-native-async-storage/async-storage';

const PROFILE_KEY = '@heatwave/profile';

export type Profile = {
  name: string;
  weightKg: string;
  age: string;
};

const DEFAULT_PROFILE: Profile = {
  name: '',
  weightKg: '',
  age: '',
};

export async function loadProfile(): Promise<Profile> {
  try {
    const raw = await AsyncStorage.getItem(PROFILE_KEY);
    if (!raw) return { ...DEFAULT_PROFILE };
    const parsed = JSON.parse(raw) as Partial<Profile>;
    return {
      name: parsed.name ?? DEFAULT_PROFILE.name,
      weightKg: parsed.weightKg ?? DEFAULT_PROFILE.weightKg,
      age: parsed.age ?? DEFAULT_PROFILE.age,
    };
  } catch {
    return { ...DEFAULT_PROFILE };
  }
}

export async function saveProfile(profile: Profile): Promise<void> {
  await AsyncStorage.setItem(PROFILE_KEY, JSON.stringify(profile));
}
