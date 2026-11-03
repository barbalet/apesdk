import Foundation

@main
struct BundleTests {
    @MainActor static func main() {
        precondition(CommandLine.arguments.count == 2)
        let bundle = Bundle(path: CommandLine.arguments[1])!
        for name in BronzeScenarioController.names {
            precondition(bundle.url(forResource: name, withExtension: "bronze") != nil)
        }
        let model = BronzeScenarioController(bundle: bundle)
        precondition(model.activeName == "first_slice.bronze" && !model.lastAttemptFailed)
        var resets = 0
        var resetSeed: UInt = 0
        let commit: (UInt) -> Void = { seed in resets += 1; resetSeed = seed }
        precondition(model.load("vertical_slice", onCommit: commit))
        precondition(resets == 1 && resetSeed == model.seed)
        precondition(model.activeName == "vertical_slice.bronze")
        precondition(!model.load("bronze_age_full", onCommit: commit))
        precondition(resets == 1)
        precondition(model.activeName == "vertical_slice.bronze" && model.showDiagnostics)
        precondition(model.diagnostics.contains("ValidationError:136"))
        precondition(!model.load("missing", onCommit: commit))
        precondition(resets == 1)
        precondition(model.activeName == "vertical_slice.bronze")
        precondition(model.load("first_slice"))
        precondition(!model.lastAttemptFailed && model.diagnostics.contains("not active yet"))
        print("Packaged Bundle resource/controller tests passed outside the repository working directory.")
    }
}
