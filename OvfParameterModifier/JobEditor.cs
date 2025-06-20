using OpenVectorFormat;
using System;
using System.Collections.Generic;
using System.Linq;

namespace OvfParameterModifier {
    public class JobEditor {
        public void ApplyParametersToLayerRange(Job job, int startLayer, int endLayer, int paramKey) {
            // Validate the layer range against the job's actual layers.
            int maxLayerIndex = job.WorkPlanes.Count - 1;
            if (startLayer < 0 || endLayer > maxLayerIndex) {
                throw new ArgumentOutOfRangeException($"Layer range is invalid. Job has layers from 0 to {maxLayerIndex}.");
            }

            for (int i = startLayer; i <= endLayer; i++) {
                var workPlane = job.WorkPlanes[i];
                foreach (var vectorBlock in workPlane.VectorBlocks) {
                    vectorBlock.MarkingParamsKey = paramKey;
                }
            }
        }

        public int FindOrCreateParameterSetKey(Job job, float power, float speed) {
            const float tolerance = 0.001f;
            var markingParamsMap = job.MarkingParamsMap;

            // Search for an existing match.
            foreach (var entry in markingParamsMap) {
                bool powerMatches = Math.Abs(entry.Value.LaserPowerInW - power) < tolerance;
                bool speedMatches = Math.Abs(entry.Value.LaserSpeedInMmPerS - speed) < tolerance;

                if (powerMatches && speedMatches) {
                    return entry.Key;
                }
            }

            // If no match, create a new one.
            var newParams = new MarkingParams {
                LaserPowerInW = power,
                LaserSpeedInMmPerS = speed,
                Name = $"P{power}W_S{speed}mmps"
            };

            int newKey = (markingParamsMap.Keys.Count == 0) ? 1 : markingParamsMap.Keys.Max() + 1;
            markingParamsMap.Add(newKey, newParams);
            return newKey;
        }
    }
}