// OvfParameterModifier/IUserInterface.cs

using OpenVectorFormat;
using System.Collections.Generic;

namespace OvfParameterModifier {
    public enum MainMenuOption {
        Unknown = 0,
        ViewParameterSets = 1,
        ApplyToLayerRange = 2,
        EditVectorBlocksInLayer = 3,
        ChangeJobName = 4,         // New Option
        DiscardChanges = 5,        // Shifted
        SaveAndExit = 6,           // Shifted
        QuitWithoutSaving = 7      // Shifted
    }

    // ... (ParameterSource enum is unchanged)
    public enum ParameterSource {
        CreateNew,
        UseExistingId,
        ReturnToMenu
    }

    public interface IUserInterface {
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
        int GetExistingParameterSetId(IEnumerable<int> availableKeys);
        (float power, float speed) GetDesiredParameters();
        int GetTargetLayerIndex();
        (float power, float speed)? GetVectorBlockParametersOrSkip(int planeNum, int blockNum, int totalBlocks, VectorBlock block);
        void WaitForAcknowledgement();
        bool ConfirmQuitWithoutSaving();
        bool ConfirmDiscardChanges();

        // New method for getting the job name
        string GetNewJobName(string currentName);
    }
}