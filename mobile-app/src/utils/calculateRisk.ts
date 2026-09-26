export type RiskLevel = 'LOW' | 'MODERATE' | 'HIGH';

export type RiskResult = {
  level: RiskLevel;
  score: number; // 0..1 from BLE riskScore characteristic
};

// riskScore is a float 0-1 from the STM AI model via BLE.
// 0.0 - 0.5: LOW, 0.5 - 0.75: MODERATE, 0.75 - 1.0: HIGH
export function calculateRisk(riskScore: number | null): RiskResult {
  const score = riskScore ?? 0;
  const level: RiskLevel = score >= 0.75 ? 'HIGH' : score >= 0.5 ? 'MODERATE' : 'LOW';
  return { level, score };
}

