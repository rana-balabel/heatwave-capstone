import { Platform } from 'react-native';
import {
  BleError,
  BleManager,
  Device,
  State as BleState,
  Subscription,
} from 'react-native-ble-plx';

export type BleScanResult = {
  id: string;
  name: string | null;
  rssi: number | null;
};

export type BleMonitorSubscription = {
  remove: () => void;
};

function base64ToBytes(base64: string): Uint8Array {
  // Minimal base64 -> bytes decoder that does not rely on atob/Buffer.
  const chars =
    'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';

  let clean = base64.replace(/[\r\n\s]/g, '');
  if (clean.length % 4 === 1) {
    throw new Error('Invalid base64 length');
  }

  const padding = clean.endsWith('==') ? 2 : clean.endsWith('=') ? 1 : 0;
  const outputLen = Math.floor((clean.length * 3) / 4) - padding;
  const bytes = new Uint8Array(outputLen);

  let byteIndex = 0;
  for (let i = 0; i < clean.length; i += 4) {
    const c0 = chars.indexOf(clean[i] ?? '');
    const c1 = chars.indexOf(clean[i + 1] ?? '');
    const c2 = chars.indexOf(clean[i + 2] ?? '');
    const c3 = chars.indexOf(clean[i + 3] ?? '');

    if (c0 < 0 || c1 < 0 || (c2 < 0 && clean[i + 2] !== '=') || (c3 < 0 && clean[i + 3] !== '=')) {
      throw new Error('Invalid base64 character');
    }

    const n =
      (c0 << 18) |
      (c1 << 12) |
      ((c2 & 63) << 6) |
      (c3 & 63);

    if (byteIndex < outputLen) bytes[byteIndex++] = (n >> 16) & 0xff;
    if (byteIndex < outputLen) bytes[byteIndex++] = (n >> 8) & 0xff;
    if (byteIndex < outputLen) bytes[byteIndex++] = n & 0xff;
  }

  return bytes;
}

// Placeholder: this is where you'll parse your FreeRTOS payload into vitals.
// Do NOT implement decoding logic yet (Phase 4/5 will wire it to the store).
function decodeVitalsPacket(_bytes: Uint8Array): void {
  return;
}

class BleService {
  private manager: BleManager;
  private connectedDevice: Device | null = null;

  constructor() {
    this.manager = new BleManager();
  }

  destroy() {
    this.manager.destroy();
  }

  async waitForPoweredOn(timeoutMs = 10_000): Promise<void> {
    const state = await this.manager.state();
    if (state === BleState.PoweredOn) return;

    await new Promise<void>((resolve, reject) => {
      const timer = setTimeout(() => {
        sub.remove();
        reject(new Error('Bluetooth not powered on (timeout)'));
      }, timeoutMs);

      const sub = this.manager.onStateChange((next) => {
        if (next === BleState.PoweredOn) {
          clearTimeout(timer);
          sub.remove();
          resolve();
        }
      }, true);
    });
  }

  startScan(onDevice: (device: BleScanResult) => void, onError?: (e: BleError) => void) {
    const seen = new Set<string>();

    this.manager.startDeviceScan(null, null, (error, device) => {
      if (error) {
        onError?.(error);
        return;
      }
      if (!device) return;

      if (seen.has(device.id)) return;
      seen.add(device.id);

      onDevice({
        id: device.id,
        name: device.name ?? device.localName ?? null,
        rssi: device.rssi ?? null,
      });
    });

    return () => this.manager.stopDeviceScan();
  }

  async connect(deviceId: string): Promise<Device> {
    const device = await this.manager.connectToDevice(deviceId, {
      // Helps stability on Android with some peripherals.
      requestMTU: Platform.OS === 'android' ? 185 : undefined,
    });

    const ready = await device.discoverAllServicesAndCharacteristics();
    this.connectedDevice = ready;
    return ready;
  }

  async disconnect(): Promise<void> {
    if (!this.connectedDevice) return;
    const id = this.connectedDevice.id;
    this.connectedDevice = null;
    await this.manager.cancelDeviceConnection(id);
  }

  monitorCharacteristic(
    deviceId: string,
    serviceUUID: string,
    characteristicUUID: string,
    onBytes: (bytes: Uint8Array) => void,
    onError?: (e: BleError) => void,
  ): BleMonitorSubscription {
    const sub: Subscription = this.manager.monitorCharacteristicForDevice(
      deviceId,
      serviceUUID,
      characteristicUUID,
      (error, characteristic) => {
        if (error) {
          onError?.(error);
          return;
        }
        const value = characteristic?.value;
        if (!value) return;

        try {
          const bytes = base64ToBytes(value);
          // Placeholder: keep the hook here but don't parse into vitals yet.
          decodeVitalsPacket(bytes);
          onBytes(bytes);
        } catch (e) {
          onError?.(e as BleError);
        }
      },
    );

    return { remove: () => sub.remove() };
  }
}

export const bleService = new BleService();
export { base64ToBytes };

