// OvfParameterModifier/ConsoleUI.cs

// ... (no change to using statements)

using OpenVectorFormat;
using OvfParameterModifier.Exceptions;

namespace OvfParameterModifier {
    public class ConsoleUI : IUserInterface {
        // ... (all existing methods up to this point are unchanged)

        // New method implementation
        public void WaitForAcknowledgement() {
            Console.WriteLine("\nPress Enter to return to the Main Menu...");
            Console.ReadLine();
        }

        // ... (all other methods are unchanged)
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
        public ParameterSource GetParameterSourceChoice() {
            Console.WriteLine("\nHow do you want to specify the parameters?");
            Console.WriteLine("  1. Create New (by entering Power and Speed)");
            Console.WriteLine("  2. Use Existing (by entering a Parameter Set ID)");
            Console.Write("Select an option: ");

            string input = Console.ReadLine() ?? "";
            return input == "2" ? ParameterSource.UseExistingId : ParameterSource.CreateNew;
        }

        public int GetExistingParameterSetId() {
            return GetIntegerInput("\nEnter the ID of the existing Parameter Set to apply: ");
        }
        public void DisplayDashboard(string filePath, string jobName, int layerCount, bool isModified) {
            Console.Clear();
            Console.WriteLine("================================================================");
            Console.WriteLine("OVF Interactive Parameter Editor");
            Console.WriteLine("----------------------------------------------------------------");
            Console.WriteLine($"File:     {filePath}");
            Console.WriteLine($"Job Name: {jobName}");
            Console.WriteLine($"Layers:   {layerCount} (numbered 1 to {layerCount})");

            string status = isModified ? "Modified" : "Unchanged";
            ConsoleColor statusColor = isModified ? ConsoleColor.Yellow : ConsoleColor.Green;

            Console.Write("Status:   ");
            Console.ForegroundColor = statusColor;
            Console.WriteLine(status);
            Console.ResetColor();
            Console.WriteLine("================================================================");
        }
        public string GetSourceFilePath() {
            string? filePath = null;
            while (string.IsNullOrEmpty(filePath) || !File.Exists(filePath)) {
                Console.Write("\nEnter path to the SOURCE OVF file to edit: ");
                filePath = Console.ReadLine() ?? "";
                if (!File.Exists(filePath)) {
                    DisplayMessage("ERROR: File not found. Please try again.", isError: true);
                }
            }
            return filePath;
        }

        public string GetOutputFilePath(string defaultPath) {
            Console.Write($"\nEnter path for the new file (or press Enter to use '{defaultPath}'): ");
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
            Console.WriteLine("\nEnter the layer range to modify (e.g., 1, 2, 3...).");
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

        public int GetTargetLayerIndex() {
            return GetIntegerInput("\nEnter the Layer number (e.g., 1, 2, 3...) you wish to edit: ");
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
        public int GetIntegerInput(string prompt) {
            Console.Write(prompt);
            if (!int.TryParse(Console.ReadLine() ?? "", out int result)) {
                throw new UserInputException("Invalid input. Please enter a valid integer.");
            }
            return result;
        }

        private float GetFloatInput(string prompt) {
            Console.Write(prompt);
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