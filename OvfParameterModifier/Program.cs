// OvfParameterModifier/Program.cs

namespace OvfParameterModifier {
    /// <summary>
    /// Main program entry point.
    /// Its only job is to instantiate and run the main application class.
    /// </summary>
    public class Program {
        public static void Main(string[] args) {
            var app = new ParameterModifier();
            app.Run();
        }
    }
}