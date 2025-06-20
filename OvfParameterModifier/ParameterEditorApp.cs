// OvfParameterModifier/ParameterEditorApp.cs

// ... (no change to using statements)

using OpenVectorFormat;
using OpenVectorFormat.OVFReaderWriter;
using OvfParameterModifier.Exceptions;

namespace OvfParameterModifier {
    public class ParameterEditorApp(IUserInterface ui, JobEditor editor) {
        // ... (no change to fields or Run method)
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
                        case MainMenuOption.ViewParameterSets:
                            ui.DisplayParameterSets(_activeJob.MarkingParamsMap);
                            // CHANGE: Add the call to wait for the user.
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
                        default:
                            ui.DisplayMessage("Invalid selection, please try again.", isError: true);
                            break;
                    }
                }
                // ... (no change to catch blocks)
                catch (UserInputException ex) {
                    ui.DisplayMessage(ex.Message, isError: true);
                } catch (Exception ex) {
                    ui.DisplayMessage(ex.Message, isError: true);
                }
            }
        }

        // ... (all other methods are unchanged)
        private void DoApplyParametersToRange() {
            int keyToUse;
            var choice = ui.GetParameterSourceChoice();

            if (choice == ParameterSource.UseExistingId) {
                keyToUse = ui.GetExistingParameterSetId();
                if (!editor.DoesParamSetExist(_activeJob, keyToUse)) {
                    ui.DisplayMessage($"Parameter Set with ID {keyToUse} does not exist.", isError: true);
                    return;
                }
            } else {
                var (power, speed) = ui.GetDesiredParameters();
                keyToUse = editor.FindOrCreateParameterSetKey(_activeJob, power, speed);
            }

            var (startLayer, endLayer) = ui.GetLayerRange();
            editor.ApplyParametersToLayerRange(_activeJob, startLayer - 1, endLayer - 1, keyToUse);

            _isModified = true;
            ui.DisplayMessage($"Successfully applied Parameter Set ID {keyToUse} to layers {startLayer}-{endLayer}.");
        }

        private void DoVectorBlockEditing() {
            int layerNumber = ui.GetTargetLayerIndex();
            int layerIndex = layerNumber - 1;

            if (layerIndex < 0 || layerIndex > editor.GetMaxLayerIndex(_activeJob)) {
                ui.DisplayMessage($"Invalid layer number. Please enter a number between 1 and {editor.GetMaxLayerIndex(_activeJob) + 1}.", isError: true);
                return;
            }

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
            }
            ui.DisplayMessage($"Finished editing Layer {layerNumber}.");
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