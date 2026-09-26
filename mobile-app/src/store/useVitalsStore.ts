import { create } from 'zustand';

export type VitalsReading = {
  timestamp: number;
  riskScore: number | null;
  envTemp: number | null;
  skinTemp: number | null;
  humidity: number | null;
  expToSun: number | null;
  bpm: number | null;
};

export type VitalsSnapshot = Omit<VitalsReading, 'timestamp'>;

type VitalsStore = VitalsSnapshot & {
  history: VitalsReading[];
  setVitals: (vitals: Partial<VitalsSnapshot>) => void;
  appendReading: (reading?: Partial<VitalsReading>) => void;
  clearHistory: () => void;
};

export const useVitalsStore = create<VitalsStore>()((set, get) => ({
  riskScore: null,
  envTemp: null,
  skinTemp: null,
  humidity: null,
  expToSun: null,
  bpm: null,
  history: [],

  setVitals: (vitals) => set(vitals),

  appendReading: (reading) => {
    const s = get();

    const next: VitalsReading = {
      timestamp: reading?.timestamp ?? Date.now(),
      riskScore: reading?.riskScore ?? s.riskScore,
      envTemp: reading?.envTemp ?? s.envTemp,
      skinTemp: reading?.skinTemp ?? s.skinTemp,
      humidity: reading?.humidity ?? s.humidity,
      expToSun: reading?.expToSun ?? s.expToSun,
      bpm: reading?.bpm ?? s.bpm,
    };

    set({ history: [...s.history, next] });
  },

  clearHistory: () => set({ history: [] }),

}));

