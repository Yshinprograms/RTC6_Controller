// OvfParameterModifier/ParameterEditorApp.cs

using OpenVectorFormat;
using OpenVectorFormat.OVFReaderWriter;
using OvfParameterModifier.Exceptions;
using System;
using System.IO;

//C:\Users\pin20\Downloads\SIMTech_Internship\RTC6_Controller\RTC6_Controller\x64\Debug\valid_3_layers.ovf

namespace OvfParameterModifier {
    /// <summary>
    /// The main application controller, using a C# 12 Primary Constructor.
    /// The 'ui' and 'editor' parameters are available throughout the class.
    /// </summary>
    public class ParameterEditorApp(IUserInterface ui, JobEditor editor) {
        // These fields represent the state of the application.
        // We use the null-forgiving operator (!) to tell the compiler that
        // we guarantee these will be initialized in LoadJob() before ever being read.
        private Job _activeJob = null!;
        private string _sourceFilePath = null!;

        public void Run() {
            ui.DisplayWelcomeMessage();
            try {
                LoadJob();
                MainLoop();
            } catch (Exception ex) {
                ui.DisplayMessage($"A fatal error occurred: {ex.Message}", isError: true);
            }
        }

        private void MainLoop() {
            bool running = true;
            while (running) {
                try {
                    var choice = ui.GetMainMenuSelection();
                    switch (choice) {
                        case MainMenuOption.ViewParameterSets:
                            ui.DisplayParameterSets(_activeJob.MarkingParamsMap);
                            break;
                        case MainMenuOption.ApplyToLayerRange:
                            DoApplyParametersToRange();
                            break;
                        case MainMenuOption.EditVectorBlocksInLayer:
                            DoVectorBlockEditing();
                            break;
                        case MainMenuOption.SaveAndExit:
                            DoSaveAndExit();
                            running = false;
                            break;
                        default:
                            ui.DisplayMessage("Invalid selection, please try again.", isError: true);
                            break;
                    }
                } catch (UserInputException ex) {
                    ui.DisplayMessage(ex.Message, isError: true);
                }
            }
        }

        private void DoApplyParametersToRange() {
            var (start, end) = ui.GetLayerRange();
            var (power, speed) = ui.GetDesiredParameters();

            int keyToUse = editor.FindOrCreateParameterSetKey(_activeJob, power, speed);
            editor.ApplyParametersToLayerRange(_activeJob, start, end, keyToUse);

            ui.DisplayMessage($"Successfully applied Parameter Set ID {keyToUse} to layers {start}-{end}.");
        }

        private void DoVectorBlockEditing() {
            int layerIndex = ui.GetTargetLayerIndex();

            // Validate the requested layer index
            if (layerIndex < 0 || layerIndex >= _activeJob.WorkPlanes.Count) {
                ui.DisplayMessage($"Invalid layer index. Please enter a number between 0 and {_activeJob.WorkPlanes.Count - 1}.", isError: true);
                return;
            }

            var workPlane = _activeJob.WorkPlanes[layerIndex];

            for (int i = 0; i < workPlane.VectorBlocks.Count; i++) {
                var block = workPlane.VectorBlocks[i];
                var desiredParams = ui.GetVectorBlockParametersOrSkip(layerIndex, i + 1, workPlane.VectorBlocks.Count, block);

                if (desiredParams.HasValue) {
                    var (power, speed) = desiredParams.Value;
                    int keyToUse = editor.FindOrCreateParameterSetKey(_activeJob, power, speed);
                    block.MarkingParamsKey = keyToUse;
                }
            }
            ui.DisplayMessage($"Finished editing Layer {layerIndex}.");
        }

        private void LoadJob() {
            _sourceFilePath = ui.GetSourceFilePath();
            using (var reader = new OVFFileReader()) {
                reader.OpenJob(_sourceFilePath);
                _activeJob = reader.CacheJobToMemory();
                ui.DisplayMessage($"Successfully loaded '{_activeJob.JobMetaData.JobName}' with {_activeJob.WorkPlanes.Count} work planes.");
            }
        }

        private void DoSaveAndExit() {
            // Use the stored _sourceFilePath to create a sensible default name.
            string defaultPath = Path.ChangeExtension(_sourceFilePath, ".modified.ovf");
            string outputPath = ui.GetOutputFilePath(defaultPath);

            using (var writer = new OVFFileWriter()) {
                // The library's SimpleJobWrite has a bug, so we write explicitly.
                writer.StartWritePartial(_activeJob, outputPath);
                foreach (var workPlane in _activeJob.WorkPlanes) {
                    writer.AppendWorkPlane(workPlane);
                }
            }
            ui.DisplayGoodbyeMessage();
        }
    }
}