using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OvfFileWriter.Exceptions {
    internal class OvfFileGeneratorException : OvfFileWriterException {
        public OvfFileGeneratorException(string message) : base(message) { }
        public OvfFileGeneratorException(string message, Exception innerException)
            : base(message, innerException) {
        }
    }
}
