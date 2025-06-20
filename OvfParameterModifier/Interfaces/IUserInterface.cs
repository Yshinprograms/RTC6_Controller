using OpenVectorFormat;
using System.Collections.Generic;

namespace OvfParameterModifier {
    public enum MainMenuOption {
        Unknown = 0,
        ViewParameterSets = 1,
        ApplyToLayerRange = 2,
        EditVectorBlocksInLayer = 3,
        SaveAndExit = 4
    }
    public interface IUserInterface {
        void DisplayWelcomeMessage();
        void DisplayGoodbyeMessage();
        void DisplayMessage(string message, bool isError = false);
        void DisplayParameterSets(IDictionary<int, MarkingParams> markingParamsMap);

        MainMenuOption GetMainMenuSelection();
        string GetSourceFilePath();
        string GetOutputFilePath(string defaultPath);
        (int start, int end) GetLayerRange();
        (float power, float speed) GetDesiredParameters();
        int GetTargetLayerIndex();
        (float power, float speed)? GetVectorBlockParametersOrSkip(int planeNum, int blockNum, int totalBlocks, VectorBlock block);
    }
}