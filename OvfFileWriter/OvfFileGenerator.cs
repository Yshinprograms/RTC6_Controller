using System;
using System.IO;
using OpenVectorFormat;
using OpenVectorFormat.OVFReaderWriter;
using OvfFileWriter.Exceptions;

namespace OvfFileWriter {
    internal class OvfFileGenerator {
        private const float SQUARE_SIZE = 20.0f; // Size of the square in mm
        private const float HATCH_SPACING = 1.0f; // Spacing between hatches in mm
        private const float LAYER_OFFSET = 1.0f; // Offset between square layers in mm

        private readonly struct SquareDefinition {
            public float CenterX { get; }
            public float CenterY { get; }
            public float Size { get; }

            public float XMin => CenterX - Size / 2;
            public float XMax => CenterX + Size / 2;
            public float YMin => CenterY - Size / 2;
            public float YMax => CenterY + Size / 2;

            public SquareDefinition(float centerX, float centerY, float size) {
                CenterX = centerX;
                CenterY = centerY;
                Size = size;
            }
        }

        public void Generate(ParsedArguments args) {
            try {
                using (var writer = new OVFFileWriter()) {
                    Job jobShell = CreateJobShell();

                    writer.StartWritePartial(jobShell, args.OutputFilePath);
                    Console.WriteLine("Started writing OVF file...");

                    for (int i = 0; i < args.NumberOfLayers; i++) {
                        Console.WriteLine($"  - Creating geometry for WorkPlane {i}...");
                        WorkPlane plane = CreateWorkPlaneForLayer(i);

                        writer.AppendWorkPlane(plane);
                        Console.WriteLine($"  - Appended WorkPlane {i}.");
                    }
                }
                Console.WriteLine("Successfully generated OVF file.");
            } catch (Exception ex) {
                throw new OvfFileGeneratorException($"Failed during OVF file generation: {ex.Message}", ex);
            }
        }

        //private Job CreateJobShell() {
        //    Job job = new Job();
        //    job.JobMetaData = CreateDefaultMetaData();
        //    return job;
        //}
        private Job CreateJobShell() {
            var job = new Job();
            job.JobMetaData = CreateDefaultMetaData();

            // --- THIS IS THE FIX ---
            // We must add at least one set of marking parameters to the map
            // so that the key '0' exists for the vector blocks to look up.
            var defaultParams = new MarkingParams {
                Name = "Default",
                LaserPowerInW = 200.0f,
                LaserSpeedInMmPerS = 1500.0f,
                JumpSpeedInMmS = 5000.0f
            };

            // Add the new parameter set to the map with key 0.
            job.MarkingParamsMap[0] = defaultParams;

            return job;
        }

        private Job.Types.JobMetaData CreateDefaultMetaData() {
            return new Job.Types.JobMetaData {
                JobName = "Hatched Square Job",
                Description = "Test file with a hatched square on each layer",
                Version = 1
            };
        }

        private WorkPlane CreateWorkPlaneForLayer(int layerIndex) {
            WorkPlane workPlane = new WorkPlane {
                WorkPlaneNumber = layerIndex,
                ZPosInMm = layerIndex * 0.1f
            };

            SquareDefinition squareDefinition = new SquareDefinition(
                centerX: 0.0f,
                centerY: 0.0f,
                size: SQUARE_SIZE
            );

            List<VectorBlock> vectorBlocks = CreateHatchedSquareGeometry(squareDefinition);

            workPlane.VectorBlocks.AddRange(vectorBlocks);
            return workPlane;
        }

        List<VectorBlock> CreateHatchedSquareGeometry(SquareDefinition squareDefinition) {
            VectorBlock contourBlock = CreateContourBlock(squareDefinition);
            VectorBlock hatchBlock = CreateHatchBlock(squareDefinition, HATCH_SPACING);

            return new List<VectorBlock> {
                contourBlock,
                hatchBlock
            };
        }

        private VectorBlock CreateContourBlock(SquareDefinition squareDefinition) {
            VectorBlock block = new VectorBlock();
            VectorBlock.Types.LineSequence square = new VectorBlock.Types.LineSequence();

            square.Points.Add(squareDefinition.XMin); square.Points.Add(squareDefinition.YMin);
            square.Points.Add(squareDefinition.XMax); square.Points.Add(squareDefinition.YMin);
            square.Points.Add(squareDefinition.XMax); square.Points.Add(squareDefinition.YMax);
            square.Points.Add(squareDefinition.XMin); square.Points.Add(squareDefinition.YMax);
            square.Points.Add(squareDefinition.XMin); square.Points.Add(squareDefinition.YMin);

            block.LineSequence = square;
            return block;
        }

        private VectorBlock CreateHatchBlock(SquareDefinition squareDefinition, float spacing) {
            VectorBlock block = new VectorBlock();
            VectorBlock.Types.Hatches hatches = new VectorBlock.Types.Hatches();

            for (float y = squareDefinition.YMin; y <= squareDefinition.YMax; y += spacing) {
                hatches.Points.Add(squareDefinition.XMin);
                hatches.Points.Add(y);
                hatches.Points.Add(squareDefinition.XMax);
                hatches.Points.Add(y);
            }

            block.Hatches = hatches;
            return block;
        }
    }
}
