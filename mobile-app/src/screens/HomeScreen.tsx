import { useCallback, useEffect, useRef, useState } from "react";
import { Pressable, StyleSheet, Text, View } from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";

import {
  startHeatstrokeBleBridge,
  type HeatstrokeBleBridge,
} from "../services/heatstrokeBleBridge";
import { bleService } from "../services/bleService";
import { calculateRisk } from "../utils/calculateRisk";
import { useVitalsStore } from "../store/useVitalsStore";

type BleStatus = "idle" | "scanning" | "connecting" | "connected" | "error";

const DEVICE_NAME = "Heat";

function formatNumber(value: number | null, digits = 1) {
  if (value == null) return "—";
  return value.toFixed(digits);
}

function formatTime(ts: number | null) {
  if (!ts) return "—";
  const d = new Date(ts);
  return d.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
}

export function HomeScreen() {
  const { riskScore, envTemp, skinTemp, humidity, expToSun, bpm, history } =
    useVitalsStore();
  const lastTs = history.length ? history[history.length - 1]?.timestamp : null;
  const risk = calculateRisk(riskScore);

  const [bleStatus, setBleStatus] = useState<BleStatus>("idle");
  const bridgeRef = useRef<HeatstrokeBleBridge | null>(null);

  const riskStyle =
    risk.level === "HIGH"
      ? styles.riskHigh
      : risk.level === "MODERATE"
      ? styles.riskModerate
      : styles.riskLow;

  const connect = useCallback(async () => {
    try {
      setBleStatus("scanning");
      await bleService.waitForPoweredOn();

      const deviceId = await new Promise<string>((resolve, reject) => {
        const timeout = setTimeout(() => {
          stopScan();
          reject(new Error("Heat not found within 15 s"));
        }, 15_000);

        const stopScan = bleService.startScan(
          (device) => {
            if (device.name === DEVICE_NAME) {
              clearTimeout(timeout);
              stopScan();
              resolve(device.id);
            }
          },
          (e) => {
            clearTimeout(timeout);
            reject(e);
          }
        );
      });

      setBleStatus("connecting");
      await bleService.connect(deviceId);

      bridgeRef.current = startHeatstrokeBleBridge(deviceId, {
        onError: (e) => console.warn("BLE bridge error", e),
      });

      setBleStatus("connected");
    } catch (e) {
      console.warn("BLE connect failed", e);
      setBleStatus("error");
    }
  }, []);

  const disconnect = useCallback(async () => {
    bridgeRef.current?.remove();
    bridgeRef.current = null;
    await bleService.disconnect();
    setBleStatus("idle");
  }, []);

  useEffect(() => {
    return () => {
      bridgeRef.current?.remove();
      bridgeRef.current = null;
    };
  }, []);

  const bleLabel: Record<BleStatus, string> = {
    idle: "Connect",
    scanning: "Scanning…",
    connecting: "Connecting…",
    connected: "Disconnect",
    error: "Retry",
  };

  return (
    <SafeAreaView style={styles.container} edges={["top", "left", "right"]}>
      <View style={styles.header}>
        <View style={styles.headerLeft}>
          <Text style={styles.title}>Dashboard</Text>
          <Text style={styles.subtitle}>Last update: {formatTime(lastTs)}</Text>
        </View>

        <View style={[styles.riskPill, riskStyle]}>
          <Text style={styles.riskLabel}>{risk.level}</Text>
        </View>
      </View>

      <Pressable
        style={[
          styles.bleButton,
          bleStatus === "connected" && styles.bleButtonConnected,
        ]}
        onPress={bleStatus === "connected" ? disconnect : connect}
        disabled={bleStatus === "scanning" || bleStatus === "connecting"}
        accessibilityRole="button"
      >
        <Text style={styles.bleButtonLabel}>
          {bleStatus === "connected" ? "● " : "○ "}
          {bleLabel[bleStatus]}
        </Text>
      </Pressable>

      <View style={styles.grid}>
        <View style={styles.card}>
          <Text style={styles.cardLabel}>Risk score</Text>
          <Text style={styles.cardValue}>
            {riskScore != null ? riskScore.toFixed(3) : "—"}
          </Text>
        </View>

        <View style={styles.card}>
          <Text style={styles.cardLabel}>Heart rate</Text>
          <Text style={styles.cardValue}>{formatNumber(bpm, 0)} bpm</Text>
        </View>

        <View style={styles.card}>
          <Text style={styles.cardLabel}>Body temp</Text>
          <Text style={styles.cardValue}>{formatNumber(skinTemp, 1)} °C</Text>
        </View>

        <View style={styles.card}>
          <Text style={styles.cardLabel}>Env temp</Text>
          <Text style={styles.cardValue}>{formatNumber(envTemp, 1)} °C</Text>
        </View>

        <View style={styles.card}>
          <Text style={styles.cardLabel}>Humidity</Text>
          <Text style={styles.cardValue}>{formatNumber(humidity, 1)} %</Text>
        </View>

        <View style={styles.card}>
          <Text style={styles.cardLabel}>Sun exposure</Text>
          <Text style={styles.cardValue}>{formatNumber(expToSun, 2)}</Text>
        </View>
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 16,
    gap: 16,
  },
  header: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    gap: 12,
  },
  headerLeft: {
    flex: 1,
    gap: 4,
  },
  title: {
    fontSize: 28,
    fontWeight: "700",
  },
  subtitle: {
    fontSize: 14,
    opacity: 0.7,
  },
  riskPill: {
    paddingHorizontal: 12,
    paddingVertical: 8,
    borderRadius: 999,
  },
  riskLabel: {
    fontSize: 14,
    fontWeight: "800",
    letterSpacing: 0.5,
    color: "white",
  },
  riskLow: {
    backgroundColor: "#2e7d32",
  },
  riskModerate: {
    backgroundColor: "#b26a00",
  },
  riskHigh: {
    backgroundColor: "#b71c1c",
  },
  grid: {
    flexDirection: "row",
    flexWrap: "wrap",
    gap: 12,
  },
  card: {
    width: "48%",
    padding: 14,
    borderRadius: 14,
    backgroundColor: "rgba(0,0,0,0.05)",
    gap: 8,
  },
  cardLabel: {
    fontSize: 13,
    opacity: 0.7,
    fontWeight: "600",
  },
  cardValue: {
    fontSize: 20,
    fontWeight: "800",
  },
  bleButton: {
    paddingVertical: 10,
    paddingHorizontal: 16,
    borderRadius: 10,
    backgroundColor: "rgba(0,0,0,0.06)",
    borderWidth: 1,
    borderColor: "rgba(0,0,0,0.12)",
    alignItems: "center",
  },
  bleButtonConnected: {
    backgroundColor: "#e8f5e9",
    borderColor: "#2e7d32",
  },
  bleButtonLabel: {
    fontSize: 14,
    fontWeight: "600",
  },
});
