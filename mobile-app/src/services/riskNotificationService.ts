import * as Notifications from 'expo-notifications';
import { Platform } from 'react-native';

import { calculateRisk, type RiskLevel } from '../utils/calculateRisk';
import { useVitalsStore } from '../store/useVitalsStore';

const riskRank: Record<RiskLevel, number> = {
  LOW: 0,
  MODERATE: 1,
  HIGH: 2,
};

Notifications.setNotificationHandler({
  handleNotification: async () => ({
    shouldShowBanner: true,
    shouldShowList: true,
    shouldPlaySound: true,
    shouldSetBadge: false,
  }),
});

export async function ensureNotificationsReady() {
  if (Platform.OS === 'android') {
    await Notifications.setNotificationChannelAsync('risk-alerts', {
      name: 'Risk alerts',
      importance: Notifications.AndroidImportance.HIGH,
      vibrationPattern: [0, 200, 100, 200],
      sound: 'default',
    });
  }

  const current = await Notifications.getPermissionsAsync();
  if (current.granted) return;

  const requested = await Notifications.requestPermissionsAsync();
  if (!requested.granted) {
    // User denied; we silently keep the app running without notifications.
    return;
  }
}

export type RiskWatcherSubscription = {
  remove: () => void;
};

/**
 * Watches vitals changes and triggers a local notification only when
 * risk level increases (LOW -> MODERATE -> HIGH).
 */
export function startRiskIncreaseWatcher(): RiskWatcherSubscription {
  const initial = calculateRisk(useVitalsStore.getState());
  let lastLevel: RiskLevel = initial.level;

  const unsubscribe = useVitalsStore.subscribe((state) => {
    const { level, score } = calculateRisk(state);
    if (riskRank[level] <= riskRank[lastLevel]) return;

    lastLevel = level;

    void Notifications.scheduleNotificationAsync({
      content: {
        title: `Heat risk increased: ${level}`,
        body: `Current risk score: ${score}/100`,
        sound: 'default',
        ...(Platform.OS === 'android' ? { channelId: 'risk-alerts' } : null),
      },
      trigger: null,
    });
  });

  return { remove: unsubscribe };
}
