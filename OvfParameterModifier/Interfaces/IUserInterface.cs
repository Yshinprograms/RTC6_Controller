// OvfParameterModifier/IUserInterface.cs

using OpenVectorFormat;
using System.Collections.Generic;

namespace OvfParameterModifier {
    public enum MainMenuOption {
        Unknown = 0,
        ViewParameterSets = 1,
        ApplyToLayerRange = 2,
        EditVectorBlocksInLayer = 3,
        SaveAndExit = 4,
        DiscardChanges = 5,
        QuitWithoutSaving = 6
    }

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
    }
}