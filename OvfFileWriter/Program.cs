using OpenVectorFormat.OVFReaderWriter;
using OpenVectorFormat;
using OvfFileWriter;
using System;                          
using System.IO;

// The main entry point for our command-line tool.
public class Program {
    public static int Main(string[] args) {
        try {
            ParsedArguments parsedArgs = CommandLineHandler.Parse(args);
            Console.WriteLine("Command-line arguments parsed successfully.");
            Console.WriteLine($"  - Output Path: {parsedArgs.OutputFilePath}");
            Console.WriteLine($"  - Layers: {parsedArgs.NumberOfLayers}");

            OvfFileGenerator generator = new OvfFileGenerator();
            generator.Generate( parsedArgs );

            return 0;
        }
        catch (OvfFileWriterException ex) {
            Console.Error.WriteLine($"Error: {ex.Message}");
            return 1;
        }
    }


}