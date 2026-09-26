import { StatusBar } from 'expo-status-bar';
import { NavigationContainer } from '@react-navigation/native';
import { useEffect } from 'react';

import { RootTabs } from './src/navigation/RootTabs';
import {
  ensureNotificationsReady,
  startRiskIncreaseWatcher,
} from './src/services/riskNotificationService';

export default function App() {
  useEffect(() => {
    let sub: { remove: () => void } | null = null;

    void ensureNotificationsReady().finally(() => {
      sub = startRiskIncreaseWatcher();
    });

    return () => sub?.remove();
  }, []);

  return (
    <NavigationContainer>
      <RootTabs />
      <StatusBar style="auto" />
    </NavigationContainer>
  );
}
