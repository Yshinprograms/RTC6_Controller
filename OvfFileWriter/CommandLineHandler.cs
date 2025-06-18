using OvfFileWriter.Exceptions;

namespace OvfFileWriter {
    internal class CommandLineHandler {
        public static ParsedArguments Parse(string[] args) {
            if (args.Length != 2) {
                throw new CommandLineHandlerException("Invalid number of arguments. Expected 2 arguments: <outputFilePath> <numOfLayers>");
            }

            string outputFilePath = args[0];

            if (!int.TryParse(args[1], out int numOfLayers)) {
                throw new CommandLineHandlerException("Error: Could not parse" +
                    $"number of layers: {args[1]}");
            }

            if (numOfLayers < 0) {
                throw new CommandLineHandlerException("Error: Number of layers cannot be less than 0.");
            }

            return new ParsedArguments(outputFilePath, numOfLayers);
        }
    }
}
