// OvfParameterModifier/ParameterEditorApp.cs

// ... (keep all existing using statements)

using OpenVectorFormat;
using OpenVectorFormat.OVFReaderWriter;
using OvfParameterModifier.Exceptions;

namespace OvfParameterModifier {
    public class ParameterEditorApp(IUserInterface ui, JobEditor editor) {
        // ... (fields are unchanged)
        private Job _activeJob = null!;
        private string _sourceFilePath = null!;
        private bool _isModified = false;
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
                ui.DisplayDashboard(_sourceFilePath, _activeJob.JobMetaData.JobName, _activeJob.WorkPlanes.Count, _isModified);

                try {
                    var choice = ui.GetMainMenuSelection();
                    switch (choice) {
                        // ... (cases 1, 2, 3 are unchanged)
                        case MainMenuOption.ViewParameterSets:
                            ui.DisplayParameterSets(_activeJob.MarkingParamsMap);
                            ui.WaitForAcknowledgement();
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
                        // New case for quitting
                        case MainMenuOption.QuitWithoutSaving:
                            running = !DoQuitWithoutSaving(); // running is false if quit is confirmed
                            break;
                        default:
                            ui.DisplayMessage("Invalid selection, please try again.", isError: true);
                            ui.WaitForAcknowledgement();
                            break;
                    }
                } catch (UserInputException ex) {
                    ui.DisplayMessage(ex.Message, isError: true);
                    ui.WaitForAcknowledgement();
                } catch (Exception ex) {
                    ui.DisplayMessage(ex.Message, isError: true);
                    ui.WaitForAcknowledgement();
                }
            }
        }

        private void DoApplyParametersToRange() {
            int keyToUse;
            var choice = ui.GetParameterSourceChoice();

            if (choice == ParameterSource.UseExistingId) {
                // CHANGE: Pass the available keys to the UI method.
                keyToUse = ui.GetExistingParameterSetId(_activeJob.MarkingParamsMap.Keys);
                if (!editor.DoesParamSetExist(_activeJob, keyToUse)) {
                    ui.DisplayMessage($"Parameter Set with ID {keyToUse} does not exist.", isError: true);
                    ui.WaitForAcknowledgement();
                    return;
                }
            } else // Create New
              {
                var (power, speed) = ui.GetDesiredParameters();
                keyToUse = editor.FindOrCreateParameterSetKey(_activeJob, power, speed);
            }

            var (startLayer, endLayer) = ui.GetLayerRange();
            int maxLayer = editor.GetMaxLayerIndex(_activeJob) + 1;

            if (startLayer < 1 || endLayer > maxLayer || startLayer > endLayer) {
                ui.DisplayMessage($"Invalid layer range. Please enter numbers between 1 and {maxLayer}.", isError: true);
                ui.WaitForAcknowledgement();
                return;
            }

            editor.ApplyParametersToLayerRange(_activeJob, startLayer - 1, endLayer - 1, keyToUse);

            _isModified = true;
            ui.DisplayMessage($"Successfully applied Parameter Set ID {keyToUse} to layers {startLayer}-{endLayer}.");
            ui.WaitForAcknowledgement();
        }

        // New method to handle the quit logic
        private bool DoQuitWithoutSaving() {
            if (_isModified) {
                if (ui.ConfirmQuitWithoutSaving()) {
                    ui.DisplayMessage("Exiting without saving changes. Goodbye!");
                    return true; // Yes, quit
                }
                return false; // No, don't quit
            }

            // If not modified, just exit.
            ui.DisplayMessage("Exiting application. Goodbye!");
            return true;
        }

        // ... (DoVectorBlockEditing, LoadJob, DoSaveAndExit are unchanged)
        private void DoVectorBlockEditing() {
            int layerNumber = ui.GetTargetLayerIndex();
            int maxLayer = editor.GetMaxLayerIndex(_activeJob) + 1;

            if (layerNumber < 1 || layerNumber > maxLayer) {
                ui.DisplayMessage($"Invalid layer number. Please enter a number between 1 and {maxLayer}.", isError: true);
                ui.WaitForAcknowledgement();
                return;
            }

            int layerIndex = layerNumber - 1;
            var workPlane = _activeJob.WorkPlanes[layerIndex];
            bool wasAnyBlockModified = false;

            for (int i = 0; i < workPlane.VectorBlocks.Count; i++) {
                var block = workPlane.VectorBlocks[i];
                var desiredParams = ui.GetVectorBlockParametersOrSkip(layerNumber, i + 1, workPlane.VectorBlocks.Count, block);

                if (desiredParams.HasValue) {
                    var (power, speed) = desiredParams.Value;
                    int keyToUse = editor.FindOrCreateParameterSetKey(_activeJob, power, speed);
                    block.MarkingParamsKey = keyToUse;
                    wasAnyBlockModified = true;
                }
            }

            if (wasAnyBlockModified) {
                _isModified = true;
                ui.DisplayMessage($"Changes applied successfully to Layer {layerNumber}.");
            } else {
                ui.DisplayMessage($"No changes were made to Layer {layerNumber}.");
            }
            ui.WaitForAcknowledgement();
        }
        private void LoadJob() {
            _sourceFilePath = ui.GetSourceFilePath();
            using (var reader = new OVFFileReader()) {
                reader.OpenJob(_sourceFilePath);
                _activeJob = reader.CacheJobToMemory();
                ui.DisplayMessage($"Successfully loaded '{_activeJob.JobMetaData.JobName}' with {_activeJob.WorkPlanes.Count} work planes.");
                ui.WaitForAcknowledgement();
            }
        }

        private void DoSaveAndExit() {
            string defaultPath = Path.ChangeExtension(_sourceFilePath, ".modified.ovf");
            string outputPath = ui.GetOutputFilePath(defaultPath);

            using (var writer = new OVFFileWriter()) {
                writer.StartWritePartial(_activeJob, outputPath);
                foreach (var workPlane in _activeJob.WorkPlanes) {
                    writer.AppendWorkPlane(workPlane);
                }
            }
            ui.DisplayGoodbyeMessage();
        }
    }
}