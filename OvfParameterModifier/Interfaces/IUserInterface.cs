using OpenVectorFormat;
using System.Collections.Generic;

namespace OvfParameterModifier {
    public enum MainMenuOption {
        Unknown = 0,
        ViewParameterSets = 1,
        ApplyToLayerRange = 2,
        SaveAndExit = 3
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
    }
}