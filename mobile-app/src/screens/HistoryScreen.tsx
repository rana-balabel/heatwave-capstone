import { useMemo, useState } from "react";
import {
  ActivityIndicator,
  Alert,
  Pressable,
  ScrollView,
  StyleSheet,
  Text,
  View,
} from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";

import { File, Paths } from "expo-file-system";
import * as Sharing from "expo-sharing";

import { useVitalsStore } from "../store/useVitalsStore";

type SeriesKey =
  | "bpm"
  | "skinTemp"
  | "envTemp"
  | "humidity"
  | "expToSun"
  | "riskScore";

const SERIES_LABEL: Record<SeriesKey, string> = {
  bpm: "Heart rate (bpm)",
  skinTemp: "Body temp (°C)",
  envTemp: "Env temp (°C)",
  humidity: "Humidity (%)",
  expToSun: "Sun exposure",
  riskScore: "Risk score",
};

const SERIES_COLOR: Record<SeriesKey, string> = {
  bpm: "#e53935",
  skinTemp: "#fb8c00",
  envTemp: "#43a047",
  humidity: "#1e88e5",
  expToSun: "#f4c430",
  riskScore: "#8e24aa",
};

function MiniChart({ data, color }: { data: number[]; color: string }) {
  if (data.length < 2) return null;
  const min = Math.min(...data);
  const max = Math.max(...data);
  const range = max - min || 1;
  const BAR_WIDTH = 6;
  const HEIGHT = 60;

  return (
    <View
      style={{
        flexDirection: "row",
        alignItems: "flex-end",
        height: HEIGHT,
        gap: 2,
        marginTop: 8,
      }}
    >
      {data.map((v, i) => {
        const h = Math.max(4, ((v - min) / range) * HEIGHT);
        return (
          <View
            key={i}
            style={{
              width: BAR_WIDTH,
              height: h,
              backgroundColor: color,
              borderRadius: 2,
              opacity: 0.8,
            }}
          />
        );
      })}
    </View>
  );
}

