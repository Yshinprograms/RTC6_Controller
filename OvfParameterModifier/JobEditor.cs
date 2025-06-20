// OvfParameterModifier/JobEditor.cs

using OpenVectorFormat;
using System;
using System.Collections.Generic;
using System.Linq;

namespace OvfParameterModifier {
    /// <summary>
    /// Encapsulates all business logic for querying and modifying a Job object.
    /// This class is the single source of truth for the Job data model rules.
    /// </summary>
    public class JobEditor {
        // --- Query Methods ---
        public int GetMaxLayerIndex(Job job) {
            return job.WorkPlanes.Count - 1;
        }

        public bool DoesParamSetExist(Job job, int key) {
            return job.MarkingParamsMap.ContainsKey(key);
        }

        // --- Modification Methods ---
        public void ApplyParametersToLayerRange(Job job, int startLayer, int endLayer, int paramKey) {
            // THE FIX: Ensure the result of the helper method is checked correctly.
            if (!IsLayerRangeValid(job, startLayer, endLayer)) {
                throw new ArgumentOutOfRangeException(nameof(startLayer), "Layer range is invalid for this job.");
            }
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

        // --- Private Helper Methods ---
        private bool IsLayerRangeValid(Job job, int start, int end) {
            if (start > end) return false;
            if (start < 0) return false;
            if (end > GetMaxLayerIndex(job)) return false;
            return true;
        }

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