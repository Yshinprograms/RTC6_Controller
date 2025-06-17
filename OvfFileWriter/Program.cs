// These 'using' statements are like #includes in C++. They tell the compiler
// which namespaces to look in to find the classes we want to use.
using OpenVectorFormat.OVFReaderWriter; // For OVFFileWriter
using OpenVectorFormat;                 // For Job, WorkPlane, VectorBlock, etc.
using System;                           // For Console, args, Exception
using System.IO;                        // For Path

// The main entry point for our command-line tool.
public class Program {
    public static int Main(string[] args) {
        // 1. VALIDATE ARGUMENTS
        // We expect exactly two arguments: the output path and the number of layers.
        if (args.Length != 2) {
            Console.Error.WriteLine("Error: Invalid arguments.");
            Console.Error.WriteLine("Usage: OvfTestFileGenerator.exe <output_path> <num_layers>");
            return 1; // Return an error code
        }

        string outputPath = args[0];
        if (!int.TryParse(args[1], out int numLayers)) {
            Console.Error.WriteLine($"Error: Could not parse number of layers: {args[1]}");
            return 1;
        }

        try {
            Console.WriteLine($"Generating OVF file at: {outputPath} with {numLayers} layers...");

            // 2. CREATE THE JOB SHELL
            // This object holds the high-level metadata for the file.
            var jobShell = new Job();
            jobShell.JobMetaData = new Job.Types.JobMetaData { JobName = "TestJob" };

            // 3. USE THE FILE WRITER
            // The 'using' statement is the C# way to guarantee an object is disposed of
            // correctly. When the code leaves this block, writer.Dispose() is called
            // automatically. This is the exact same principle as our C++ RAII destructor!
            using (var writer = new OVFFileWriter()) {
                // 3a. Start the writing process. This creates the file and writes the header.
                writer.StartWritePartial(jobShell, outputPath);

                // 3b. Loop to create and append each layer.
                for (int i = 0; i < numLayers; i++) {
                    // Create the WorkPlane object in memory.
                    var p = new WorkPlane {
                        WorkPlaneNumber = i,
                        ZPosInMm = i * 0.05f
                    };

                    // Add a VectorBlock with a simple line sequence to the WorkPlane.
                    var block = new VectorBlock();
                    block.LineSequence = new VectorBlock.Types.LineSequence();
                    block.LineSequence.Points.Add(i * 10.0f);
                    block.LineSequence.Points.Add(i * 15.0f);
                    p.VectorBlocks.Add(block);

                    // Append the fully-formed WorkPlane to the file.
                    writer.AppendWorkPlane(p);
                    Console.WriteLine($"  - Appended WorkPlane {i}");
                }
            } // 3c. writer.Dispose() is automatically called here, which finalizes the file.

            Console.WriteLine("Successfully generated OVF file.");
            return 0; // Return 0 for success
        } catch (Exception ex) {
            Console.Error.WriteLine($"An unexpected error occurred: {ex.Message}");
            Console.Error.WriteLine(ex.StackTrace); // Print detailed error info
            return 1;
        }
    }
}