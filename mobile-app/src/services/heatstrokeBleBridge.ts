import type { BleError } from 'react-native-ble-plx';

import { bleService } from './bleService';
import { useVitalsStore } from '../store/useVitalsStore';

// Derived from your BlueNRG COPY_UUID_128 macros.
// The firmware passes the UUID bytes in little-endian order; the canonical UUID
// string below is the reversed byte order.
export const HeatstrokeBleUuids = {
  service:   '0100e180-0234-1200-0000-10000000aaaa',
  risk:      '0200e180-0234-1200-0000-10000000aaaa',
  envTemp:   '0300e180-0234-1200-0000-10000000aaaa',
  skinTemp:  '0400e180-0234-1200-0000-10000000aaaa',
  humidity:  '0500e180-0234-1200-0000-10000000aaaa',
  expToSun:  '0600e180-0234-1200-0000-10000000aaaa',
  heartRate: '0700e180-0234-1200-0000-10000000aaaa',
} as const;

function float32FromBytesLE(bytes: Uint8Array): number {
  if (bytes.byteLength < 4) throw new Error('Expected 4 bytes for float32');
  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  return view.getFloat32(0, true);
}

export type HeatstrokeBleBridge = {
  remove: () => void;
};

export function startHeatstrokeBleBridge(
  deviceId: string,
  opts?: { onError?: (e: BleError | Error) => void },
): HeatstrokeBleBridge {
  let commitTimer: ReturnType<typeof setTimeout> | null = null;
  let removed = false;

  const scheduleCommit = () => {
    if (commitTimer) clearTimeout(commitTimer);
    commitTimer = setTimeout(() => {
      commitTimer = null;
      if (removed) return;
      useVitalsStore.getState().appendReading();
    }, 250);
  };

  const onError = (e: BleError) => opts?.onError?.(e);

  const riskSub = bleService.monitorCharacteristic(
    deviceId,
    HeatstrokeBleUuids.service,
    HeatstrokeBleUuids.risk,
    (bytes) => {
      useVitalsStore.getState().setVitals({ riskScore: float32FromBytesLE(bytes) });
      scheduleCommit();
    },
    onError,
  );

  const envTempSub = bleService.monitorCharacteristic(
    deviceId,
    HeatstrokeBleUuids.service,
    HeatstrokeBleUuids.envTemp,
    (bytes) => {
      useVitalsStore.getState().setVitals({ envTemp: float32FromBytesLE(bytes) });
      scheduleCommit();
    },
    onError,
  );

  const skinSub = bleService.monitorCharacteristic(
    deviceId,
    HeatstrokeBleUuids.service,
    HeatstrokeBleUuids.skinTemp,
    (bytes) => {
      useVitalsStore.getState().setVitals({ skinTemp: float32FromBytesLE(bytes) });
      scheduleCommit();
    },
    onError,
  );

  const humiditySub = bleService.monitorCharacteristic(
    deviceId,
    HeatstrokeBleUuids.service,
    HeatstrokeBleUuids.humidity,
    (bytes) => {
      useVitalsStore.getState().setVitals({ humidity: float32FromBytesLE(bytes) });
      scheduleCommit();
    },
    onError,
  );

  const expToSunSub = bleService.monitorCharacteristic(
    deviceId,
    HeatstrokeBleUuids.service,
    HeatstrokeBleUuids.expToSun,
    (bytes) => {
      useVitalsStore.getState().setVitals({ expToSun: float32FromBytesLE(bytes) });
      scheduleCommit();
    },
    onError,
  );

  const hrSub = bleService.monitorCharacteristic(
    deviceId,
    HeatstrokeBleUuids.service,
    HeatstrokeBleUuids.heartRate,
    (bytes) => {
      useVitalsStore.getState().setVitals({ bpm: float32FromBytesLE(bytes) });
      scheduleCommit();
    },
    onError,
  );

  return {
    remove: () => {
      removed = true;
      if (commitTimer) clearTimeout(commitTimer);
      riskSub.remove();
      envTempSub.remove();
      skinSub.remove();
      humiditySub.remove();
      expToSunSub.remove();
      hrSub.remove();
    },
  };
}

