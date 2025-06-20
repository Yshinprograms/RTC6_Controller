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
            if (!IsLayerRangeValid(job, startLayer, endLayer)) {
                throw new ArgumentOutOfRangeException(nameof(startLayer), $"Layer range is invalid for this job.");
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

            // If no match was found, create a new one.
            var newParams = new MarkingParams {
                LaserPowerInW = power,
                LaserSpeedInMmPerS = speed,
                Name = $"P{power}W_S{speed}mmps"
            };

            int newKey = GetNextAvailableParamKey(markingParamsMap);
            markingParamsMap.Add(newKey, newParams);
            return newKey;
        }

        // --- Private Helper Methods for Readability & Logic Encapsulation ---

        /// <summary>
        /// Checks if a given layer range is valid for the specified job.
        /// </summary>
        private bool IsLayerRangeValid(Job job, int start, int end) {
            // A range is valid if it doesn't start after it ends,
            // and if both start and end are within the job's bounds.
            if (start > end) return false;
            if (start < 0) return false;
            if (end > GetMaxLayerIndex(job)) return false;

            return true;
        }

        /// <summary>
        /// Determines the next available integer key for the marking parameters map.
        /// </summary>
        private int GetNextAvailableParamKey(IDictionary<int, MarkingParams> markingParamsMap) {
            // If the map is empty, start with key 1. Otherwise, find the max key and add 1.
            if (markingParamsMap.Keys.Count == 0) {
                return 1;
            }
            return markingParamsMap.Keys.Max() + 1;
        }

        /// <summary>
        /// Compares an existing MarkingParams object with desired values for power and speed.
        /// </summary>
        private bool DoParamsMatch(MarkingParams existingParams, float desiredPower, float desiredSpeed) {
            // Parameters match if both power and speed are approximately equal.
            bool powerMatches = AreFloatsClose(existingParams.LaserPowerInW, desiredPower);
            bool speedMatches = AreFloatsClose(existingParams.LaserSpeedInMmPerS, desiredSpeed);

            return powerMatches && speedMatches;
        }

        /// <summary>
        /// Safely compares two floating-point numbers for approximate equality.
        /// </summary>
        private static bool AreFloatsClose(float a, float b, float tolerance = 0.001f) {
            return Math.Abs(a - b) < tolerance;
        }
    }
}