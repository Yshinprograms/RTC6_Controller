// OvfParameterModifier/IUserInterface.cs

// ... (no change to using statements or enums)

using OpenVectorFormat;

namespace OvfParameterModifier {
    // ... (enums are unchanged)
    public enum MainMenuOption {
        Unknown = 0,
        ViewParameterSets = 1,
        ApplyToLayerRange = 2,
        EditVectorBlocksInLayer = 3,
        SaveAndExit = 4
    }
    public enum ParameterSource {
        CreateNew,
        UseExistingId
    }

    public interface IUserInterface {
        // ... (existing methods are unchanged)
        void DisplayWelcomeMessage();
        void DisplayGoodbyeMessage();
        void DisplayMessage(string message, bool isError = false);
        void DisplayParameterSets(IDictionary<int, MarkingParams> markingParamsMap);
        void DisplayDashboard(string filePath, string jobName, int layerCount, bool isModified);
        MainMenuOption GetMainMenuSelection();
        string GetSourceFilePath();
        string GetOutputFilePath(string defaultPath);
        (int start, int end) GetLayerRange();
        ParameterSource GetParameterSourceChoice();
        int GetExistingParameterSetId();
        (float power, float speed) GetDesiredParameters();
        int GetTargetLayerIndex();
        (float power, float speed)? GetVectorBlockParametersOrSkip(int planeNum, int blockNum, int totalBlocks, VectorBlock block);

        // New method for pausing the UI
        void WaitForAcknowledgement();
    }
}