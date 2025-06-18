using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OvfFileWriter {
    internal abstract class OvfFileWriterException : Exception {
        protected OvfFileWriterException(string message) : base(message) { }
        protected OvfFileWriterException(string message, Exception innerException) 
            : base(message, innerException) {
        }
}
