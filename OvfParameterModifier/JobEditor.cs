// OvfParameterModifier/JobEditor.cs

using OpenVectorFormat;
using System;
using System.Collections.Generic;
using System.Linq;

namespace OvfParameterModifier {
    public class JobEditor {
        // ... (GetMaxLayerIndex and DoesParamSetExist are unchanged)
        public int GetMaxLayerIndex(Job job) {
            return job.WorkPlanes.Count - 1;
        }

        public bool DoesParamSetExist(Job job, int key) {
            return job.MarkingParamsMap.ContainsKey(key);
        }

        public void ApplyParametersToLayerRange(Job job, int startLayer, int endLayer, int paramKey) {
            // REMOVED: The IsLayerRangeValid check is now handled by the controller.
            if (!DoesParamSetExist(job, paramKey)) {
                throw new KeyNotFoundException($"Parameter Set with ID {paramKey} does not exist.");
            }

            for (int i = startLayer; i <= endLayer; i++) {
                var workPlane = job.WorkPlanes[i];
                foreach (var vectorBlock in workPlane.VectorBlocks) {
                    vectorBlock.MarkingParamsKey = paramKey;
                }
            }
        }

        // ... (FindOrCreateParameterSetKey is unchanged)
        public int FindOrCreateParameterSetKey(Job job, float power, float speed) {
            var markingParamsMap = job.MarkingParamsMap;

            foreach (var entry in markingParamsMap) {
                if (DoParamsMatch(entry.Value, power, speed)) {
                    return entry.Key;
                }
            }

            var newParams = new MarkingParams {
                LaserPowerInW = power,
                LaserSpeedInMmPerS = speed,
                Name = $"P{power}W_S{speed}mmps"
            };

            int newKey = GetNextAvailableParamKey(markingParamsMap);
            markingParamsMap.Add(newKey, newParams);
            return newKey;
        }

        // REMOVED: This private helper is no longer needed here.
        // private bool IsLayerRangeValid(Job job, int start, int end) { ... }

        // ... (GetNextAvailableParamKey, DoParamsMatch, AreFloatsClose are unchanged)
        private int GetNextAvailableParamKey(IDictionary<int, MarkingParams> markingParamsMap) {
            if (markingParamsMap.Keys.Count == 0) {
                return 1;
            }
            return markingParamsMap.Keys.Max() + 1;
        }

        private bool DoParamsMatch(MarkingParams existingParams, float desiredPower, float desiredSpeed) {
            bool powerMatches = AreFloatsClose(existingParams.LaserPowerInW, desiredPower);
            bool speedMatches = AreFloatsClose(existingParams.LaserSpeedInMmPerS, desiredSpeed);

            return powerMatches && speedMatches;
        }

        private static bool AreFloatsClose(float a, float b, float tolerance = 0.001f) {
            return Math.Abs(a - b) < tolerance;
        }
    }
}