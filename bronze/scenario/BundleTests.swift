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
        precondition(model.activeName == "example_large.bronze" && !model.lastAttemptFailed)
        precondition(model.professions?.pointee.count == 120 && model.seed == 90210)
        var resets = 0
        var resetSeed: UInt = 0
        let commit: (UInt) -> Void = { seed in resets += 1; resetSeed = seed }
        precondition(model.load("vertical_slice", onCommit: commit))
        precondition(resets == 1 && resetSeed == model.seed)
        precondition(model.activeName == "vertical_slice.bronze")
        precondition(model.load("bronze_age_full", onCommit: commit))
        precondition(resets == 2 && model.activeName == "bronze_age_full.bronze")
        let activeProgram = model.program
        precondition(activeProgram != nil)
        precondition(!model.load("missing", onCommit: commit))
        precondition(resets == 2 && model.showDiagnostics)
        precondition(model.activeName == "bronze_age_full.bronze" && model.program == activeProgram)
        model.showDiagnostics = false
        precondition(!model.load("missing", presentsDiagnostics: false, onCommit: commit))
        precondition(resets == 2 && model.lastAttemptFailed && !model.showDiagnostics)
        precondition(!model.diagnostics.isEmpty && model.program == activeProgram)
        precondition(model.load("first_slice"))
        precondition(!model.lastAttemptFailed && model.program != nil)
        print("Packaged Bundle resource/controller tests passed outside the repository working directory.")
    }
}
