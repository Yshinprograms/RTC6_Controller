namespace OvfParameterModifier {
    public class Program {
        public static void Main(string[] args) {
            // This is the Composition Root. It creates and "wires up"
            // all the major components of the application.
            var ui = new ConsoleUI();
            var editor = new JobEditor();
            var app = new ParameterEditorApp(ui, editor);

            app.Run();
        }
    }
}