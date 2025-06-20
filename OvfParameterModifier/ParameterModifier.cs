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
        /// <summary>
        /// The highest level of abstraction. This method orchestrates the entire application workflow.
        /// </summary>
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
            Console.WriteLine("\nStarting interactive parameter modification...");
            foreach (var workPlane in job.WorkPlanes) {
                ProcessWorkPlane(workPlane, job.MarkingParamsMap);
            }
        }

        private void ProcessWorkPlane(WorkPlane workPlane, IDictionary<int, MarkingParams> jobMarkingParams) {
            Console.WriteLine($"\n--- Processing WorkPlane Number: {workPlane.WorkPlaneNumber} (Z-Height: {workPlane.ZPosInMm:F3} mm) ---");

            // Get the desired parameters from the user.
            var (desiredPower, desiredSpeed) = GetDesiredParametersFromUser();

            // Find an existing parameter set that matches, or create a new one if none is found.
            int keyToUse = FindOrCreateParameterSetKey(desiredPower, desiredSpeed, jobMarkingParams);

            // Update all vector blocks in this work plane to use the determined key.
            UpdateVectorBlocksInWorkPlane(workPlane, keyToUse);
        }

        // --- Low-Level Abstraction Methods ---

        #region User Interaction & Console Output

        private void DisplayWelcomeMessage() {
            Console.WriteLine("OVF Parameter Modifier Tool");
            Console.WriteLine("===========================");
        }

        private string GetSourceFilePathFromUser() {
            string? filePath = null;
            while (string.IsNullOrEmpty(filePath) || !File.Exists(filePath)) {
                Console.Write("Enter path to the SOURCE OVF file: ");
                filePath = Console.ReadLine();
                if (!File.Exists(filePath)) {
                    Console.WriteLine("ERROR: File not found. Please try again.\n" +
                        "Ensure that there are no inverted commas wrapping the filepath\n");
                }
            }
            return filePath;
        }

        private string GetOutputFilePathFromUser() {
            Console.Write("Enter path for the NEW, MODIFIED OVF file: ");
            return Console.ReadLine() ?? "output.ovf";
        }

        private (float power, float speed) GetDesiredParametersFromUser() {
            Console.Write("  Enter new Laser Power (W): ");
            float newPower = Convert.ToSingle(Console.ReadLine());

            Console.Write("  Enter new Marking Speed (mm/s): ");
            float newSpeed = Convert.ToSingle(Console.ReadLine());

            return (newPower, newSpeed);
        }

        private void DisplaySuccessMessage() {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("\nOperation completed successfully!");
            Console.ResetColor();
        }

        private void DisplayErrorMessage(Exception ex) {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"\n[FATAL ERROR] An unexpected error occurred: {ex.Message}");
            if (ex.InnerException != null) {
                Console.WriteLine($"  -> Inner Exception: {ex.InnerException.Message}");
            }
            Console.ResetColor();
        }

        /// <summary>
        /// Searches the job's parameter map for a set matching the desired values.
        /// If a match is found, its key is returned. Otherwise, a new parameter set
        /// is created, added to the map, and its new key is returned.
        /// </summary>
        /// <param name="power">Desired laser power in Watts.</param>
        /// <param name="speed">Desired marking speed in mm/s.</param>
        /// <param name="markingParamsMap">The job's central map of marking parameters.</param>
        /// <returns>The key of the matching or newly created parameter set.</returns>
        private int FindOrCreateParameterSetKey(float power, float speed, IDictionary<int, MarkingParams> markingParamsMap) {
            // A small tolerance for comparing floating-point numbers.
            const float tolerance = 0.001f;

            // 1. Search for an existing match.
            foreach (var entry in markingParamsMap) {
                bool powerMatches = Math.Abs(entry.Value.LaserPowerInW - power) < tolerance;
                bool speedMatches = Math.Abs(entry.Value.LaserSpeedInMmPerS - speed) < tolerance;

                if (powerMatches && speedMatches) {
                    Console.WriteLine($" -> Found matching Parameter Set. Reusing ID: {entry.Key}");
                    return entry.Key; // Match found, return its key.
                }
            }

            // 2. If the loop completes, no match was found. Create a new one.
            Console.WriteLine(" -> No existing match found. Creating a new Parameter Set...");
            var newParams = new MarkingParams {
                LaserPowerInW = power,
                LaserSpeedInMmPerS = speed,
                Name = $"P{power}W_S{speed}mmps" // A more generic name
            };

            // Reuse our existing method to add it and get the new key.
            int newKey = AddNewParametersToJob(newParams, markingParamsMap);
            Console.WriteLine($" -> Created new Parameter Set with ID: {newKey}");
            return newKey;
        }
        #endregion

        #region File and Data Manipulation

        private Job LoadOvfJob(string path) {
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
            try {
                Console.WriteLine("\nWriting modified job to new file...");
                // The 'using' statement ensures that writer.Dispose() is called automatically
                // at the end, which is crucial for finalizing the file.
                using (var writer = new OVFFileWriter()) {
                    // 1. Initialize the file for writing.
                    writer.StartWritePartial(job, path);

                    // 2. Loop through our in-memory work planes and append them one by one.
                    foreach (var workPlane in job.WorkPlanes) {
                        writer.AppendWorkPlane(workPlane);
                    }
                } // 3. writer.Dispose() is automatically called here, finalizing the file write.
            } catch (Exception ex) {
                throw new OvfParameterModifierException($"Failed to write the OVF file to '{path}'.", ex);
            }
        }

        private int AddNewParametersToJob(MarkingParams newParams, IDictionary<int, MarkingParams> markingParamsMap) {
            int newKey = (markingParamsMap.Keys.Count == 0) ? 1 : markingParamsMap.Keys.Max() + 1;
            markingParamsMap.Add(newKey, newParams);
            return newKey;
        }

        private void UpdateVectorBlocksInWorkPlane(WorkPlane workPlane, int newKey) {
            foreach (var vectorBlock in workPlane.VectorBlocks) {
                vectorBlock.MarkingParamsKey = newKey;
            }
        }

        #endregion
    }
}