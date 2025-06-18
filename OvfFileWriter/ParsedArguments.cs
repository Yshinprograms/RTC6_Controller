using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OvfFileWriter {
    internal class ParsedArguments {
        // This property can be read by anyone, but can ONLY be set
        // from within this class (i.e., in the constructor).
        public string OutputFilePath { get; private set; }
        public int NumberOfLayers { get; private set; }

        public ParsedArguments(string outputFilePath, int numberOfLayers) {
            OutputFilePath = outputFilePath;
            NumberOfLayers = numberOfLayers;
        }
    }
}
