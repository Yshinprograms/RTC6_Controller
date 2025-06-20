// OvfParameterModifier/ParameterModifier.cs

using OpenVectorFormat;
using OpenVectorFormat.OVFReaderWriter;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace OvfParameterModifier {
    /// <summary>
    /// Encapsulates the entire logic for the OVF Parameter Modifier tool.
    /// </summary>
    public class ParameterModifier {
        // --- High-Level Orchestration ---
        public void Run() {
            DisplayWelcomeMessage();
            try {
                string sourcePath = GetSourceFilePathFromUser();
                string outputPath = GetOutputFilePathFromUser();

                Job job = LoadOvfJob(sourcePath);
                ModifyJobInteractively(job);
                SaveOvfJob(outputPath, job);

                DisplaySuccessMessage();
            } catch (Exception ex) {
                DisplayErrorMessage(ex);
            }
        }

        // --- Mid-Level Abstraction Methods ---

        private void ModifyJobInteractively(Job job) {
            Console.WriteLine("\nStarting interactive parameter modification on a per-vector-block basis.");
            Console.WriteLine("For each block, enter new parameters or press Enter to skip and keep the existing ones.");

            foreach (var workPlane in job.WorkPlanes) {
                ProcessVectorBlocksInWorkPlane(workPlane, job.MarkingParamsMap);
            }
        }

        /// <summary>
        /// Manages the iteration over the vector blocks within a single work plane.
        /// </summary>
        private void ProcessVectorBlocksInWorkPlane(WorkPlane workPlane, IDictionary<int, MarkingParams> jobMarkingParams) {
            Console.WriteLine($"\n--- Processing WorkPlane Number: {workPlane.WorkPlaneNumber} (Z-Height: {workPlane.ZPosInMm:F3} mm) ---");
            for (int i = 0; i < workPlane.VectorBlocks.Count; i++) {
                VectorBlock vectorBlock = workPlane.VectorBlocks[i];
                ProcessSingleVectorBlock(vectorBlock, workPlane.WorkPlaneNumber, i + 1, workPlane.VectorBlocks.Count, jobMarkingParams);
            }
        }

        /// <summary>
        /// Handles the logic for a single vector block: getting user input and updating the block.
        /// </summary>
        private void ProcessSingleVectorBlock(VectorBlock block, int planeNum, int blockNum, int totalBlocks, IDictionary<int, MarkingParams> jobMarkingParams) {
            // Get the desired parameters. This method now returns a nullable tuple to handle the "skip" case.
            var desiredParams = GetDesiredParametersFromUser(planeNum, blockNum, totalBlocks, block);

            // If the user entered values (did not skip)
            if (desiredParams.HasValue) {
                var (power, speed) = desiredParams.Value;

                // Find an existing parameter set that matches, or create a new one.
                int keyToUse = FindOrCreateParameterSetKey(power, speed, jobMarkingParams);

                // Update this specific vector block to use the determined key.
                block.MarkingParamsKey = keyToUse;
            }
            // If desiredParams is null, the user skipped, so we do nothing.
        }

        // --- Low-Level Abstraction Methods ---

        #region User Interaction & Console Output

        private void DisplayWelcomeMessage() {
            Console.WriteLine("OVF Parameter Modifier Tool");
            Console.WriteLine("===========================");
        }

        private string GetSourceFilePathFromUser() {
            // ... (This method is unchanged)
            string? filePath = null;
            while (string.IsNullOrEmpty(filePath) || !File.Exists(filePath)) {
                Console.Write("Enter path to the SOURCE OVF file: ");
                filePath = Console.ReadLine();
                if (!File.Exists(filePath)) {
                    Console.WriteLine("ERROR: File not found. Please try again.");
                }
            }
            return filePath;
        }

        private string GetOutputFilePathFromUser() {
            // ... (This method is unchanged)
            Console.Write("Enter path for the NEW, MODIFIED OVF file: ");
            return Console.ReadLine() ?? "output.ovf";
        }

        /// <summary>
        /// Prompts the user for parameters for a specific vector block.
        /// Returns a nullable tuple. If the user skips, returns null.
        /// </summary>
        private (float power, float speed)? GetDesiredParametersFromUser(int planeNum, int blockNum, int totalBlocks, VectorBlock block) {
            Console.WriteLine($"\nEditing Plane {planeNum}, Vector Block {blockNum}/{totalBlocks} (Type: {block.VectorDataCase}, Current Key: {block.MarkingParamsKey})");
            Console.Write("  Enter new Laser Power (W) or press Enter to skip: ");
            string powerInput = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(powerInput)) {
                Console.WriteLine("  -> Skipped.");
                return null;
            }

            Console.Write("  Enter new Marking Speed (mm/s): ");
            string speedInput = Console.ReadLine();

            if (float.TryParse(powerInput, out float power) && float.TryParse(speedInput, out float speed)) {
                return (power, speed);
            }

            Console.WriteLine("  -> Invalid input. Skipping block.");
            return null;
        }

        private void DisplaySuccessMessage() {
            // ... (This method is unchanged)
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("\nOperation completed successfully!");
            Console.ResetColor();
        }

        private void DisplayErrorMessage(Exception ex) {
            // ... (This method is unchanged)
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\n[FATAL ERROR] An unexpected error occurred: {ex.Message}");
            if (ex.InnerException != null) {
                Console.WriteLine($"  -> Inner Exception: {ex.InnerException.Message}");
            }
            Console.ResetColor();
        }

        #endregion

        #region File and Data Manipulation

        private Job LoadOvfJob(string path) {
            // ... (This method is unchanged)
            try {
                Console.WriteLine("\nReading and caching source file...");
                using (var reader = new OVFFileReader()) {
                    reader.OpenJob(path);
                    Job job = reader.CacheJobToMemory();
                    Console.WriteLine($"Successfully read job '{job.JobMetaData.JobName}' with {job.WorkPlanes.Count} work planes.");
                    return job;
                }
            } catch (Exception ex) {
                throw new OvfParameterModifierException($"Failed to load or parse the OVF file at '{path}'.", ex);
            }
        }

        private void SaveOvfJob(string path, Job job) {
            // ... (This method is unchanged)
            try {
                Console.WriteLine("\nWriting modified job to new file...");
                using (var writer = new OVFFileWriter()) {
                    writer.StartWritePartial(job, path);
                    foreach (var workPlane in job.WorkPlanes) {
                        writer.AppendWorkPlane(workPlane);
                    }
                }
            } catch (Exception ex) {
                throw new OvfParameterModifierException($"Failed to write the OVF file to '{path}'.", ex);
            }
        }

        private int FindOrCreateParameterSetKey(float power, float speed, IDictionary<int, MarkingParams> markingParamsMap) {
            // ... (This method is unchanged and still perfectly reusable)
            const float tolerance = 0.001f;
            foreach (var entry in markingParamsMap) {
                bool powerMatches = Math.Abs(entry.Value.LaserPowerInW - power) < tolerance;
                bool speedMatches = Math.Abs(entry.Value.LaserSpeedInMmPerS - speed) < tolerance;
                if (powerMatches && speedMatches) {
                    Console.WriteLine($"  -> Found matching Parameter Set. Reusing ID: {entry.Key}");
                    return entry.Key;
                }
            }
            Console.WriteLine("  -> No existing match found. Creating a new Parameter Set...");
            var newParams = new MarkingParams {
                LaserPowerInW = power,
                LaserSpeedInMmPerS = speed,
                Name = $"P{power}W_S{speed}mmps"
            };
            int newKey = AddNewParametersToJob(newParams, markingParamsMap);
            Console.WriteLine($"  -> Created new Parameter Set with ID: {newKey}");
            return newKey;
        }

        private int AddNewParametersToJob(MarkingParams newParams, IDictionary<int, MarkingParams> markingParamsMap) {
            // ... (This method is unchanged)
            int newKey = (markingParamsMap.Keys.Count == 0) ? 1 : markingParamsMap.Keys.Max() + 1;
            markingParamsMap.Add(newKey, newParams);
            return newKey;
        }

        #endregion
    }
}