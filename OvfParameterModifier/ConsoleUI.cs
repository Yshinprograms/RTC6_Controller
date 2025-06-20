// OvfParameterModifier/ConsoleUI.cs

using OpenVectorFormat;
using OvfParameterModifier.Exceptions;
using System;
using System.Collections.Generic;
using System.IO;

namespace OvfParameterModifier {
    public class ConsoleUI : IUserInterface {
        public void DisplayWelcomeMessage() {
            Console.WriteLine("OVF Interactive Parameter Editor");
            Console.WriteLine("==============================");
        }

        public void DisplayGoodbyeMessage() {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("\nFile saved. Goodbye!");
            Console.ResetColor();
        }

        public MainMenuOption GetMainMenuSelection() {
            Console.WriteLine("\n--- Main Menu ---");
            Console.WriteLine("1. View existing Parameter Sets");
            Console.WriteLine("2. Apply Parameters to a Layer Range");
            Console.WriteLine("3. Edit Individual Vector Blocks in a Layer");
            Console.WriteLine("4. Save and Exit");
            Console.Write("Select an option: ");

            string input = Console.ReadLine() ?? "";
            if (int.TryParse(input, out int choice) && Enum.IsDefined(typeof(MainMenuOption), choice)) {
                return (MainMenuOption)choice;
            }
            return MainMenuOption.Unknown;
        }

        public int GetTargetLayerIndex() {
            return GetIntegerInput("\nEnter the Layer number you wish to edit: ");
        }

        public (float power, float speed)? GetVectorBlockParametersOrSkip(int planeNum, int blockNum, int totalBlocks, VectorBlock block) {
            Console.WriteLine($"\nEditing Plane {planeNum}, Vector Block {blockNum}/{totalBlocks} (Type: {block.VectorDataCase}, Current Key: {block.MarkingParamsKey})");
            Console.Write("  Enter new Laser Power (W) or press Enter to skip: ");
            string powerInput = Console.ReadLine() ?? "";

            if (string.IsNullOrWhiteSpace(powerInput)) {
                Console.WriteLine("  -> Skipped.");
                return null;
            }

            Console.Write("  Enter new Marking Speed (mm/s): ");
            string speedInput = Console.ReadLine() ?? "";

            if (float.TryParse(powerInput, out float power) && float.TryParse(speedInput, out float speed)) {
                return (power, speed);
            }

            DisplayMessage("Invalid input. Skipping block.", isError: true);
            return null;
        }

        public string GetSourceFilePath() {
            string? filePath = null;
            while (string.IsNullOrEmpty(filePath) || !File.Exists(filePath)) {
                Console.Write("\nEnter path to the SOURCE OVF file to edit: ");
                // FIX: Handle potential null return from ReadLine.
                filePath = Console.ReadLine() ?? "";
                if (!File.Exists(filePath)) {
                    DisplayMessage("ERROR: File not found. Please try again.", isError: true);
                }
            }
            return filePath;
        }

        public string GetOutputFilePath(string defaultPath) {
            Console.Write($"\nEnter path for the new file (or press Enter to use '{defaultPath}'): ");
            // FIX: Handle potential null return from ReadLine.
            string inputPath = Console.ReadLine() ?? "";
            return string.IsNullOrWhiteSpace(inputPath) ? defaultPath : inputPath;
        }

        public void DisplayParameterSets(IDictionary<int, MarkingParams> markingParamsMap) {
            Console.WriteLine("\n--- Existing Parameter Sets ---");
            if (markingParamsMap.Count == 0) {
                Console.WriteLine("No parameter sets found.");
                return;
            }

            foreach (var entry in markingParamsMap) {
                Console.WriteLine($"  ID: {entry.Key}, Name: '{entry.Value.Name}', Power: {entry.Value.LaserPowerInW} W, Speed: {entry.Value.LaserSpeedInMmPerS} mm/s");
            }
        }

        public (int start, int end) GetLayerRange() {
            Console.WriteLine("\nEnter the layer range to modify (inclusive).");
            int start = GetIntegerInput("  Start Layer: ");
            int end = GetIntegerInput("  End Layer: ");

            if (start > end) {
                throw new UserInputException("The start layer cannot be greater than the end layer.");
            }
            return (start, end);
        }

        public (float power, float speed) GetDesiredParameters() {
            Console.WriteLine("Enter the new parameters to apply.");
            float power = GetFloatInput("  New Laser Power (W): ");
            float speed = GetFloatInput("  New Marking Speed (mm/s): ");
            return (power, speed);
        }

        public int GetIntegerInput(string prompt) {
            Console.Write(prompt);
            // FIX: Handle potential null return from ReadLine.
            if (!int.TryParse(Console.ReadLine() ?? "", out int result)) {
                throw new UserInputException("Invalid input. Please enter a valid integer.");
            }
            return result;
        }

        private float GetFloatInput(string prompt) {
            Console.Write(prompt);
            // FIX: Handle potential null return from ReadLine.
            if (!float.TryParse(Console.ReadLine() ?? "", out float result)) {
                throw new UserInputException("Invalid input. Please enter a valid number.");
            }
            return result;
        }

        public void DisplayMessage(string message, bool isError = false) {
            if (isError) {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"\nERROR: {message}");
                Console.ResetColor();
            } else {
                Console.ForegroundColor = ConsoleColor.Cyan;
                Console.WriteLine($"\nINFO: {message}");
                Console.ResetColor();
            }
        }
    }
}