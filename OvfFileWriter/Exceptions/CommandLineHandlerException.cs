using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OvfFileWriter.Exceptions {
    internal class CommandLineHandlerException : OvfFileWriterException {
        public CommandLineHandlerException(string message) : base(message) { }
        public CommandLineHandlerException(string message, Exception innerException) 
            : base(message, innerException) { }
    }
}