export function HistoryScreen() {
  const history = useVitalsStore((s) => s.history);
  const [series, setSeries] = useState<SeriesKey>("bpm");
  const [exporting, setExporting] = useState(false);

  const chartData = useMemo(
    () =>
      history
        .map((h) => h[series] as number | null)
        .filter((v): v is number => v != null),
    [history, series]
  );

  const canExport = history.length > 0 && !exporting;

  async function handleExportCsv() {
    if (!canExport) return;
    setExporting(true);
    try {
      const header =
        "timestamp_iso,bpm,skinTemp,envTemp,humidity,expToSun,riskScore\n";
      const rows = history
        .map((h) => {
          const ts = new Date(h.timestamp).toISOString();
          return [
            ts,
            h.bpm ?? "",
            h.skinTemp ?? "",
            h.envTemp ?? "",
            h.humidity ?? "",
            h.expToSun ?? "",
            h.riskScore ?? "",
          ].join(",");
        })
        .join("\n");

      const csv = header + rows + "\n";
      const file = new File(Paths.document, `vitals-history-${Date.now()}.csv`);
      await file.write(csv);

      const canShare = await Sharing.isAvailableAsync();
      if (!canShare) {
        Alert.alert("Exported", `Saved CSV to: ${file.uri}`);
        return;
      }

      await Sharing.shareAsync(file.uri, {
        mimeType: "text/csv",
        dialogTitle: "Share vitals history",
      });
    } catch (e) {
      console.error(e);
      Alert.alert("Export failed", "Unable to export history CSV.");
    } finally {
      setExporting(false);
    }
  }

  const latestFew = history.slice(-5).reverse();

  return (
    <SafeAreaView style={styles.container} edges={["top", "left", "right"]}>
      <Text style={styles.title}>History</Text>
      <Text style={styles.subtitle}>
        View trends over time and export to CSV.
      </Text>

      <ScrollView
        horizontal
        showsHorizontalScrollIndicator={false}
        style={styles.seriesRow}
      >
        {(Object.keys(SERIES_LABEL) as SeriesKey[]).map((key) => {
          const selected = key === series;
          return (
            <Pressable
              key={key}
              onPress={() => setSeries(key)}
              style={[
                styles.chip,
                selected && {
                  backgroundColor: SERIES_COLOR[key],
                  borderColor: SERIES_COLOR[key],
                },
              ]}
            >
              <Text
                style={[styles.chipLabel, selected && styles.chipLabelSelected]}
              >
                {SERIES_LABEL[key]}
              </Text>
            </Pressable>
          );
        })}
      </ScrollView>

      <View style={styles.chartContainer}>
        {chartData.length > 0 ? (
          <>
            <Text style={styles.chartTitle}>{SERIES_LABEL[series]}</Text>
            <ScrollView horizontal showsHorizontalScrollIndicator={false}>
              <MiniChart data={chartData} color={SERIES_COLOR[series]} />
            </ScrollView>
            <View style={styles.statsRow}>
              <Text style={styles.stat}>
                Min: {Math.min(...chartData).toFixed(1)}
              </Text>
              <Text style={styles.stat}>
                Avg:{" "}
                {(
                  chartData.reduce((a, b) => a + b, 0) / chartData.length
                ).toFixed(1)}
              </Text>
              <Text style={styles.stat}>
                Max: {Math.max(...chartData).toFixed(1)}
              </Text>
            </View>
          </>
        ) : (
          <View style={styles.emptyState}>
            <Text style={styles.emptyTitle}>No history yet</Text>
            <Text style={styles.emptyBody}>
              Wear your device and stay connected to start collecting vitals.
            </Text>
          </View>
        )}
      </View>

      {latestFew.length > 0 && (
        <View style={styles.recentSection}>
          <Text style={styles.recentTitle}>Recent readings</Text>
          {latestFew.map((h, i) => (
            <View key={i} style={styles.recentRow}>
              <Text style={styles.recentTime}>
                {new Date(h.timestamp).toLocaleTimeString()}
              </Text>
              <Text style={styles.recentValue}>
                {h[series] != null ? (h[series] as number).toFixed(2) : "—"}
              </Text>
            </View>
          ))}
        </View>
      )}

      <Pressable
        accessibilityRole="button"
        onPress={handleExportCsv}
        disabled={!canExport}
        style={[styles.exportButton, !canExport && styles.exportButtonDisabled]}
      >
        {exporting ? (
          <ActivityIndicator color="#fff" />
        ) : (
          <Text style={styles.exportLabel}>Export history to CSV</Text>
        )}
      </Pressable>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 16,
    gap: 12,
  },
  title: {
    fontSize: 28,
    fontWeight: "700",
  },
  subtitle: {
    fontSize: 14,
    opacity: 0.7,
  },
  seriesRow: {
    flexGrow: 0,
  },
  chip: {
    paddingHorizontal: 10,
    paddingVertical: 6,
    borderRadius: 999,
    borderWidth: 1,
    borderColor: "rgba(0,0,0,0.15)",
    marginRight: 8,
  },
  chipLabel: {
    fontSize: 12,
    opacity: 0.8,
  },
  chipLabelSelected: {
    color: "#fff",
    opacity: 1,
    fontWeight: "600",
  },
  chartContainer: {
    flex: 1,
    justifyContent: "center",
  },
  chartTitle: {
    fontSize: 14,
    fontWeight: "600",
    opacity: 0.7,
    marginBottom: 4,
  },
  statsRow: {
    flexDirection: "row",
    gap: 16,
    marginTop: 8,
  },
  stat: {
    fontSize: 13,
    opacity: 0.7,
  },
  emptyState: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
    paddingHorizontal: 32,
    gap: 8,
  },
  emptyTitle: {
    fontSize: 18,
    fontWeight: "600",
  },
  emptyBody: {
    fontSize: 14,
    opacity: 0.7,
    textAlign: "center",
  },
  recentSection: {
    gap: 6,
  },
  recentTitle: {
    fontSize: 14,
    fontWeight: "600",
    opacity: 0.7,
  },
  recentRow: {
    flexDirection: "row",
    justifyContent: "space-between",
    paddingVertical: 4,
    borderBottomWidth: StyleSheet.hairlineWidth,
    borderBottomColor: "rgba(0,0,0,0.1)",
  },
  recentTime: {
    fontSize: 13,
    opacity: 0.6,
  },
  recentValue: {
    fontSize: 13,
    fontWeight: "600",
  },
  exportButton: {
    paddingVertical: 12,
    borderRadius: 999,
    backgroundColor: "#1976d2",
    alignItems: "center",
  },
  exportButtonDisabled: {
    backgroundColor: "rgba(0,0,0,0.15)",
  },
  exportLabel: {
    color: "#fff",
    fontWeight: "600",
  },
});
